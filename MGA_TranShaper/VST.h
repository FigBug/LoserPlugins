/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */ 

#ifndef __VST__
#define __VST__

#include "public.sdk/source/vst2.x/audioeffectx.h"

//-------------------------------------------------------------------------------------------------------
class VST : public AudioEffectX
{
public:
	VST (audioMasterCallback audioMaster);
	~VST ();
	
	// Suspend
//	virtual void suspend ();

    virtual void resume ();

	// Processing
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);

	// Program
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);

	// Parameters
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();

protected:
    double a0EnvLP, b1EnvLP, tmpEnvLP, env;
    double a0EnvAttLP, b1EnvAttLP, tmpEnvAttLP, envAtt;
    double a0EnvRelLP, b1EnvRelLP, tmpEnvRelLP, envRel;
    double a0GainLP, b1GainLP, tmpGainLP;
    
    double mid, side;
    float detMode;
    float mixMode;

   	double freqLP;
    float freqLPFader;
   	double tmp;
   	double w0LP;
    double cosw0LP, sinw0LP, alphaLP;
    double b0LP, b1LP, b2LP, a0LP, a1LP, a2LP;
    double xl1LP, xl2LP, yl1LP, yl2LP;
    double xr1LP, xr2LP, yr1LP, yr2LP;
 
   	double freqHP;
    float freqHPFader;
   	double w0HP;
    double cosw0HP, sinw0HP, alphaHP;
    double b0HP, b1HP, b2HP, a0HP, a1HP, a2HP;
    double xl1HP, xl2HP, yl1HP, yl2HP;
    double xr1HP, xr2HP, yr1HP, yr2HP;
                          
    double maxSpls;
	double smooth;
	double gain, gainAtt, gainRel;
    double out;
    float outFader;
	
	float attackFader, sustainFader;
    double attack, sustain;

    double detMid, detSide;
    double procMid, procSide;
	double dryL, dryR;
   	
	char programName[kVstMaxProgNameLen + 1];
};

#endif
