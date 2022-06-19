//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>




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
	: AudioEffectX (audioMaster, 1, 4)	// 1 program, 4 parameter only
{


    freqHPFader = 0.75912577;
    freqHP = 7000;

    xHP = exp(-2.0*cPi_*freqHP/getSampleRate());
    a0HP = 1.0-xHP;
    b1HP = -xHP;

    clipBoost = 1;
    harmDistr = harmDistrFader = 0;

    mixIn = 0.5;

///

	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('X','I','T','E') );	// identify
//	canMono ();				// makes sense to feed both inputs with the same signal
	canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

    suspend();
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

}

void AVst::suspend()
{
    tmp1HP1 = tmp2HP1 = 0;
    tmp1HP2 = tmp2HP2 = 0;
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
        freqHPFader = value;
        freqHP = 200+sqr(freqHPFader)*11800;
        xHP = exp(-2.0*cPi_*freqHP/getSampleRate());
        a0HP = 1.0-xHP;
        b1HP = -xHP;
        break;

		case 1 : 
        clipBoost = 1+value * 7;
        break;

		case 2 : 
        harmDistrFader = value;
        harmDistr = 2*harmDistrFader*.99/(1-sqrt(harmDistrFader*.99));        
        break;

		case 3 : 
        mixIn = value*2;
        break;
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = freqHPFader;     break;
		case 1 : v = (clipBoost-1)/7;     break;
		case 2 : v = harmDistrFader;  break;
		case 3 : v = mixIn/2;         break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
     	switch (index)
	{
		case 0 : strcpy (label, "High-Pass");   break;
		case 1 : strcpy (label, "Clip Boost");  break;
		case 2 : strcpy (label, "Harmonics");   break;
		case 3 : strcpy (label, "Mix In");      break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : float2string (freqHP, text);                 break;
		case 1 : dB2string (clipBoost, text); break;
		case 2 : float2string (harmDistrFader*100, text);     break;
		case 3 : dB2string (mixIn, text);     break;

	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "Hz");	break;
		case 1 : strcpy (label, "dB");	break;
		case 2 : strcpy (label, "%");	break;
		case 3 : strcpy (label, "dB");	break;

	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Exciter");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Exciter");
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
    s1 = (*in1);
    s2 = (*in2);
       
    s1 -= (tmp1HP1 = a0HP * s1 - b1HP * tmp1HP1 + cDcAdd_) - cDcAdd_;
    s2 -= (tmp2HP1 = a0HP * s2 - b1HP * tmp2HP1 + cDcAdd_) - cDcAdd_;

    s1 = min(max(s1*clipBoost,-1),1);
    s2 = min(max(s2*clipBoost,-1),1);

    s1 = (1+harmDistr)*s1/(1+harmDistr*abs(s1));
    s2 = (1+harmDistr)*s2/(1+harmDistr*abs(s2));

    s1 -= (tmp1HP2 = a0HP * s1 - b1HP * tmp1HP2 + cDcAdd_) - cDcAdd_;
    s2 -= (tmp2HP2 = a0HP * s2 - b1HP * tmp2HP2 + cDcAdd_) - cDcAdd_;
    
    (*out1++) += (*in1++) + s1 * mixIn;
    (*out2++) += (*in2++) + s2 * mixIn;
    
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
    s1 = (*in1);
    s2 = (*in2);
       
    s1 -= (tmp1HP1 = a0HP * s1 - b1HP * tmp1HP1 + cDcAdd_) - cDcAdd_;
    s2 -= (tmp2HP1 = a0HP * s2 - b1HP * tmp2HP1 + cDcAdd_) - cDcAdd_;

    s1 = min(max(s1*clipBoost,-1),1);
    s2 = min(max(s2*clipBoost,-1),1);

    s1 = (1+harmDistr)*s1/(1+harmDistr*abs(s1));
    s2 = (1+harmDistr)*s2/(1+harmDistr*abs(s2));

    s1 -= (tmp1HP2 = a0HP * s1 - b1HP * tmp1HP2 + cDcAdd_) - cDcAdd_;
    s2 -= (tmp2HP2 = a0HP * s2 - b1HP * tmp2HP2 + cDcAdd_) - cDcAdd_;
    
    (*out1++) = (*in1++) + s1 * mixIn;
    (*out2++) = (*in2++) + s2 * mixIn;

    }
}
