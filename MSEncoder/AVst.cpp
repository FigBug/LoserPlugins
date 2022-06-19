//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 0)	// 1 program, 0 parameters
{

	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('M','S','N','C') );	// identify
//	canMono ();				// makes sense to feed both inputs with the same signal
	canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{
	// nothing to do here
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

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{

}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "M/S Encoder");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "M/S Encoder");
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
        (*out1++) += ((*in1)+(*in2))/2 ;    // accumulating
        (*out2++) += ((*in2++)-(*in1++))/2 ;
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
        (*out1++) = ((*in1)+(*in2))/2 ;    // replacing
        (*out2++) = ((*in2++)-(*in1++))/2 ;
    }
}
