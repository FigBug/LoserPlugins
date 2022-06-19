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

    // Suspend
	virtual void suspend ();

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
  

	float freqLP, freqLPFader;
	float freqHP, freqHPFader;
	float slopeLP, slopeHP;

    float xLP6, a0LP6, b1LP6;
    float xHP6, a0HP6, b1HP6;
    
    float tmp1LP6, tmp2LP6, tmp1HP6, tmp2HP6;
    
    float w0HP;
    float cosw0HP, sinw0HP, alphaHP;
    float b0HP, b1HP, b2HP, a0HP, a1HP, a2HP;
    
    float w0LP;
    float cosw0LP, sinw0LP, alphaLP;
    float b0LP, b1LP, b2LP, a0LP, a1LP, a2LP;

    float tmp;

    float xl1LP, xl2LP, yl1LP, yl2LP;
    float xr1LP, xr2LP, yr1LP, yr2LP;
    float xl1HP, xl2HP, yl1HP, yl2HP;
    float xr1HP, xr2HP, yr1HP, yr2HP;
    
    char programName[32];
};

#endif
