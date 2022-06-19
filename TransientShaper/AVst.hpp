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

        
// Process:
     
    float a0EnvLP, b1EnvLP, tmpEnvLP, env;
    float a0EnvAttLP, b1EnvAttLP, tmpEnvAttLP, envAtt;
    float a0EnvRelLP, b1EnvRelLP, tmpEnvRelLP, envRel;
    float a0GainLP, b1GainLP, tmpGainLP;
          
	float maxSpls;
	float smooth;
	float gain, gainAtt, gainRel;
    float outVolume, outVolumeFader;
	
	float attackFader, attack;
	float sustainFader, sustain;

	char programName[32];
};

#endif
