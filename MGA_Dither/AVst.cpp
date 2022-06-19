/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */


#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>


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



#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 6)	// 1 program, 6 parameters
{

// <sof setting default parameters>

    bitDepthFader = 0.428571428;
    bitDepth = 15;
    bit = pow(2,bitDepth);
        
    ditherType = 2;

    ditherShaping = 0;

    ditherAmp = 2;

    dcShift = 0.5;
    
    noiseShaping = 0;


// <sof> setting the VST stuff

	setNumInputs (2);		// stereo in
	setNumOutputs (2);
	setUniqueID (CCONST ('D','I','T','H') );
    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name
	
	resume();
	suspend();

}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

}

void AVst::suspend()
{
    e1l = e2l = e1r = e2r = 0;
}

void AVst::resume ()
{
    srand( time( 0 ) ); // init Rand() function    
//	AudioEffectX::resume ();
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
            bitDepthFader = value;
            bitDepth = 3 + floor(value * 14)*2;
            bit = pow(2,bitDepth);
        break;

		case 1 : 
            ditherType = floor(value * 3);
        break;

		case 2 : 
            ditherShaping = ceil(value);
        break;

		case 3 : 
            ditherAmp = value * 4;
        break;

		case 4 : 
            dcShift = (value-0.5f)*4 + 0.5f;
        break;

		case 5 : 
            noiseShaping = ceil(value)*.5;
        break;

    }

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = bitDepthFader;      break;
		case 1 : v = ditherType/3;       break;
		case 2 : v = ditherShaping;      break;
		case 3 : v = ditherAmp/4;        break;
		case 4 : v = (dcShift+1.5f)/4;      break;
		case 5 : v = noiseShaping*2;     break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case 0 : strcpy (label, "Bit Depth");         break;
		case 1 : strcpy (label, "Dither");            break;
		case 2 : strcpy (label, "Dither Shaping");    break;
		case 3 : strcpy (label, "Dither Amplitude");  break;
		case 4 : strcpy (label, "DC Shift");          break;
		case 5 : strcpy (label, "Noise Shaping");     break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : float2string (bitDepth+1, text);        break;
		
        case 1 : 
            if (ditherType == 0) strcpy (text, "No");
            else if (ditherType == 1) strcpy (text, "RPDF");
            else if (ditherType == 2) strcpy (text, "TPDF");
            else if (ditherType == 3) strcpy (text, "Gaussian");
        break;
        
        case 2 :
            if (ditherShaping == 0) strcpy (text, "No");
            else if (ditherShaping == 1) strcpy (text, "HP");
        break;
     
		case 3 : float2string (ditherAmp, text);        break;

		case 4 : float2string (dcShift-0.5f, text);        break;
				
        case 5 :
            if (noiseShaping == 0) strcpy (text, "No");
            else if (noiseShaping == 0.5) strcpy (text, "Simple");
        break;
	}

e1l = e2l = e1r = e2r = 0;
r1=r2=0;

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "Bit");	    break;
		case 1 : strcpy (label, "Type");    break;
		case 2 : strcpy (label, "Type");    break;
		case 3 : strcpy (label, "lsb");     break;
		case 4 : strcpy (label, "lsb");     break;
		case 5 : strcpy (label, "Type");    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Dither");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Dither");
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


// ACCUMULATING!!!!!  
        (*out1++) += (*in1++);
        (*out2++) += (*in2++);


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
        r2=r1; 
        if (ditherType == 0) r1=r2=0;
        else if (ditherType == 1)
        {
            r1 = rand()/ static_cast<float>(RAND_MAX) -0.5f;
        }
        else if (ditherType == 2)
        {
            r1 = ( rand()/ static_cast<float>(RAND_MAX) + rand()/ static_cast<float>(RAND_MAX) )/2 -0.5f;
        }
        else if (ditherType == 3)
        {
            r1 = rand()/ static_cast<float>(RAND_MAX) + 
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX) +
                 rand()/ static_cast<float>(RAND_MAX);
            r1 = r1 / 10 -0.5f;
        }
        if (ditherShaping == 0) random = r1;
        else if (ditherShaping == 1) random = (r1-r2);

        (*in1) += noiseShaping * (e1l + e1l - e2l); // error feedback
        (*in2) += noiseShaping * (e1r + e1r - e2r); // error feedback

        noise = (random)*ditherAmp + dcShift;

        (*out1) = max(min(floor((*in1)*bit+noise)/bit,1),-1);
        (*out2) = max(min(floor((*in2)*bit+noise)/bit,1),-1);
        
        e2l = e1l;
        e2r = e1r;

        e1l = (*in1++) - (*out1++);           //error
        e1r = (*in2++) - (*out2++);           //error

    }

}
