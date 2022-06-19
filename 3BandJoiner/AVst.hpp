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
	// --- cern.th.skei
	virtual bool getInputProperties (long index, VstPinProperties* properties)
	{
		if ((index & 1) == 0) { properties->flags |= kVstPinIsStereo; }
		return true;
	};
	virtual bool getOutputProperties (long index, VstPinProperties* properties)
	{
		if ((index & 1) == 0) { properties->flags |= kVstPinIsStereo; }
		return true;
	};
	// ---
	
protected:
  

    float ch12Vol, ch34Vol, ch56Vol, outVol;
    float ch12VolF, ch34VolF, ch56VolF, outVolF;

    
    char programName[32];
};

#endif
