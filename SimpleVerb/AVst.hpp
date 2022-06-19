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

protected:
   
    unsigned long preDelayPos, preDelayLength, preDelayMaxLength;
	float *preDelay;
	float preDelayFader;

    unsigned long comb1Pos, comb1Length, comb1MaxLength;
	float *comb1;
    unsigned long comb2Pos, comb2Length, comb2MaxLength;
	float *comb2;
    unsigned long comb3Pos, comb3Length, comb3MaxLength;
	float *comb3;
    unsigned long comb4Pos, comb4Length, comb4MaxLength;
	float *comb4;
    unsigned long comb5Pos, comb5Length, comb5MaxLength;
	float *comb5;
    unsigned long comb6Pos, comb6Length, comb6MaxLength;
	float *comb6;
    unsigned long comb7Pos, comb7Length, comb7MaxLength;
	float *comb7;
    unsigned long comb8Pos, comb8Length, comb8MaxLength;
	float *comb8;
    unsigned long comb9Pos, comb9Length, comb9MaxLength;
	float *comb9;
    unsigned long comb10Pos, comb10Length, comb10MaxLength;
	float *comb10;
    unsigned long comb11Pos, comb11Length, comb11MaxLength;
	float *comb11;
    unsigned long comb12Pos, comb12Length, comb12MaxLength;
	float *comb12;

    unsigned long allpassL1Pos, allpassL1Length;
	float *allpassL1;
    unsigned long allpassL2Pos, allpassL2Length;
	float *allpassL2;
    unsigned long allpassL3Pos, allpassL3Length;
	float *allpassL3;

    unsigned long allpassR1Pos, allpassR1Length;
	float *allpassR1;
    unsigned long allpassR2Pos, allpassR2Length;
	float *allpassR2;
    unsigned long allpassR3Pos, allpassR3Length;
	float *allpassR3;

    void FlushBuffers ();
    void FlushPreDelay ();
    void ReallocPreDelay ();
    void ReallocComb1 ();
    void ReallocComb2 ();
    void ReallocComb3 ();
    void ReallocComb4 ();
    void ReallocComb5 ();
    void ReallocComb6 ();
    void ReallocComb7 ();
    void ReallocComb8 ();
    void ReallocComb9 ();
    void ReallocComb10 ();
    void ReallocComb11 ();
    void ReallocComb12 ();
    void ReallocAllpassL1 ();
    void ReallocAllpassL2 ();
    void ReallocAllpassL3 ();
    void ReallocAllpassR1 ();
    void ReallocAllpassR2 ();
    void ReallocAllpassR3 ();


    float reverb, damp, dry, wet, left, right;
    float roomSize, roomSizeFader;
    float dampFader;
    
    float freqLP, freqLPFader;
	float freqHP, freqHPFader;
    
    float a0LP, b1LP, tmp1LP, tmp2LP;
    float a0HP, b1HP, tmp1HP, tmp2HP;

	char programName[32];
};

#endif
