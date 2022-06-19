#include <cstring>
#include <cstdio>
#include <cmath>

#ifndef __VST__
#include "VST.h"
#endif

#ifndef __VSTEditor__
#include "editor/VSTEditor.h"
#endif

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new VST (audioMaster);
}

//-----------------------------------------------------------------------------
VST::VST (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, kNumParams) // # programms , # params
{
	setNumInputs (2);
	setNumOutputs (2);
	setUniqueID (CCONST ('S','f','C','p') );
	canProcessReplacing ();
	canDoubleReplacing ();
       
    samplesSinceLastPeakResetL = 0;
    samplesSinceLastPeakResetR = 0;
    samplesSinceLastIdleCall = 0;
     
    gain  = 1;
    P1 = 0.5;
    P0 = 0;
    P2 = 0;
              
	// create the editor
	editor = new VSTEditor (this);

	vst_strncpy (programName, "Default", kVstMaxProgNameLen);

//	resume ();
}

//-------------------------------------------------------------------------------------------------------
VST::~VST ()
{

}

//-------------------------------------------------------------------------------------------------------
void VST::resume()
{ 

	GRMeterL = GRPeakL = 1;
	GRMeterR = GRPeakR = 1;
     
    KMeterL = 0;
    KMeterR = 0;
    KMeterSumL = 0;
    KMeterSumR = 0;
     
//    GRMeterFallOff = 1/exp( -1 / (getSampleRate() * 600/1000) );
    GRMeterFallOff = exp( +30*0.115129254 / getSampleRate() );

//    KMeterCoef = exp( -1 / (getSampleRate() * 300/1000) );
    KMeterCoef = exp( -30*0.115129254 / getSampleRate() );
}

void VST::suspend()
{

}


//-------------------------------------------------------------------------------------------------------
void VST::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void VST::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void VST::setParameter (VstInt32 index, float value)
{
    switch (index)
	{
        case kGain : 
            gain = exp( value*20 * 0.115129254);   
        break;
        case kKnee:
            P1 = value;   
        break;
	}

        P0 = -P1*log(gain)*8.685889638;
        P2 = +P1*log(gain)*8.685889638;
            
	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);

}

