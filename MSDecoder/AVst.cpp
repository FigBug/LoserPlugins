//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>

#define cAmpDB_ 8.656170245

inline float abs(float x)
{
    return ( x<0 ? -x:x);
}

#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 4)	// 1 program, 4 parameters
{
    midVolFader = .5;
    midVol = midVolFader * midVolFader * 4;
	
    sideVolFader = .5;
    sideVol = sideVolFader * sideVolFader * 4;

    panFader = .5;
    pan = panFader * 2 - 1;
    
    swapFader = 0;
    swap = 1 - ceil(swapFader) * 2;

///

	setNumInputs (2);		// stereo in + stereo aux
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('M','S','D','E') );	// identify
//	canMono ();				// makes sense to feed both inputs with the same signal
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
        midVolFader = value;
        midVol = midVolFader * midVolFader * 4;
        break;
		
        case 1 : 
        sideVolFader = value;
        sideVol = sideVolFader * sideVolFader * 4 * swap;
        break;

		case 2 :
        panFader = value;
        pan = panFader * 2 - 1;
        break;

		case 3 : 
        swapFader = value;
        swap = 1 - ceil(swapFader) * 2 ;
        sideVol = sideVolFader * sideVolFader * 4 * swap;
        break;

	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = midVolFader; break;
		case 1 : v = sideVolFader; break;
		case 2 : v = panFader; break;
		case 3 : v = swapFader; break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
     	switch (index)
	{
		case 0 : strcpy (label, "Mid"); break;
		case 1 : strcpy (label, "Side"); break;
		case 2 : strcpy (label, "Balance"); break;
		case 3 : strcpy (label, "Swap Left/Right"); break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : dB2string (midVol, text); break;
		case 1 : dB2string (abs(sideVol), text); break;
		case 2 : float2string (pan*100, text); break;
		case 3 : 
            if (swap == 1)
                strcpy (text, "NO");
            else
                strcpy (text, "YES");
        break;
	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "dB");	break;
		case 1 : strcpy (label, "dB");	break;
		case 2 : strcpy (label, "%");	break;
		case 3 : strcpy (label, "Mode");	break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "M/S Decoder");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "M/S Decoder");
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

    mid = (*in1++) * midVol;
    side = (*in2++) * sideVol;
    (*out1++) += (mid -  side) * (pan > 0 ? 1-pan:1);
    (*out2++) += (mid +  side) * (pan < 0 ? 1+pan:1);

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

    mid = (*in1++) * midVol;
    side = (*in2++) * sideVol;
    (*out1++) = (mid -  side) * (pan > 0 ? 1-pan:1);
    (*out2++) = (mid +  side) * (pan < 0 ? 1+pan:1);

    }
}
