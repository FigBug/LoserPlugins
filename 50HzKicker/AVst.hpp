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
/*	virtual bool getInputProperties (long index, VstPinProperties* properties)
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
	*/
protected:

        
// Process:

	float freqDBPFader;
    
    float w0DBP;
    float cosw0DBP, sinw0DBP, alphaDBP;
    float b0DBP, b1DBP, b2DBP, a0DBP, a1DBP, a2DBP;  
    float x1DBP, x2DBP, y1DBP, y2DBP;

    float w0OBP;
    float cosw0OBP, sinw0OBP, alphaOBP;
    float b0OBP, b1OBP, b2OBP, a0OBP, a1OBP, a2OBP;    
    float x1OBP, x2OBP, y1OBP, y2OBP;

    float tmp;

    
     
    float a0EnvAttLP, b1EnvAttLP, tmpEnvAttLP, envAtt;
    float env, attEnv, relEnv;
          
	float gain, det;
	float lengthFader;
    float dry, dryFader;
    float wet, wetFader;
	
    float pos, adj;
    float freqFader;
    
	char programName[32];
};

#endif
