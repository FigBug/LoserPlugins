//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// � 2003, Steinberg Media Technologies, All Rights Reserved
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

protected:
    void FlushBuffer ();
    void ReallocBuffer ();

    float delayFader;
    unsigned long delayMaxSize, delaySize, delayBufPos;
	float *delayBuffer;

    float mixIn;
    
    float feedback;
    
    float freqLP, freqLPFader;
	float freqHP, freqHPFader;
    
    float xLP, a0LP, b1LP;
    float xHP, a0HP, b1HP;
    
    float trim;
    
    float del1, del2;
    
    float out1LP, out2LP, out1HP, out2HP;
    float tmp1LP, tmp2LP, tmp1HP, tmp2HP;
    
    float dry;
    float wet;

    float outVolume, outVolumeFader;

	char programName[32];
};

#endif
