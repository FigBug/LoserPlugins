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
	setUniqueID (CCONST ('L','i','m','s') );
	canProcessReplacing ();
	canDoubleReplacing ();
   
    samplesSinceLastPeakResetL = 0;
    samplesSinceLastPeakResetR = 0;
    samplesSinceLastIdleCall = 0;
      
    gain  = 1;
       
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
    KMeterL = 0;
    KMeterR = 0;
    KMeterSumL = 0;
    KMeterSumR = 0;
        
    GRMeterL = GRPeakL = 1;
	GRMeterR = GRPeakR = 1;

    envC = 0; envR = 0; envL = 0;
    	
    linkAtt = exp( +6*0.115129254 / (getSampleRate() *25/1000) );
    rel = exp( -6*0.115129254 / (getSampleRate() *190/1000) );
    env_rel = exp( -6*0.115129254 / (getSampleRate() *010/1000) );
    
    GRMeterFallOff = exp( +30*0.115129254 / getSampleRate() );
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
        case kGain : gain = exp( value*20 * 0.115129254);
        break;
	}

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
		default : vst_strncpy (label, "WTF", kVstMaxParamStrLen); break;       
	}
}

//-----------------------------------------------------------------------------------------
void VST::getParameterLabelDisplay (VstInt32 index, float value, char* string)
{
    switch (index)
    {
        case kGain : sprintf ( string, "%.2fdB", 20*value ); break;
    }
}

//------------------------------------------------------------------------
bool VST::getEffectName (char* name)
{
	vst_strncpy (name, "Limes", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool VST::getProductString (char* text)
{
	vst_strncpy (text, "Limes", kVstMaxProductStrLen);
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
        
        float env_inL = (*in1) > 0 ? (*in1) : -(*in1);
        float env_inR = (*in2) > 0 ? (*in2) : -(*in2);

        envL  = envL < env_inL ? env_inL  : max(rel * envL , env_inL  );
        envR  = envR < env_inR ? env_inR  : max(rel * envR , env_inR  );

        float env_inC = max(envL,envR);
        envC  = envC < env_inC ? min(linkAtt * envC, env_inC) : env_inC;

        envL = max(envL, envC);
        envR = max(envR, envC);

        float gainL = envL > 0.988553094f ? 0.988553094f/envL : 1;
        float gainR = envR > 0.988553094f ? 0.988553094f/envR : 1;
/*
        float gainL = envL > 0.97 ? pow(envL,0.02-1) * (pow(0.97,-(0.02-1))) : 1;
        float gainR = envR > 0.97 ? pow(envR,0.02-1) * (pow(0.97,-(0.02-1))) : 1;
*/        
        (*in1) *= gainL;
        (*in2) *= gainR;
/*
        (*out1) = min( max( (*in1) , -0.998849369 ) , 0.998859369) ;
        (*out2) = min( max( (*in2) , -0.998859369 ) , 0.998859369) ;
*/
        (*out1) = (*in1);
        (*out2) = (*in2);

        *in1++; *in2++;

    // K-METER !!!!!
    if (editor)
    {
        if(gainL<GRMeterL) GRMeterL = gainL;
        if(gainR<GRMeterR) GRMeterR = gainR;
        KMeterSumL += (*out1)*(*out1++);
        KMeterSumR += (*out2)*(*out2++);
    }
    // EOF


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
        
        double env_inL = (*in1) > 0 ? (*in1) : -(*in1);
        double env_inR = (*in2) > 0 ? (*in2) : -(*in2);

        env_inL = env_inL + 1e-30;
        env_inR = env_inR + 1e-30;
        
        env_iL = env_iL < env_inL ? env_inL : env_inL + env_rel * (env_iL - env_inL);
        env_iR = env_iR < env_inR ? env_inR : env_inR + env_rel * (env_iR - env_inR);

        envL  = envL < env_iL ? env_iL  : max(rel * envL , env_iL  );
        envR  = envR < env_iR ? env_iR  : max(rel * envR , env_iR  );

        double env_inC = max(envL,envR);
        envC  = envC < env_inC ? min(linkAtt * envC, env_inC) : env_inC;

        envL = max(envL, envC);
        envR = max(envR, envC);

        double gainL = envL > 0.988553094 ? 0.988553094/envL : 1;
        double gainR = envR > 0.988553094 ? 0.988553094/envR : 1;
/*
        double gainL = envL > 0.97 ? pow(envL,0.02-1) * (pow(0.97,-(0.02-1))) : 1;
        double gainR = envR > 0.97 ? pow(envR,0.02-1) * (pow(0.97,-(0.02-1))) : 1;
*/        
        (*in1) *= gainL;
        (*in2) *= gainR;
/*
        (*out1) = min( max( (*in1) , -0.998849369 ) , 0.998859369) ;
        (*out2) = min( max( (*in2) , -0.998859369 ) , 0.998859369) ;
*/
        (*out1) = (*in1);
        (*out2) = (*in2);

        *in1++; *in2++;

    // K-METER !!!!!
    if (editor)
    {
        if(gainL<GRMeterL) GRMeterL = gainL;
        if(gainR<GRMeterR) GRMeterR = gainR;
        KMeterSumL += (*out1)*(*out1++);
        KMeterSumR += (*out2)*(*out2++);
    }
    // EOF
    }

}
