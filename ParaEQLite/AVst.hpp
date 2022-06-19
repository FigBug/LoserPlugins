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

    // Set coeffs
    void setHS();
    void setLS();
    void setP1();
    void setP2();
    void setP3();

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
  
	float freqHP, freqHPFader;
    
    float w0HP;
    float cosw0HP, sinw0HP, alphaHP;
    float b0HP, b1HP, b2HP, a0HP, a1HP, a2HP;
    
    float tmp;

    float xl1HP, xl2HP, yl1HP, yl2HP;
    float xr1HP, xr2HP, yr1HP, yr2HP;

    float freqLS, freqLSFader;
    float dBgainLS, dBgainLSFader, ALS, w0LS, SLS;
//    float cosw0~, sinw0~
    float alphaLS;
    float b0LS, b1LS, b2LS, a0LS, a1LS, a2LS;

    float xl1LS, xl2LS, yl1LS, yl2LS;
    float xr1LS, xr2LS, yr1LS, yr2LS;


    float freqHS, freqHSFader;
    float dBgainHS, dBgainHSFader, AHS, w0HS, SHS;
//    float cosw0~, sinw0~
    float alphaHS;
    float b0HS, b1HS, b2HS, a0HS, a1HS, a2HS;

    float xl1HS, xl2HS, yl1HS, yl2HS;
    float xr1HS, xr2HS, yr1HS, yr2HS;


    float freqP1, freqP1Fader;
    float dBgainP1, dBgainP1Fader, AP1, w0P1, BWP1;
//    float cosw0~, sinw0~
    float alphaP1;
    float b0P1, b1P1, b2P1, a0P1, a1P1, a2P1;

    float xl1P1, xl2P1, yl1P1, yl2P1;
    float xr1P1, xr2P1, yr1P1, yr2P1;


    float freqP2, freqP2Fader;
    float dBgainP2, dBgainP2Fader, AP2, w0P2, BWP2;
//    float cosw0~, sinw0~
    float alphaP2;
    float b0P2, b1P2, b2P2, a0P2, a1P2, a2P2;

    float xl1P2, xl2P2, yl1P2, yl2P2;
    float xr1P2, xr2P2, yr1P2, yr2P2;


    float freqP3, freqP3Fader;
    float dBgainP3, dBgainP3Fader, AP3, w0P3, BWP3;
//    float cosw0~, sinw0~
    float alphaP3;
    float b0P3, b1P3, b2P3, a0P3, a1P3, a2P3;

    float xl1P3, xl2P3, yl1P3, yl2P3;
    float xr1P3, xr2P3, yr1P3, yr2P3;


    float outVolume, outVolumeFader;

    char programName[32];
};

#endif
