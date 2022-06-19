//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>

#define cAmpDB_ 8.656170245
#define cDcAdd_ 1e-30
#define cPi_ 3.141592654

inline float abs(float x){
    return ( x<0 ? -x:x);
}

inline float min (float x, float y){
    return ( x<y ? x:y);
}

inline float max (float x, float y){
    return ( x>y ? x:y);
}

#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 3)	// 1 program, 3 parameter biatch
{

    widthLP = 0.5;
    widthCoeffLP = 1;

    widthHP = 0.5;
    widthCoeffHP = 1;
    
    freqHPFader = .2751604;
    freqHP = 500;

    xHP = exp(-2.0*cPi_*freqHP/getSampleRate());
    a0HP = 1.0-xHP;
    b1HP = -xHP;

///

	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('S','E','N','H') );	// identify
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
    out1HP = out2HP = 0;
    tmp1HP = tmp2HP = 0;
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
        widthLP = value;
        widthCoeffLP = max(widthLP,1);
        break;
		
        case 1 : 
        freqHPFader = value;
        freqHP = freqHPFader*freqHPFader*freqHPFader*24000;
        xHP = exp(-2.0*cPi_*freqHP/getSampleRate());
        a0HP = 1.0-xHP;
        b1HP = -xHP;
        break;

		case 2 : 
        widthHP = value;
        widthCoeffHP = max(widthHP,1);
        break;
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = widthLP;       break;
		case 1 : v = freqHPFader;   break;
		case 2 : v = widthHP;       break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
     	switch (index)
	{
		case 0 : strcpy (label, "Width Lows");       break;
		case 1 : strcpy (label, "Crossover");        break;
		case 2 : strcpy (label, "Width Highs");      break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : float2string (widthLP*200, text);      break;
		case 1 : float2string (freqHP, text);           break;
		case 2 : float2string (widthHP*200, text);      break;
	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "%");	break;
		case 1 : strcpy (label, "Hz");	break;
		case 2 : strcpy (label, "%");	break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Stereo Enhancer");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Stereo Enhancer");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getVendorString (char* text)
{
	strcpy (text, "LOSER-Development");
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
    out1HP = (*in1);
    out2HP = (*in2);
   
    (*in1) = (tmp1HP = a0HP * (*in1) - b1HP * tmp1HP + cDcAdd_);
    (*in2) = (tmp2HP = a0HP * (*in2) - b1HP * tmp2HP + cDcAdd_);

    out1HP -= (*in1);
    out2HP -= (*in2);

    monoLP = ((*in1) + (*in2)) / 2.0f;
    stereoLP = (*in1) - (*in2);
    (*in1) = (monoLP + stereoLP * widthLP) / widthCoeffLP;
    (*in2) = (monoLP - stereoLP * widthLP) / widthCoeffLP;

    monoHP = (out1HP + out2HP) / 2.0f;
    stereoHP = out1HP - out2HP;
    out1HP = (monoHP + stereoHP * widthHP) / widthCoeffHP;
    out2HP = (monoHP - stereoHP * widthHP) / widthCoeffHP;

    (*out1++) = (*in1++) + out1HP;
    (*out2++) = (*in2++) + out2HP;

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
    out1HP = (*in1);
    out2HP = (*in2);
   
    (*in1) = (tmp1HP = a0HP * (*in1) - b1HP * tmp1HP + cDcAdd_);
    (*in2) = (tmp2HP = a0HP * (*in2) - b1HP * tmp2HP + cDcAdd_);

    out1HP -= (*in1);
    out2HP -= (*in2);

    monoLP = ((*in1) + (*in2)) / 2.0f;
    stereoLP = (*in1) - (*in2);
    (*in1) = (monoLP + stereoLP * widthLP) / widthCoeffLP;
    (*in2) = (monoLP - stereoLP * widthLP) / widthCoeffLP;

    monoHP = (out1HP + out2HP) / 2.0f;
    stereoHP = out1HP - out2HP;
    out1HP = (monoHP + stereoHP * widthHP) / widthCoeffHP;
    out2HP = (monoHP - stereoHP * widthHP) / widthCoeffHP;

    (*out1++) = (*in1++) + out1HP;
    (*out2++) = (*in2++) + out2HP;

    }
}
