//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __AVST_H
#define __AVST_H

#include "audioeffectx.h"

//-------------------------------------------------------------------------------------------------------
class AVst : public AudioEffectX
{
public:
	AVst (audioMasterCallback audioMaster);
	~AVst ();

	// Processes
	virtual void process (float **inputs, float **outputs, long sampleFrames);
	virtual void processReplacing (float **inputs, float **outputs, long sampleFrames);

	// Program
	virtual void setProgramName (char *name);
	virtual void getProgramName (char *name);

	// Parameters
	virtual void setParameter (long index, float value);
	virtual float getParameter (long index);
	virtual void getParameterLabel (long index, char *label);
	virtual void getParameterDisplay (long index, char *text);
	virtual void getParameterName (long index, char *text);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long getVendorVersion () { return 1000; }
	
	virtual VstPlugCategory getPlugCategory () { return kPlugCategEffect; }

protected:
  
    float rotation, rotationFader;
    
    float mono, stereo, angle, radius;
    float s1, s2;

    float width1, widthCoeff1;
    float center1, side1, centerFader1;
    float width2, widthCoeff2;
    float center2, side2, centerFader2;
    
    float pan1, pan1Left, pan1Right;
    float pan2, pan2Left, pan2Right;

    float outVol, outVolFader;
        
    char programName[32];
};

#endif
