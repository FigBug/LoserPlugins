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
	virtual void resume ();

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

    float attackC2;
    float releaseC2;

	float freqLP;
	float freqHP;
    
    float w0HP;
    float cosw0HP, sinw0HP, alphaHP;
    float b0HP, b1HP, b2HP, a0HP, a1HP, a2HP;
    
    float w0LP;
    float cosw0LP, sinw0LP, alphaLP;
    float b0LP, b1LP, b2LP, a0LP, a1LP, a2LP;

    float tmpL, xl1LP, xl2LP, yl1LP, yl2LP;
    float tmpR, xr1LP, xr2LP, yr1LP, yr2LP;
    float xl1HP, xl2HP, yl1HP, yl2HP;
    float xr1HP, xr2HP, yr1HP, yr2HP;

      
// Process:
    
    float freqEnvDetLPFader;
    float envDet, a0EnvLP, b1EnvLP, tmpEnvLP, seekMaxSpls;
      
    float DEEnvDet, a0DEEnvLP, b1DEEnvLP, tmpDEEnvLP;
    
    float deEsser;

	float freqLPDE;
	float freqHPDE;
    
    float w0HPDE;
    float cosw0HPDE, sinw0HPDE, alphaHPDE;
    float b0HPDE, b1HPDE, b2HPDE, a0HPDE, a1HPDE, a2HPDE;
    
    float xl1LPDE, xl2LPDE, yl1LPDE, yl2LPDE;
    float xr1LPDE, xr2LPDE, yr1LPDE, yr2LPDE;
    float xl1HPDE, xl2HPDE, yl1HPDE, yl2HPDE;
    float xr1HPDE, xr2HPDE, yr1HPDE, yr2HPDE;
    
    float gTmpoR, gTmpoL;

    float filters;

    float threshC1, threshC1Fader, threshC2, threshC2Fader;
    float threshDE, threshDEFader, gainDE;    
    float maxSpls, gainC1, seekGainC2, gainC2;
    float outVolume, outVolumeFader;
    
    float GRC1, GRC2, GRDE;

	char programName[32];
};

#endif
