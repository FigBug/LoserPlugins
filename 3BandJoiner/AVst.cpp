//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>

// Mich's Formulas V 0.3

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
	: AudioEffectX (audioMaster, 1, 3)	// 1 program, 5 parameter only
{

    ch12Vol = ch34Vol = ch56Vol = outVol = 1;
    ch12VolF = ch34VolF = ch56VolF = 0.5f;

///

	setNumInputs (6);		// 3 x stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('3','B','J','N') );
	canMono ();				// makes sense to feed both inputs with the same signal
	canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

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

    switch (index)
	{
		case 0 : 
        ch12VolF = value;
        ch12Vol = dB2Amp( sign( ch12VolF-.5)* sqr((ch12VolF -.5)*2)*18 );
        break;

		case 1 : 
        ch34VolF = value;
        ch34Vol = dB2Amp( sign( ch34VolF-.5)* sqr((ch34VolF -.5)*2)*18 );;
        break;

		case 2 : 
        ch56VolF = value;
        ch56Vol = dB2Amp( sign( ch56VolF-.5)* sqr((ch56VolF -.5)*2)*18 );;
        break;

	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = ch12VolF;   break;
		case 1 : v = ch34VolF;   break;
		case 2 : v = ch56VolF;   break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
     	switch (index)
	{
		case 0 : strcpy (label, "Low Band");  break;
		case 1 : strcpy (label, "Mid Band");  break;
		case 2 : strcpy (label, "High Band");  break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : float2string (amp2DB(ch12Vol), text);      break;
		case 1 : float2string (amp2DB(ch34Vol), text);      break;
		case 2 : float2string (amp2DB(ch56Vol), text);      break;
	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "dB");	break;
		case 1 : strcpy (label, "dB");	break;
		case 2 : strcpy (label, "dB");	break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "3-Band Joiner");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "3-Band Joiner");
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
    float *in3  =  inputs[2];
    float *in4  =  inputs[3];
    float *in5  =  inputs[4];
    float *in6  =  inputs[5];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    while (--sampleFrames >= 0)
    {
        
    (*out1++) += ((*in1++)*ch12Vol+(*in3++)*ch34Vol+(*in5++)*ch56Vol) * outVol;
    (*out2++) += ((*in2++)*ch12Vol+(*in4++)*ch34Vol+(*in6++)*ch56Vol) * outVol;
    
    }
}

//-----------------------------------------------------------------------------------------
void AVst::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *in3  =  inputs[2];
    float *in4  =  inputs[3];
    float *in5  =  inputs[4];
    float *in6  =  inputs[5];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    while (--sampleFrames >= 0)
    {

    (*out1++) = ((*in1++)*ch12Vol+(*in3++)*ch34Vol+(*in5++)*ch56Vol) * outVol;
    (*out2++) = ((*in2++)*ch12Vol+(*in4++)*ch34Vol+(*in6++)*ch56Vol) * outVol;

    }
}
