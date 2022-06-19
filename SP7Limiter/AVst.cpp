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

    threshFader = 1;
    thresh = 1;
  
//    releaseFader = 0.430886938; // ~200ms
//    releaseSpls = 0.2 * getSampleRate(); // =200 ms
    releaseSpls = releaseFader = release = 0;
    release = dB2Amp( -6 / max( releaseSpls , 0) );

    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() ); // 10Hz
    a0EnvLP = 1.0 + b1EnvLP;
    
    ceilingFader = 1; // ~ 0 dB
    ceiling = 1; // = 0 dB
    
    saturation = 0;
    satC = 1;
    
    volume = 1; // = 0 dB
    
	setNumInputs (2);		// stereo in + stereo sidechain (aux)
	setNumOutputs (2);
	setUniqueID (CCONST ('S','P','7','L') );
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
    gain = seekGain = 1;
   
    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() );
    a0EnvLP = 1.0 + b1EnvLP;
    tmpEnvLP = 0;
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
            releaseFader = value;
            releaseSpls = (releaseFader*releaseFader*releaseFader) * 2.5 * getSampleRate();
            release = dB2Amp( -6 / releaseSpls );        
        break;
        
        case  2 :
            saturation = 0.5*value;
            satC = sin(saturation*cPi_);
        break;   
                          
		case  3 :
            ceilingFader = value;
            ceiling = dB2Amp ( -6 + ceilingFader * 6 );
        break;

	}


    volume = ceiling / thresh;

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = threshFader;      break;
		case 1 : v = releaseFader;     break;
        case 2 : v = saturation*2;    break;
		case 3 : v = ceilingFader;   break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case 0 : strcpy (label, "Threshold");     break;
		case 1 : strcpy (label, "Release");       break;
		case 2 : strcpy (label, "Saturation");   break;
		case 3 : strcpy (label, "Ceiling");        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : dB2string (thresh, text);           break;
		case 1 : ms2string (releaseSpls, text);       break;
		case 2 : float2string (saturation*200, text);           break;
		case 3 : dB2string (ceiling, text);        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "dB");	    break;
		case 1 : strcpy (label, "ms");	    break;
		case 2 : strcpy (label, "%");	    break;
		case 3 : strcpy (label, "dB");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Simple Peak-7 Limiter");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Simple Peak-7 Limiter");
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
    seekMaxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    rms = sqrt( (tmpEnvLP = a0EnvLP*seekMaxSpls - b1EnvLP*tmpEnvLP + cDcAdd_) - cDcAdd_ );
    if (releaseSpls != 0) rms = max(seekMaxSpls,rms);
   
// =========================================
// GR-TRACKER: Calculate the current gain to be applied
    if (rms > thresh) seekGain =  thresh / rms ;
    else seekGain = 1;

// =========================================
// GAIN FOLLOWER: make 'gain' follow seekGain with the specified sttack and release speed
    if (releaseSpls != 0)
    {
        if (gain > seekGain) 
        {
            gain = seekGain;
        }
        else 
        {
            gain = min( gain/release , seekGain );
        }
    }
    else gain = seekGain;
    
// =========================================
// MIXING: REPLACING!!!!!:
    (*in1) = limit( (*in1)*gain*volume , 1 );
    (*in2) = limit( (*in2)*gain*volume , 1 );
    
    if (saturation)
    {
        (*in1) = sin( (*in1) * saturation * cPi_ ) / satC;
        (*in2) = sin( (*in2) * saturation * cPi_ ) / satC;
    }
    
    (*out1++) = (*in1++) * ceiling;
    (*out2++) = (*in2++) * ceiling;
    
    
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
    seekMaxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    rms = sqrt( (tmpEnvLP = a0EnvLP*seekMaxSpls - b1EnvLP*tmpEnvLP + cDcAdd_) - cDcAdd_ );
    if (releaseSpls != 0) rms = max(seekMaxSpls,rms);
   
// =========================================
// GR-TRACKER: Calculate the current gain to be applied
    if (rms > thresh) seekGain =  thresh / rms ;
    else seekGain = 1;

// =========================================
// GAIN FOLLOWER: make 'gain' follow seekGain with the specified sttack and release speed
    if (releaseSpls != 0)
    {
        if (gain > seekGain) 
        {
            gain = seekGain;
        }
        else 
        {
            gain = min( gain/release , seekGain );
        }
    }
    else gain = seekGain;
    
// =========================================
// MIXING: REPLACING!!!!!:
    (*in1) = limit( (*in1)*gain*volume , 1 );
    (*in2) = limit( (*in2)*gain*volume , 1 );
    
    if (saturation)
    {
        (*in1) = sin( (*in1) * saturation * cPi_ ) / satC;
        (*in2) = sin( (*in2) * saturation * cPi_ ) / satC;
    }
    
    (*out1++) = (*in1++) * ceiling;
    (*out2++) = (*in2++) * ceiling;
    
  } // eof while

}
