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
	: AudioEffectX (audioMaster, 1, 9)	// 1 program, 9 parameters
{

// <sof setting default parameters>

    delayFader = 0.341995189;
    delayMaxSize = delaySize = 2*static_cast<unsigned long>( 0.2 * getSampleRate() );
    delayBuffer = new float[delayMaxSize];
        
    mixIn = 0.707106781;

    feedback = .5;

    freqLPFader = 1;
    freqHPFader = 0;

    freqLP = 24000;
    freqHP = 0;

    xLP = exp(-2.0*cPi_*freqLP/getSampleRate());
    a0LP = 1.0-xLP;
    b1LP = -xLP;

    xHP = exp(-2.0*cPi_*freqHP/getSampleRate());
    a0HP = 1.0-xHP;
    b1HP = -xHP;
    
    trim  = 0;

    dry = 1;

    wet = .5;

    outVolumeFader = .5;
    outVolume = 1;

// <sof> setting the VST stuff

	setNumInputs (2);		// stereo in
	setNumOutputs (2);
	setUniqueID ( CCONST ('D','L','A','Y') );
//    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

// call suspend to flush buffer
	
	suspend(); // flush buffers and reset variables
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{
	if (delayBuffer)
        delete[] delayBuffer;
}

void AVst::FlushBuffer ()
{
    memset (delayBuffer, 0, delaySize * sizeof(float));
}

void AVst::ReallocBuffer ()
{
    delayBuffer = (float*) realloc(delayBuffer, delayMaxSize * sizeof(float));
}

void AVst::suspend()
{
    delayBufPos = 0;
    if (delayMaxSize != delaySize)
    { 
        delayMaxSize = delaySize;
        ReallocBuffer();
    }
    FlushBuffer();
    out1LP = out2LP = out1HP = out2HP = 0;
    tmp1LP = tmp2LP = tmp1HP = tmp2HP = 0;
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
		case 0 : 
            delayFader = value;
            delaySize = 2* static_cast<unsigned long>( max( pow(delayFader,3) * 5* getSampleRate()  ,1) );
            if (delaySize > delayMaxSize)
            {
                delayMaxSize = 2* static_cast<unsigned long>( max( 5 * getSampleRate()  ,1) ); // Set 'delayMaxSize' to max possible size !!!
                ReallocBuffer();
            }
            FlushBuffer();
            delayBufPos = 0;
        break;
        	
        case 1 : 
            mixIn = value;
        break;

        case 2 : 
            feedback = value;
        break;
        
        case 3 : 
        freqLPFader = value;
        freqLP = freqLPFader*freqLPFader*freqLPFader*24000;
        xLP = exp(-2.0*cPi_*freqLP/getSampleRate());
        a0LP = 1.0-xLP;
        b1LP = -xLP;
        break;

		case 4 : 
        freqHPFader = value;
        freqHP = freqHPFader*freqHPFader*freqHPFader*24000;
        xHP = exp(-2.0*cPi_*freqHP/getSampleRate());
        a0HP = 1.0-xHP;
        b1HP = -xHP;
        break;

		case 5 :
            trim = floor(value);
        break;

		case 6 :
            dry = value;
        break;

		case 7 :
            wet = value;
        break;

		case 8 : 
            outVolumeFader = value;
            outVolume = exp( (outVolumeFader-.5)*24 / cAmpDB_);
        break;

// Debug Fader
//        case 9 : break;
        
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
// use the SDK's v variable and return the xxxFader values, since me is lazy and don't want to convert back, hehe
    float v = 0;
	switch (index)
	{
		case 0: v = delayFader;       break;
		case 1 : v = mixIn;           break;
		case 2 : v = feedback;        break;
		case 3 : v = freqLPFader;     break;
		case 4 : v = freqHPFader;     break;
		case 5 : v = trim;            break;
		case 6 : v = dry;             break;
		case 7 : v = wet;             break;
		case 8 : v = outVolumeFader;  break;
// Debug Fader		
//		case 9 : v = delayMaxSize;  break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case 0 : strcpy (label, "Delay");     break;
		case 1 : strcpy (label, "Mix In");    break;
		case 2 : strcpy (label, "Feedback");  break;
		case 3 : strcpy (label, "Low-Pass");  break;
		case 4 : strcpy (label, "High-Pass"); break;
		case 5 : strcpy (label, "Trim");      break;
		case 6 : strcpy (label, "Dry");       break;
		case 7 : strcpy (label, "Wet");       break;
		case 8 : strcpy (label, "Output");    break;
// Debug Fader		
//		case 9 : strcpy (label, "DELAY MAX SIZE");    break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : ms2string    ( delaySize/2, text);                   break;
		case 1 : dB2string ( mixIn , text);           break;
		case 2 : dB2string ( feedback , text);        break;
		case 3 : float2string (freqLP, text);   break;
		case 4 : float2string (freqHP, text);   break;
		case 5 :
            if (trim == 0) strcpy (text, "MIX");
            else if (trim == 1) strcpy (text, "FBQ");
        break;
		case 6 : dB2string ( dry , text);       break;
		case 7 : dB2string ( wet , text);       break;
		case 8 : dB2string ( outVolume , text);       break;
// Debug Fader, hehe
//		case 9 : float2string (delayMaxSize, text);         break;
	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "ms");	    break;
		case 1 : strcpy (label, "dB");	    break;
		case 2 : strcpy (label, "dB");	    break;
		case 3 : strcpy (label, "Hz");	    break;
		case 4 : strcpy (label, "Hz");	    break;
		case 5 : strcpy (label, "Mode");    break;
		case 6 : strcpy (label, "dB");	    break;
		case 7 : strcpy (label, "dB");	    break;
		case 8 : strcpy (label, "dB");	    break;
// Debug Fader		
//		case 9 : strcpy (label, "Debug");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Delay");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Delay");
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

    if (trim == 0)
    {
        del1 = (*in1);
        del2 = (*in2);
    }
    else if (trim == 1)
    {
        del1 = (*in1)*mixIn + delayBuffer[delayBufPos]*feedback;
        del2 = (*in2)*mixIn + delayBuffer[delayBufPos+1]*feedback;
    }

    if (freqLPFader != 1)
    {
        (del1) = (tmp1LP = a0LP * (del1) - b1LP * tmp1LP + cDcAdd_) - cDcAdd_;
        (del2) = (tmp2LP = a0LP * (del2) - b1LP * tmp2LP + cDcAdd_) - cDcAdd_;
    }
    if (freqHPFader != 0)
    {
        (del1) -= (tmp1HP = a0HP * (del1) - b1HP * tmp1HP + cDcAdd_) - cDcAdd_;
        (del2) -= (tmp2HP = a0HP * (del2) - b1HP * tmp2HP + cDcAdd_) - cDcAdd_;  
    }
        
    if (trim == 0)
    {
        del1 = (del1)*mixIn + delayBuffer[delayBufPos]*feedback;
        del2 = (del2)*mixIn + delayBuffer[delayBufPos+1]*feedback;
    }
           
    delayBuffer[delayBufPos] = (del1);
    delayBuffer[delayBufPos+1] = (del2);

    if ( (delayBufPos+=2) >= delaySize) delayBufPos = 0;

// ACCUMULATING!!!
        (*out1++) += ((*in1++)*dry + delayBuffer[delayBufPos]*wet) * outVolume;
        (*out2++) += ((*in2++)*dry + delayBuffer[delayBufPos+1]*wet) * outVolume;

    }
       
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

    if (trim == 0)
    {
        del1 = (*in1);
        del2 = (*in2);
    }
    else if (trim == 1)
    {
        del1 = (*in1)*mixIn + delayBuffer[delayBufPos]*feedback;
        del2 = (*in2)*mixIn + delayBuffer[delayBufPos+1]*feedback;
    }

    if (freqLPFader != 1)
    {
        (del1) = (tmp1LP = a0LP * (del1) - b1LP * tmp1LP + cDcAdd_) - cDcAdd_;
        (del2) = (tmp2LP = a0LP * (del2) - b1LP * tmp2LP + cDcAdd_) - cDcAdd_;
    }
    if (freqHPFader != 0)
    {
        (del1) -= (tmp1HP = a0HP * (del1) - b1HP * tmp1HP + cDcAdd_) - cDcAdd_;
        (del2) -= (tmp2HP = a0HP * (del2) - b1HP * tmp2HP + cDcAdd_) - cDcAdd_;  
    }
        
    if (trim == 0)
    {
        del1 = (del1)*mixIn + delayBuffer[delayBufPos]*feedback;
        del2 = (del2)*mixIn + delayBuffer[delayBufPos+1]*feedback;
    }

    delayBuffer[delayBufPos] = (del1);
    delayBuffer[delayBufPos+1] = (del2);

    if ( (delayBufPos+=2) >= delaySize) delayBufPos = 0;

// REPLACING!!!!!
        (*out1++) = ((*in1++)*dry + delayBuffer[delayBufPos]*wet) * outVolume;
        (*out2++) = ((*in2++)*dry + delayBuffer[delayBufPos+1]*wet) * outVolume;

    }

}
