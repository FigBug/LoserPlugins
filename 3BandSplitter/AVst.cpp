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
	: AudioEffectX (audioMaster, 1, 4)	// 1 program, 2 parameter only
{

    freqLPFader = 0.20105025;
    freqLP = min(5+freqLPFader*freqLPFader*freqLPFader*23995,(getSampleRate()/2)-10);
    freqHPFader = 0.436456249;
    freqHP = min(5+freqHPFader*freqHPFader*freqHPFader*23995,(getSampleRate()/2)-10);

    
    slopeHP = 0;
    slopeLP = 1;
        
    xLP6 = exp(-2.0*cPi_*freqLP/getSampleRate());
    a0LP6 = 1.0-xLP6;
    b1LP6 = -xLP6;

    xHP6 = exp(-2.0*cPi_*freqHP/getSampleRate());
    a0HP6 = 1.0-xHP6;
    b1HP6 = -xHP6;
       
    w0HP = 2 * cPi_ * freqHP/getSampleRate();
    cosw0HP = cos(w0HP);
    sinw0HP = sin(w0HP);
    alphaHP = sinw0HP / cSqrt2_;
    b0HP = (1 + cosw0HP)/2;
    b1HP = -(1 + cosw0HP);
    b2HP = (1 + cosw0HP)/2;
    a0HP = 1 + alphaHP;
    a1HP = -2 * cosw0HP;
    a2HP = 1 - alphaHP;
    b0HP /= a0HP;
    b1HP /= a0HP;
    b2HP /= a0HP;
    a1HP /= a0HP;
    a2HP /= a0HP;

    w0LP = 2 * cPi_ * freqLP/getSampleRate();
    cosw0LP = cos(w0LP);
    sinw0LP = sin(w0LP);
    alphaLP = sinw0LP / cSqrt2_;
    b0LP = (1 - cosw0LP)/2;
    b1LP = (1 - cosw0LP);
    b2LP = (1 - cosw0LP)/2;
    a0LP = 1 + alphaLP;
    a1LP = -2 * cosw0LP;
    a2LP = 1 - alphaLP;
    b0LP /= a0LP;
    b1LP /= a0LP;
    b2LP /= a0LP;
    a1LP /= a0LP;
    a2LP /= a0LP;


///

	setNumInputs (2);		// stereo in
	setNumOutputs (6);		// stereo out
	setUniqueID (CCONST ('3','B','S','P') );
	canMono ();				// makes sense to feed both inputs with the same signal
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
    tmp1LP6 = tmp2LP6 = tmp1HP6 = tmp2HP6 = 0;
  
    xl1LP = xl2LP = yl1LP = yl2LP =
    xr1LP = xr2LP = yr1LP = yr2LP =
    xl1HP = xl2HP = yl1HP = yl2HP =
    xr1HP = xr2HP = yr1HP = yr2HP = 0;

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
        freqLPFader = value;
        freqLP = min(5+freqLPFader*freqLPFader*freqLPFader*23995,(getSampleRate()/2)-10);
        w0LP = 2 * cPi_ * freqLP/getSampleRate();
        cosw0LP = cos(w0LP);
        sinw0LP = sin(w0LP);
        alphaLP = sinw0LP / cSqrt2_;
        b0LP = (1 - cosw0LP)/2;
        b1LP = (1 - cosw0LP);
        b2LP = (1 - cosw0LP)/2;
        a0LP = 1 + alphaLP;
        a1LP = -2 * cosw0LP;
        a2LP = 1 - alphaLP;
        b0LP /= a0LP;
        b1LP /= a0LP;
        b2LP /= a0LP;
        a1LP /= a0LP;
        a2LP /= a0LP;
        
        xLP6 = exp(-2.0*cPi_*freqLP/getSampleRate());
        a0LP6 = 1.0-xLP6;
        b1LP6 = -xLP6;
        break;
        
        case 1 :
            slopeLP = ceil(value);
        break;
    
        case 2 :
        freqHPFader = value;
        freqHP = min(5+freqHPFader*freqHPFader*freqHPFader*23995,(getSampleRate()/2)-10);
        w0HP = 2 * cPi_ * freqHP/getSampleRate();
        cosw0HP = cos(w0HP);
        sinw0HP = sin(w0HP);
        alphaHP = sinw0HP / cSqrt2_;
        b0HP = (1 + cosw0HP)/2;
        b1HP = -(1 + cosw0HP);
        b2HP = (1 + cosw0HP)/2;
        a0HP = 1 + alphaHP;
        a1HP = -2 * cosw0HP;
        a2HP = 1 - alphaHP;
        b0HP /= a0HP;
        b1HP /= a0HP;
        b2HP /= a0HP;
        a1HP /= a0HP;
        a2HP /= a0HP;

        xHP6 = exp(-2.0*cPi_*freqHP/getSampleRate());
        a0HP6 = 1.0-xHP6;
        b1HP6 = -xHP6;
        break;

        case 3 :
            slopeHP = ceil(value);
        break;
            
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = freqLPFader;   break;
		case 1 : v = slopeLP;        break;
		case 2 : v = freqHPFader;   break;
		case 3 : v = slopeHP;        break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
     	switch (index)
	{
		case 0 : strcpy (label, "Low-Mid Crossover"); break;
		case 1 : strcpy (label, "Slope");               break;
		case 2 : strcpy (label, "Mid-High Crossover");  break;
		case 3 : strcpy (label, "Slope");               break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 :
            float2string (freqLP, text);
        break;
		case 1 :
            if (slopeLP == 0) strcpy (text, "6dB");
            else if (slopeLP == 1) strcpy (text, "12dB");
        break;
		case 2 :
            float2string (freqHP, text);
        break;
		case 3 :
            if (slopeHP == 0) strcpy (text, "6dB");
            else if (slopeHP == 1) strcpy (text, "12dB");
        break;

	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "Hz");	break;
		case 1 : strcpy (label, "/Oct");	break;
		case 2 : strcpy (label, "Hz");	break;
		case 3 : strcpy (label, "/Oct");	break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "3-Band Splitter");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "3-Band Splitter");
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
    float *out3 = outputs[2];
    float *out4 = outputs[3];
    float *out5 = outputs[4];
    float *out6 = outputs[5];
    while (--sampleFrames >= 0)
    {



    }
}

