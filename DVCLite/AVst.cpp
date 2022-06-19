//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>
#include <stdlib.h>


// Mich's Formulas V 0.2

#define cAmpDB_     8.656170245 // 6/log(2);
#define cDBAmp_     0.115524530 // log(2)/6;
#define cPi_        3.141592654
#define cSqrt2_     1.414213562 // sqrt(2);
#define cSqrt2h_    0.707106781 // sqrt(0.5f);
#define cDC_     1e-30
#define cDCBig_     1e-20

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
	: AudioEffectX (audioMaster, 1, 6)	// 1 program, 6 parameters
{
    threshFader = 1;
    threshDB = -.1;
    thresh = 0.98851402;
    
    ratioFader = 0;
    ratio = 1;
    
    attackFader = 0.376060309; // ~20 ms
    attackSpls = 0.02 * getSampleRate(); // =20 ms
    attack = dB2Amp( threshDB / max( attackSpls , 0) );
    
    releaseFader = 0.430886938; // ~200ms
    releaseSpls = 0.2 * getSampleRate(); // =200 ms
    release = dB2Amp( threshDB / max( releaseSpls , 0) );

    b1EnvLP = -exp(-2.0*cPi_*7.952707288 / getSampleRate() );
    a0EnvLP = 1.0 + b1EnvLP;

    modeMakeUp = 0;
  
    outVolumeFader = .125; // ~ 0 dB
    outVolume = 1; // = 0 dB

    volume = 1; // = 0 dB

	setNumInputs (2);
	setNumOutputs (2);
	setUniqueID (CCONST ('D','V','C','L') );
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
    b1EnvLP = -exp(-2.0*cPi_*7.952707288 / getSampleRate() );
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
            threshDB = min( -90+ sqrt(sqrt(threshFader)) * 90 , -.1 );
            thresh = dB2Amp( threshDB );

            attack = dB2Amp( threshDB / attackSpls );
            release = dB2Amp( threshDB / releaseSpls );        

    // Avoid long fades if thresh to low and thus attack/release too slow...
            if ( threshDB >= -.5 )
                gain = seekGain = 1; 
        break;
        	
        case  1 : 
            ratioFader = value;
            ratio = 1 - sqrt(ratioFader);
        break;

		case  2 :
            attackFader = value;
            attackSpls = (attackFader*attackFader*attackFader*attackFader) * getSampleRate();
            attack = dB2Amp( threshDB / attackSpls );
        break;

		case  3 :
            releaseFader = value;
            releaseSpls = (releaseFader*releaseFader*releaseFader) * 2.5 * getSampleRate();
            release = dB2Amp( threshDB / releaseSpls );        
        break;
       
        case  4 :
            modeMakeUp = ceil(value);
        break;
        
		case  5 :
            outVolumeFader = value;
            outVolume = outVolumeFader * 8;
        break;
      
	}

    volume = outVolume / (modeMakeUp ? dB2Amp( threshDB -threshDB*ratio )  : 1 );

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case  0 : v = threshFader;      break;
		case  1 : v = ratioFader;       break;
		case  2 : v = attackFader;      break;
		case  3 : v = releaseFader;     break;
		case  4 : v = modeMakeUp;       break;
		case  5 : v = outVolumeFader;   break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case  0 : strcpy (label, "Threshold");     break;
		case  1 : strcpy (label, "Ratio");         break;
		case  2 : strcpy (label, "Attack");        break;
		case  3 : strcpy (label, "Release");       break;
		case  4 : strcpy (label, "Make-Up");       break;
		case  5 : strcpy (label, "Output");        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case  0 : dB2string (thresh, text);           break;
		case  1 : 
            if (ratio > 0) float2string ( 1 / ratio , text);
            else strcpy ( text, "oo");
            break;
		case  2 : ms2string (attackSpls, text);            break;
		case  3 : ms2string (releaseSpls, text);       break;
		case  4 :
                 if (modeMakeUp == 0) {strcpy (text, "OFF");}
            else if (modeMakeUp == 1) {strcpy (text, "ON");}
        break;
		case  5 : dB2string (outVolume, text);        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case  0 : strcpy (label, "dB");	    break;
		case  1 : strcpy (label, ":1");	    break;
		case  2 : strcpy (label, "ms");	    break;
		case  3 : strcpy (label, "ms");	    break;
		case  4 : strcpy (label, "Mode");   break;
		case  5 : strcpy (label, "dB");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Digital Versatile Compressor LITE");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Digital Versatile Compressor LITE");
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
// FEED: set detL/detR according to the feed
    rms = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    rms = sqrt( (tmpEnvLP = a0EnvLP*rms - b1EnvLP*tmpEnvLP + cDC_) - cDC_ );
    
// =========================================
// GR-TRACKER: Calculate the current gain to be applied
    if (rms > thresh) seekGain =  thresh * pow(rms/thresh,ratio ) / rms ;
    else seekGain = 1;

// =========================================
// GAIN FOLLOWER: make 'gain' follow seekGain with the specified sttack and release speed
    if (gain > seekGain) gain = max( gain*attack , seekGain );
    else gain = min( gain/release , seekGain );

// =========================================
// MIXING: ACCUMULATINNG!!!!!: mix output*volume + dry mix
    (*out1++) += (*in1++) * gain * volume;
    (*out2++) += (*in2++) * gain * volume;
    
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
    rms = sqrt( (tmpEnvLP = a0EnvLP*rms - b1EnvLP*tmpEnvLP + cDC_) - cDC_ );
    
// =========================================
// GR-TRACKER: Calculate the current gain to be applied
    if (rms > thresh) seekGain =  thresh * pow(rms/thresh,ratio ) / rms ;
    else seekGain = 1;

// =========================================
// GAIN FOLLOWER: make 'gain' follow seekGain with the specified sttack and release speed
    if (gain > seekGain) gain = max( gain*attack , seekGain );
    else gain = min( gain/release , seekGain );

// =========================================
// MIXING: REPLACING!!!!!: mix output*volume + dry mix
    (*out1++) = (*in1++) * gain * volume;
    (*out2++) = (*in2++) * gain * volume;
    
  } // eof while

}
