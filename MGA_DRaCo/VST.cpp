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
	setUniqueID (CCONST ('D','R','a','C') );
	canProcessReplacing ();
	canDoubleReplacing ();
	
    samplesSinceLastIdleCall = 0;
    
    thresh  = 1.;
    thresh_f = 1.;
    ratio = 0.;
    ratio_f = 0.;
    att_f = 0.02;
    rel_f = 0.08;
    out = 1.;
    out_f = 0.;

    foo = pow(thresh,-ratio);
      
	// create the editor
	editor = new VSTEditor (this);

	vst_strncpy (programName, "Default", kVstMaxProgNameLen);

//	resume (); // should get called automagically anyway
}

//-------------------------------------------------------------------------------------------------------
VST::~VST ()
{
	printf("DRaCo dies.\n");
}

//-------------------------------------------------------------------------------------------------------
void VST::resume()
{ 
    GRPeak = GRMeter = 1.;

    env_i = 0.;
    env = 0.;
    gain = 1.;

    meterFallOff = exp( +30*0.115129254 / getSampleRate() );

    att = exp( -1. / (getSampleRate() * att_f*500.*0.25/1000.) );
    rel = exp( -1. / (getSampleRate() * rel_f*0.5*2500.*0.95/1000.) );
    env_rel = exp( -1. / (getSampleRate() * rel_f*0.5*2500.*0.05/1000.) );

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
        case kThresh :
                thresh_f = value;
                thresh  = exp( (thresh_f-1)*40. * 0.115129254);
            break;
        case kRatio :
                ratio_f = value;
                ratio = (ratio_f < 1. ? 1./(ratio_f*99.+1.) : 0.) - 1.;
            break;
        case kAttack :
                att_f = value;
                att = exp( -1. / (getSampleRate() * att_f*500.*0.25/1000.) );
            break;
        case kRelease :
                rel_f = value;
                rel = exp( -1. / (getSampleRate() * rel_f*0.5*2500.*0.95/1000.) );
                env_rel = exp( -1. / (getSampleRate() * rel_f*0.5*2500.*0.05/1000.) );
            break;
        case kOutput :
                out_f = value;
                out = exp(out_f *40. * 0.115129254);
            break;

	}

    foo = pow(thresh,-ratio);

	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);

}

//-----------------------------------------------------------------------------------------
float VST::getParameter (VstInt32 index)
{
    float v = 0;
	switch (index)
	{
        case kThresh : v = thresh_f; break;
        case kRatio  : v = ratio_f; break;
        case kAttack : v = att_f; break;
        case kRelease : v = rel_f; break;
        case kOutput : v = out_f; break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------

float VST::getVU(VstInt32 index)
{
    float v = 0;
    switch (index)
    {
        case kGR : v = GRMeter; break;
        case kGRPeak : v = GRPeak; break;
    }
    return v;
}

void VST::resetLastIdleCall()
{
    samplesSinceLastPeakReset += samplesSinceLastIdleCall;
    if( (GRPeak>GRMeter) || (samplesSinceLastPeakReset > getSampleRate()) )
    {
        GRPeak = GRMeter;
        samplesSinceLastPeakReset = 0;
    }
    GRMeter *= pow(meterFallOff,samplesSinceLastIdleCall);
    samplesSinceLastIdleCall = 0;   
}

//-----------------------------------------------------------------------------------------
void VST::getParameterName (VstInt32 index, char* label)
{
    switch (index)
	{
		case kThresh : vst_strncpy(label, "Thresh", kVstMaxParamStrLen); break;       
		case kRatio : vst_strncpy(label, "Ratio", kVstMaxParamStrLen); break;
		case kAttack : vst_strncpy(label, "Attack", kVstMaxParamStrLen); break;
		case kRelease : vst_strncpy(label, "Release", kVstMaxParamStrLen); break;
		case kOutput : vst_strncpy(label, "Output", kVstMaxParamStrLen); break;
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
		default : vst_strncpy (label, "FTW", kVstMaxParamStrLen); break;       
	}
}

//-----------------------------------------------------------------------------------------
void VST::getParameterLabelDisplay (VstInt32 index, float value, char* string)
{
    switch (index)
    {
        case kThresh : sprintf ( string, "%.2fdB", -40.*(1.-value) ); break;
        case kRatio :
                if(value<1) sprintf ( string, "%.2f:1", value*99.+1. );
                else sprintf ( string, "oo :1" );
            break;
        case kAttack : sprintf ( string, "%.2fms", value*500. ); break;
        case kRelease : sprintf ( string, "%.1fms", value*2500. ); break;
        case kOutput : sprintf ( string, "%.2fdB", 40.*value ); break;
    }
}

//------------------------------------------------------------------------
bool VST::getEffectName (char* name)
{
	vst_strncpy (name, "DRaCo", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool VST::getProductString (char* text)
{
	vst_strncpy (text, "DRaCo", kVstMaxProductStrLen);
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

        float env_in = max( (*in1)<0?-(*in1):(*in1) , (*in2)<0?-(*in2):(*in2) );

        env_in = env_in + 1e-30;

        env_i = env_i < env_in ? env_in: env_in + env_rel * (env_i - env_in);
        env = env < env_i ? env_i: env_i + env_rel * (env - env_i);

//        gain_in = env > thresh ? thresh/env * pow(env/thresh,ratio ) : 1;
        float gain_in = env > thresh ? pow(env,ratio) * foo : 1;
        
        gain = gain < gain_in ? gain_in + rel * (gain - gain_in) : gain_in + att * (gain - gain_in);
  
     
    // METER !!!!!
    if(editor)
    {
        if(gain<GRMeter) GRMeter = gain;
    }
    // EOF
    
        (*out1++) = (*in1++)*gain*out;
        (*out2++) = (*in2++)*gain*out;  
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

        double env_in = max( (*in1)<0?-(*in1):(*in1) , (*in2)<0?-(*in2):(*in2) );

        env_in = env_in + 1e-30;
        
        env_i = env_i < env_in ? env_in: env_in + env_rel * (env_i - env_in);
        env = env < env_i ? env_i: env_i + env_rel * (env - env_i);

//        double gain_in = env > thresh ? thresh/env * pow(env/thresh,ratio ) : 1;
        double gain_in = env > thresh ? pow(env,ratio) * foo : 1;

        gain = gain < gain_in ? gain_in + rel * (gain - gain_in) : gain_in + att * (gain - gain_in);
  
    // METER !!!!!
    if(editor)
    {
        if(gain<GRMeter) GRMeter = gain;
    }
    // EOF
    
        (*out1++) = (*in1++)*gain*out;
        (*out2++) = (*in2++)*gain*out;      

    }

}
