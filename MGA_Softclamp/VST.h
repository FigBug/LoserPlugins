#ifndef __VST__
#define __VST__

#include "audioeffectx.h"

enum {
	// Global
	kNumPrograms = 0,

	// Parameters Tags
	kGain = 0,
	kKnee,

	kNumParams,

    // VU Tags	
	kGRL = 0,
    kGRPeakL,
    kGRR,
    kGRPeakR,
    kKMeterL,
    kKMeterR,
	kNumVUs
};  

//-------------------------------------------------------------------------------------------------------
class VST : public AudioEffectX
{
public:
	VST (audioMasterCallback audioMaster);
	~VST ();
	
	// Resume
	virtual void resume ();
    virtual void suspend();
    
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
	virtual void getParameterLabelDisplay (VstInt32 index, float value, char* string);
    	
    virtual float getVU(VstInt32 index);
    virtual void resetLastIdleCall();

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
    virtual VstInt32 getVendorVersion ();

	virtual VstPlugCategory getPlugCategory () { return kPlugCategEffect; }
	
protected:


    double P0, P1, P2;

    double gain;

    float GRMeterL, GRPeakL;
    float GRMeterR, GRPeakR;
    float GRMeterFallOff;

    float KMeterSumL, KMeterL;
    float KMeterSumR, KMeterR;
    float KMeterCoef;

    unsigned long samplesSinceLastIdleCall;
    unsigned long samplesSinceLastPeakResetL;
    unsigned long samplesSinceLastPeakResetR;
    
    char programName[kVstMaxProgNameLen + 1];
};

#endif

