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
	
	void setP1();

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
	float thresh, threshDB, threshFader;
    float threshLow, threshHigh;
// HOPEFULLY CAN DITCH THOSE AT SOME POINT
    float threshLowDB, threshHighDB;
    
    float ratio, ratioFader;

    float kneeDB, kneeSlopeDB, kneeFader;

    float attackStart, attackStartSpls;
    unsigned long lookAheadMaxSize, lookAheadSize, lookAheadBufPos;
	float *lookAheadBufferRms, *lookAheadBufferIn1, *lookAheadBufferIn2;

    signed long lookAheadMaxTime;
    float lookAheadRms;

    void FlushLookAheadBuffers ();
    void ReallocLookAheadBuffers ();

    float attack, attackSpls, attackFader;

    float release, releaseSpls, releaseFader;

    float attackShape, releaseShape;

	float rmsFader;
    unsigned long rmsMaxSize, rmsSize, rmsBufPos;
	float *rmsBuffer;
    void FlushRmsBuffer ();
    void ReallocRmsBuffer ();


	float freqLP, freqLPFader;
	float freqHP, freqHPFader;
    
    float w0HP;
    float cosw0HP, sinw0HP, alphaHP;
    float b0HP, b1HP, b2HP, a0HP, a1HP, a2HP;
    
    float w0LP;
    float cosw0LP, sinw0LP, alphaLP;
    float b0LP, b1LP, b2LP, a0LP, a1LP, a2LP;

    float xl1LP, xl2LP, yl1LP, yl2LP;
    float xr1LP, xr2LP, yr1LP, yr2LP;
    float xl1HP, xl2HP, yl1HP, yl2HP;
    float xr1HP, xr2HP, yr1HP, yr2HP;

    float freqP1, freqP1Fader;
    float dBgainP1, dBgainP1Fader, AP1, w0P1, BWP1;
//    float cosw0~, sinw0~
    float alphaP1;
    float b0P1, b1P1, b2P1, a0P1, a1P1, a2P1;

    float xl1P1, xl2P1, yl1P1, yl2P1;
    float xr1P1, xr2P1, yr1P1, yr2P1;
    
    float tmp;
    
    float listen;
    
    float feed;
    
    float modeMakeUp;

    float outVolume, outVolumeFader;
    
    float dryMix, dryMixFader;
    
    float saturation, satC;

// Process:
    
    float freqEnvDetLPFader;   
    float envDet, a0EnvLP, b1EnvLP, tmpEnvLP, seekMaxSpls;
        
    float detL, detR;
	float rms, rmsDB, kneeLineDB, maxSpls, rmsSqrSum;
	float gain, seekGain, underThresh, overThresh, spl0Out, spl1Out, volume;

    unsigned long t;

	char programName[32];
};

#endif
