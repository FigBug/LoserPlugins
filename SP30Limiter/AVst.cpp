//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>
#include <stdlib.h>



// Mich's Formulas V 0.3

#define cAmpDB_     8.656170245 // 6/log(2);
#define cDBAmp_     0.115524530 // log(2)/6;
#define cPi_        3.141592654
#define cSqrt2_     1.414213562 // sqrt(2);
#define cSqrt2h_    0.707106781 // sqrt(0.5f);
#define cDcAdd_     1e-30

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
    if (in >  limit) return  limit;
    if (in < -limit) return -limit;
    return in;
}

// eof Mich's formulas



#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 3)	// 1 program, 2 parameters
{

    threshFader = 1;
    thresh = 1;
    
    ceilingFader = 1;
    ceiling = 1;

    saturation = 0;
    satC = 1;
    
    clipThresh = 1;
    clipRatio = 1;
    clipRatioFader = 0;
    clipThreshFader = 1;
    
//    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() );
    b1EnvLP = -exp(-2.0*cPi_*7.952707288 / getSampleRate() );
    a0EnvLP = 1.0 + b1EnvLP;
  
	setNumInputs (2);		// stereo in + stereo sidechain (aux)
	setNumOutputs (2);
	setUniqueID (CCONST ('S','P','3','L') );
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
    gain = 1;
    tmpEnvLP = 0;
//    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() );
    b1EnvLP = -exp(-2.0*cPi_*7.952707288 / getSampleRate() );
    a0EnvLP = 1.0 + b1EnvLP;
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
            threshFader = value;
            thresh = dB2Amp( -20 + threshFader * 20 );
        break;      
		case  1 : 
            ceilingFader = value;
            ceiling = dB2Amp( -6 + ceilingFader * 6 );
        break;
        case  2 :
            saturation = 0.5*value;
            satC = sin(saturation*cPi_);
        break;  
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case  0 : v = threshFader;      break;
		case  1 : v = ceilingFader;      break;
        case  2 : v = saturation*2;    break;
    }
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case  0 : strcpy (label, "Threshold");     break;
		case  1 : strcpy (label, "Out Ceiling");     break;
		case  2 : strcpy (label, "Saturation");   break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case  0 : dB2string (thresh, text);           break;
		case  1 : dB2string (ceiling, text);           break;
		case  2 : float2string (saturation*200, text);           break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case  0 : strcpy (label, "dB");	    break;
		case  1 : strcpy (label, "dB");	    break;
		case  2 : strcpy (label, "%");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Simple Peak-3 Limiter");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Simple Peak-3 Limiter");
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
// FEED: set detL/detR according to the feed
    rms = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    rms = sqrt( (tmpEnvLP = a0EnvLP*rms - b1EnvLP*tmpEnvLP + cDcAdd_) - cDcAdd_ );
    
// =========================================
// GR-TRACKER: Calculate the current gain to be applied
    if (rms > thresh) gain =  rms ;
    else gain = thresh;

    (*in1) /= gain;
    (*in2) /= gain;

// =========================================
// MIXING: REPLACING!!!!!: mix output*volume + dry mix
    (*in1) = limit( (*in1) , 1 );
    (*in2) = limit( (*in2) , 1 );
    
    if (saturation)
    {
        (*in1) = sin( (*in1) * saturation * cPi_ ) / satC;
        (*in2) = sin( (*in2) * saturation * cPi_ ) / satC;
    }
    
    (*out1++) = (*in1++) * ceiling;
    (*out2++) = (*in2++) * ceiling;

  } // eof while

}