//-----------------------------------------------------------------------------------------
void AVst::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
    float *out3 = outputs[2];
    float *out4 = outputs[3];
    float *out5 = outputs[4];
    float *out6 = outputs[5];

    while (--sampleFrames >= 0)
    {

    (*in1) += cDC_;
    (*in2) += cDC_;

        if (slopeLP == 1)
        {
            tmp = (*in1);
            (*out1) = b0LP * (*in1) + b1LP * xl1LP + b2LP * xl2LP - a1LP * yl1LP - a2LP * yl2LP;
            xl2LP = xl1LP; xl1LP = tmp; yl2LP = yl1LP; yl1LP = (*out1);
            
            tmp = (*in2);
            (*out2) = b0LP * (*in2) + b1LP * xr1LP + b2LP * xr2LP - a1LP * yr1LP - a2LP * yr2LP;
            xr2LP = xr1LP; xr1LP = tmp; yr2LP = yr1LP; yr1LP = (*out2);
        }
        if (slopeLP == 0)
        {
            (*out1)  = (tmp1LP6 = a0LP6 * (*in1) - b1LP6 * tmp1LP6 ) ;
            (*out2)  = (tmp2LP6 = a0LP6 * (*in2) - b1LP6 * tmp2LP6 ) ;   
        }

        if (slopeHP == 1)
        {
            tmp = (*in1);
            (*out5) = b0HP * (*in1) + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
            xl2HP = xl1HP; xl1HP = tmp; yl2HP = yl1HP; yl1HP = (*out5);
            
            tmp = (*in2);
            (*out6) = b0HP * (*in2) + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
            xr2HP = xr1HP; xr1HP = tmp; yr2HP = yr1HP; yr1HP = (*out6);
        }
        if (slopeHP == 0)
        {
            (*out5) = (*in1) - (tmp1HP6 = a0HP6 * (*in1) - b1HP6 * tmp1HP6 ) ;
            (*out6) = (*in2) - (tmp2HP6 = a0HP6 * (*in2) - b1HP6 * tmp2HP6 ) ;            
        }
       
        (*out3++) = (*in1++) - (*out1++) - (*out5++);
        (*out4++) = (*in2++) - (*out2++) - (*out6++);

    }
}
