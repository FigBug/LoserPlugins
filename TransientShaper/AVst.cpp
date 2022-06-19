//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>
#include <stdlib.h>


// Mich's Formulas V 0.1

#define cAmpDB_     8.656170245 // 6/log(2);
#define cDBAmp_     0.115524530 // log(2)/6;
#define cPi_        3.141592654
#define cSqrt2_     1.414213562 // sqrt(2);
#define cSqrt2h_    0.707106781 // sqrt(0.5f);
#define cDC_     1e-30

inline float sqr (float x)
{
    return ( x*x );
}

inline float abs (float x)
{
    return ( x<0 ? -x:x);
}

inline float sign (float x)
{
    return (x>0 ? 1:-1);   
}

inline float min (float x, float y)
{
    return ( x<y ? x:y);
}

inline float max (float x, float y)
{
    return ( x>y ? x:y);
}

inline float amp2DB (float amp)
{
    return ( log(amp) * cAmpDB_ );
}

inline float dB2Amp (float dB)
{
    return ( exp(dB * cDBAmp_ ) );
}

inline float absmax (float x, float y)
{
    return max(abs(x),abs(y));
}

inline float limit (float in, float limit)
{
    return min(max(in,-limit),limit);   
}

// eof Mich's formulas


#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 4)	// 1 program, 15 parameters
{

    attackFader = sustainFader = 0.5;
    attack = sustain = 0;
// 1 was kick ass
    b1EnvLP = -exp(-2.0*cPi_*5 / getSampleRate() ); // 5Hz
    a0EnvLP = 1.0 + b1EnvLP;
    
    b1EnvAttLP = -exp(-2.0*cPi_*200 / getSampleRate() ); // 200Hz
    a0EnvAttLP = 1.0 + b1EnvAttLP;
    
    b1EnvRelLP = -exp(-2.0*cPi_*0.5 / getSampleRate() ); // 1Hz
    a0EnvRelLP = 1.0 + b1EnvRelLP;

    smooth = 0.5;
    b1GainLP = -exp(-2.0*cPi_*89.4427191 / getSampleRate() ); // 89.44....Hz
    a0GainLP = 1.0 + b1GainLP;
    
    outVolumeFader = outVolume = 1; // = 0 dB
       
	setNumInputs (2);		// stereo in + stereo sidechain (aux)
	setNumOutputs (2);
	setUniqueID (CCONST ('T','R','A','S') );
//    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

	suspend(); // flush buffers and reset variables
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

}

void AVst::suspend()
{
   
    b1EnvLP = -exp(-2.0*cPi_*5 / getSampleRate() );
    a0EnvLP = 1.0 + b1EnvLP;
    tmpEnvLP = 0;

    b1EnvAttLP = -exp(-2.0*cPi_*200 / getSampleRate() );
    a0EnvAttLP = 1.0 + b1EnvAttLP;
    tmpEnvAttLP = 0;

    b1EnvRelLP = -exp(-2.0*cPi_*0.5 / getSampleRate() );
    a0EnvRelLP = 1.0 + b1EnvRelLP;
    tmpEnvRelLP = 0;

    tmpGainLP = 0;
    
}

//-------------------------------------------------------------------------------------------------------
void AVst::setProgramName (char *name)
{
	strcpy (programName, name);
}

//-----------------------------------------------------------------------------------------
void AVst::getProgramName (char *name)
{
	strcpy (name, programName);
}

//-----------------------------------------------------------------------------------------
void AVst::setParameter (long index, float value)
{

// Get the values from faders(sliders) and convert and stuff
    switch (index)
	{
		case  0 : 
            attackFader = value;
            attack = (-2 + attackFader * 4);
        break;

		case  1 :
            sustainFader = value;
            sustain = (-4 + sustainFader * 8);
        break;
        
        case 2:
            smooth = value;
            b1GainLP = -exp(-2.0*cPi_* (  pow(8000,(1-smooth))   ) / getSampleRate() ); 
            a0GainLP = 1.0 + b1GainLP;   
        break;
               
        case 3 :
            outVolumeFader = value;
            outVolume = dB2Amp ( -20 + outVolumeFader * 20 );
        break;

	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = attackFader;      break;
		case 1 : v = sustainFader;     break;
		case 2 : v = smooth;     break;
		case 3 : v = outVolumeFader;     break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case 0 : strcpy (label, "Attack");     break;
		case 1 : strcpy (label, "Sustain");       break;
		case 2 : strcpy (label, "Smoothness");       break;
		case 3 : strcpy (label, "Out Volume");       break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : float2string (attack*100, text);           break;
		case 1 : float2string (sustain*50, text);       break;
		case 2 : float2string (smooth*100, text);       break;
		case 3 : float2string (amp2DB(outVolume), text);        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "%");	    break;
		case 1 : strcpy (label, "%");	    break;
		case 2 : strcpy (label, "%");	    break;
		case 3 : strcpy (label, "dB");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Transient Shaper");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Transient Shaper");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getVendorString (char* text)
{
	strcpy (text, "");
	return true;
}

//-----------------------------------------------------------------------------------------
void AVst::process (float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

  while (--sampleFrames >= 0)
  {
// =========================================
// TRACK MAX: via absmax
    maxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    env = sqrt( (tmpEnvLP = a0EnvLP*maxSpls - b1EnvLP*tmpEnvLP + cDC_)  );
    envAtt = sqrt( (tmpEnvAttLP = a0EnvAttLP*maxSpls - b1EnvAttLP*tmpEnvAttLP + cDC_)  );
    envRel = sqrt( (tmpEnvRelLP = a0EnvRelLP*maxSpls - b1EnvRelLP*tmpEnvRelLP + cDC_)  );
   
    gain = exp( log( max(envAtt/env,1) ) *attack ) * exp( log( max(envRel/env,1) ) *sustain );
    gain = min( sqrt( (tmpGainLP = a0GainLP*gain - b1GainLP*tmpGainLP + cDC_) ) , gain );
                
    (*out1++) += (*in1++) * gain * outVolume;
    (*out2++) += (*in2++) * gain * outVolume;
        
  } // eof while
       
}

//-----------------------------------------------------------------------------------------
void AVst::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
  float *in1  =  inputs[0];
  float *in2  =  inputs[1];
  float *out1 = outputs[0];
  float *out2 = outputs[1];

  while (--sampleFrames >= 0)
  {
// =========================================
// TRACK MAX: via absmax
    maxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    env = sqrt( (tmpEnvLP = a0EnvLP*maxSpls - b1EnvLP*tmpEnvLP + cDC_)  );
    envAtt = sqrt( (tmpEnvAttLP = a0EnvAttLP*maxSpls - b1EnvAttLP*tmpEnvAttLP + cDC_)  );
    envRel = sqrt( (tmpEnvRelLP = a0EnvRelLP*maxSpls - b1EnvRelLP*tmpEnvRelLP + cDC_)  );
   
    gain = exp( log( max(envAtt/env,1) ) *attack ) * exp( log( max(envRel/env,1) ) *sustain );
    gain = min( sqrt( (tmpGainLP = a0GainLP*gain - b1GainLP*tmpGainLP + cDC_) ) , gain) ;
                
    (*out1++) = (*in1++) * gain * outVolume;
    (*out2++) = (*in2++) * gain * outVolume;
    
  } // eof while

}
