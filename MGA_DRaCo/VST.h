#ifndef __VST__
#define __VST__

#include "audioeffectx.h"

enum {
	// Global
	kNumPrograms = 0,

	// Parameters Tags
	kThresh = 0,
	kRatio,
	kAttack,
	kRelease,
	kOutput,

	kNumParams,

    // VU Tags	
	kGR = 0,
    kGRPeak,	
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

    double thresh;
    double att;
    double rel;
    double ratio;
    double out;

    double foo;
    double gain, env_i, env, env_rel;

    float thresh_f, ratio_f, att_f, rel_f, out_f;

    float GRMeter, GRPeak, meterFallOff;

    unsigned long samplesSinceLastIdleCall;
    unsigned long samplesSinceLastPeakReset;
    
    char programName[kVstMaxProgNameLen + 1];
};

#endif