//-----------------------------------------------------------------------------------------
float VST::getParameter (VstInt32 index)
{
    float v = 0;
	switch (index)
	{
        case kGain : v = (log(gain)*8.685889638)/20; break;
        case kKnee: v = P1; break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------

float VST::getVU(VstInt32 index)
{
    float v = 0;
    switch (index)
    {
        case kGRL : v = GRMeterL; break;
        case kGRPeakL : v = GRPeakL; break;
        case kGRR : v = GRMeterR; break;
        case kGRPeakR : v = GRPeakR; break;
        case kKMeterL : v = KMeterL; break;
        case kKMeterR : v = KMeterR; break;
    }
    return v;
}

void VST::resetLastIdleCall()
{
    if(samplesSinceLastIdleCall)
    {
    samplesSinceLastPeakResetL += samplesSinceLastIdleCall;
    if( (GRPeakL>GRMeterL) || (samplesSinceLastPeakResetL > getSampleRate()) )
    {
        GRPeakL = GRMeterL;
        samplesSinceLastPeakResetL = 0;
    }
    
    samplesSinceLastPeakResetR += samplesSinceLastIdleCall;
    if( (GRPeakR>GRMeterR) || (samplesSinceLastPeakResetR > getSampleRate()) )
    {
        GRPeakR = GRMeterR;
        samplesSinceLastPeakResetR = 0;
    }
    
    GRMeterL *= pow(GRMeterFallOff,samplesSinceLastIdleCall);
    GRMeterR *= pow(GRMeterFallOff,samplesSinceLastIdleCall);

   
    KMeterSumL /= samplesSinceLastIdleCall;
    KMeterSumL = sqrt(KMeterSumL);
    KMeterL = KMeterSumL + pow(KMeterCoef,samplesSinceLastIdleCall) * (KMeterL - KMeterSumL);
    if(KMeterL < 0.00001) KMeterL = 0;

    KMeterSumR /= samplesSinceLastIdleCall;
    KMeterSumR = sqrt(KMeterSumR);
    KMeterR = KMeterSumR + pow(KMeterCoef,samplesSinceLastIdleCall) * (KMeterR - KMeterSumR);
    if(KMeterR < 0.00001) KMeterR = 0;
    
    KMeterSumL = 0;
    KMeterSumR = 0;

    samplesSinceLastIdleCall = 0;   
    }

}

//-----------------------------------------------------------------------------------------
void VST::getParameterName (VstInt32 index, char* label)
{
    switch (index)
	{
		case kGain : vst_strncpy(label, "Gain", kVstMaxParamStrLen); break;
		case kKnee : vst_strncpy(label, "Softness", kVstMaxParamStrLen); break;
	}
}

//-----------------------------------------------------------------------------------------
void VST::getParameterDisplay (VstInt32 index, char* text)
{

    switch (index)
	{
		default : getParameterLabelDisplay(index, getParameter(index), text); break;       
	}

}

//-----------------------------------------------------------------------------------------
void VST::getParameterLabel (VstInt32 index, char* label)
{
    switch (index)
	{
		default : vst_strncpy (label, "Zzz", kVstMaxParamStrLen); break;       
	}
}

//-----------------------------------------------------------------------------------------
void VST::getParameterLabelDisplay (VstInt32 index, float value, char* string)
{
    switch (index)
    {
        case kGain : sprintf ( string, "%.2fdB", 20*value ); break;
        case kKnee : sprintf ( string, "%.1f%%", 100*value ); break;
    }
}

//------------------------------------------------------------------------
bool VST::getEffectName (char* name)
{
	vst_strncpy (name, "Softclamp", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool VST::getProductString (char* text)
{
	vst_strncpy (text, "Softclamp", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool VST::getVendorString (char* text)
{
	vst_strncpy (text, "", kVstMaxVendorStrLen);
	return true;
}

//-----------------------------------------------------------------------------------------
VstInt32 VST::getVendorVersion ()
{ 
	return 1000; 
}

//-----------------------------------------------------------------------------------------
void VST::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
    float* in1  =  inputs[0];
    float* in2  =  inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];

    samplesSinceLastIdleCall += sampleFrames;
    
    while (--sampleFrames >= 0)
    {

        (*in1) *= gain;
        (*in2) *= gain;
                
        float inL = log(  (*in1) > 0 ? (*in1) : -(*in1)  + 1e-10 ) * 8.685889638;
        float inR = log(  (*in2) > 0 ? (*in2) : -(*in2)  + 1e-10 ) * 8.685889638;

        float gainL, gainR;

        if( inL > P0)
        {
            if( inL < P2)
            {
                float t = 1- (inL - P0) / (P2-P0) ;
                gainL = exp( ((P0 * t*t) - inL ) *0.115129254 );

            }
            else
                gainL = exp(-inL*0.115129254);
            
        }
        else
            gainL = 1;


        if( inR > P0)
        {
            if( inR < P2)
            {
                float t = 1-(inR - P0) / (P2-P0);
                gainR = exp( ((P0 * t*t) - inR ) *0.115129254 );
            }
            else
                gainR = exp(-inR*0.115129254);
            
        }
        else
            gainR = 1;
            
        (*out1) = (*in1++)*gainL;
        (*out2) = (*in2++)*gainR;

    // K-METER !!!!!
    if(editor)
    {
        if(gainL<GRMeterL) GRMeterL = gainL;
        if(gainR<GRMeterR) GRMeterR = gainR;
                
        KMeterSumL += (*out1)*(*out1);
        KMeterSumR += (*out2)*(*out2);
    }
    // EOF
	++out1;
	++out2;

    }
}

//-----------------------------------------------------------------------------------------
void VST::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{

    double* in1  =  inputs[0];
    double* in2  =  inputs[1];
    double* out1 = outputs[0];
    double* out2 = outputs[1];

    samplesSinceLastIdleCall += sampleFrames;

    while (--sampleFrames >= 0)
    {

        (*in1) *= gain;
        (*in2) *= gain;
                
        double inL = log(  (*in1) > 0 ? (*in1) : -(*in1)  + 1e-10 ) * 8.685889638;
        double inR = log(  (*in2) > 0 ? (*in2) : -(*in2)  + 1e-10 ) * 8.685889638;

        double gainL, gainR;


        if( inL > P0)
        {
            if( inL < P2)
            {
                double t = 1- (inL - P0) / (P2-P0) ;
                gainL = exp( ((P0 * t*t) - inL ) *0.115129254 );

            }
            else
                gainL = exp(-inL*0.115129254);
            
        }
        else
            gainL = 1;


        if( inR > P0)
        {
            if( inR < P2)
            {
                double t = 1-(inR - P0) / (P2-P0);
                gainR = exp( ((P0 * t*t) - inR ) *0.115129254 );
            }
            else
                gainR = exp(-inR*0.115129254);
            
        }
        else
            gainR = 1;
            
        (*out1) = (*in1++)*gainL;
        (*out2) = (*in2++)*gainR;

    // K-METER !!!!!     
    if(editor)
    {
        if(gainL<GRMeterL) GRMeterL = gainL;
        if(gainR<GRMeterR) GRMeterR = gainR;
                
        KMeterSumL += (*out1)*(*out1);
        KMeterSumR += (*out2)*(*out2);
    }
    // EOF
	++out1;
	++out2;


    }

}
