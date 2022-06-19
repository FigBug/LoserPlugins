//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>
#include <stdlib.h>


// Mich's Formulas V 0.4

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
    return (x<0 ? -1:1);   
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
	: AudioEffectX (audioMaster, 1, 3)	// 1 program, 2 parameters
{

    thresh = threshFader = 1;
    
    releaseSpls = releaseFader = release = 0;
    release = dB2Amp( -6 / max( releaseSpls , 0) );
    
    ceiling = ceilingFader = 1;
  
    bufSize = (long) ( getSampleRate() );
    setInitialDelay(bufSize-1);
	buffer1 = new float[bufSize];
	buffer2 = new float[bufSize];

    gain1 = gain2 = volume = 1; // = 0 dB
 
	setNumInputs (2);
	setNumOutputs (2);
	setUniqueID (CCONST ('0','x','M','x') );
//    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

	suspend(); // flush buffers and reset variables
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{
	if (buffer1)
        delete[] buffer1;
	if (buffer2)
        delete[] buffer2;
}

void AVst::FlushBuffers ()
{
    memset (buffer1, 0, bufSize * sizeof(float));
    memset (buffer2, 0, bufSize * sizeof(float));
}

void AVst::ReallocBuffers ()
{
    buffer1 = (float*) realloc(buffer1, bufSize * sizeof(float));
    buffer2 = (float*) realloc(buffer2, bufSize * sizeof(float));
}

void AVst::suspend()
{
    sig1 = lastSig1 = sig2 = lastSig2 = length1 = length2 = bufPos = 0;
    if (bufSize != (long)( getSampleRate() ) )
    { 
        bufSize = (long)( getSampleRate() ) ;
        ReallocBuffers();
        setInitialDelay(bufSize);
    }
    FlushBuffers();
    
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
            thresh = dB2Amp ( -20 + threshFader * 20 );
        break;
        
        case 1 : 
            releaseFader = value;
            releaseSpls = pow(releaseFader,3) * 2.5 * getSampleRate();
            release = dB2Amp( -6 / releaseSpls ); 
        break;
        	
        case  2 : 
            ceilingFader = value;
            ceiling = dB2Amp ( -6 + ceilingFader * 6 );
        break;

	}

// MAKING THE AUTO-MAKE-UP TRANSITION BETWEEN RMS AND PEAK SMOOOOTHER ...
    volume = ceiling/thresh;

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case  0 : v = threshFader;      break;
		case  1 : v = releaseFader;      break;
		case  2 : v = ceilingFader;       break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case  0 : strcpy (label, "Threshold");     break;
		case  1 : strcpy (label, "Release");     break;
		case  2 : strcpy (label, "Ceiling");         break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case  0 : dB2string (thresh, text);           break;
		case  1 : ms2string (releaseSpls, text);           break;
		case  2 : dB2string (ceiling, text);        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case  0 : strcpy (label, "dB");	    break;
		case  1 : strcpy (label, "ms");	    break;
		case  2 : strcpy (label, "dB");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "0-X Maximizer");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "0-X Maximizer");
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

    buffer1[bufPos] = (*in1);
    buffer2[bufPos] = (*in2);

    sig1 = (*in1)>0 ? 1:0 ;
    sig2 = (*in2)>0 ? 1:0 ;

    maxSpl1 = max( maxSpl1, abs(*in1++) );
    maxSpl2 = max( maxSpl2, abs(*in2++) );

    if (sig1 != lastSig1)
    {
        gain1 = min( maxSpl1 > thresh ? thresh/maxSpl1 : 1 , gain1);
        length1++;
        while(length1 >= 0)
        {
            buffer1[ bufPos - length1 + ((bufPos-length1)<0 ? bufSize+1:0) ] *= gain1;        
            length1--;
        }
        length1 = -1;
        maxSpl1 = 0;             
    }
    if((length1+=1) > bufSize) {length1 = 0; maxSpl1 = 0; }
    lastSig1 = sig1;

    if (sig2 != lastSig2)
    {
        gain2 = min( maxSpl2 > thresh ? thresh/maxSpl2 : 1 , gain2);
        length2++;                
        while(length2 >= 0)
        {
            buffer2[ bufPos - length2 + ((bufPos-length2)<0 ? bufSize+1:0) ] *= gain2;        
            length2--;
        }
        length2 = -1;
        maxSpl2 = 0;             
    }
    if((length2+=1) > bufSize) {length2 = 0; maxSpl2 = 0; }
    lastSig2 = sig2;


    if((bufPos+=1) >= bufSize) bufPos = 0;

    gain1 = releaseFader ? min(gain1/release , 1) : 1;
    gain2 = releaseFader ? min(gain2/release , 1) : 1;
   
    (*out1++) += limit(buffer1[bufPos]*volume,ceiling);
    (*out2++) += limit(buffer2[bufPos]*volume,ceiling); 
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

    buffer1[bufPos] = (*in1);
    buffer2[bufPos] = (*in2);

    sig1 = (*in1)>0 ? 1:0 ;
    sig2 = (*in2)>0 ? 1:0 ;

    maxSpl1 = max( maxSpl1, abs(*in1++) );
    maxSpl2 = max( maxSpl2, abs(*in2++) );

    if (sig1 != lastSig1)
    {
        gain1 = min( maxSpl1 > thresh ? thresh/maxSpl1 : 1 , gain1);
        while(length1 >= 0)
        {
            buffer1[ bufPos - length1 + ((bufPos-length1)<0 ? bufSize:0) ] *= gain1;        
            length1--;
        }
        length1 = -1;
        maxSpl1 = 0;             
    }
    if((length1+=1) >= bufSize) {length1 = 0; maxSpl1 = 0; }
    lastSig1 = sig1;

    if (sig2 != lastSig2)
    {
        gain2 = min( maxSpl2 > thresh ? thresh/maxSpl2 : 1 , gain2);              
        while(length2 >= 0)
        {
            buffer2[ bufPos - length2 + ((bufPos-length2)<0 ? bufSize:0) ] *= gain2;        
            length2--;
        }
        length2 = -1;
        maxSpl2 = 0;             
    }
    if((length2+=1) >= bufSize) {length2 = 0; maxSpl2 = 0; }
    lastSig2 = sig2;


    if((bufPos+=1) >= bufSize) bufPos = 0;

    gain1 = releaseFader ? min(gain1/release , 1) : 1;
    gain2 = releaseFader ? min(gain2/release , 1) : 1;
   
    (*out1++) = limit(buffer1[bufPos]*volume,10);
    (*out2++) = limit(buffer2[bufPos]*volume,10);
    
  } // eof while

}
