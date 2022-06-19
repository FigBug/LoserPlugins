/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "VST.h"

#include <cmath>
#include <algorithm>
#include <cstdlib>


#define cAmpDB_     8.656170245 // 6/log(2);
#define cDBAmp_     0.115524530 // log(2)/6;
#define cPi_        3.141592654
#define cSqrt2_     1.414213562 // sqrt(2);
#define cSqrt2h_    0.707106781 // sqrt(0.5f);
#define cDC_     1e-30

inline float sqr (float x)
{
    return ( x*x );
}

inline float abs (float x)
{
    return ( x<0 ? -x:x);
}

inline float sign (float x)
{
    return (x<0 ? -1:1);   
}

inline float min (float x, float y)
{
    return ( x<y ? x:y);
}

inline double min (float x, double y)
{
    return ( x<y ? x:y);
}

inline float max (float x, float y)
{
    return ( x>y ? x:y);
}

inline float amp2DB (float amp)
{
    return ( log(amp) * cAmpDB_ );
}

inline float dB2Amp (float dB)
{
    return ( exp(dB * cDBAmp_ ) );
}

inline float absmax (float x, float y)
{
    return max(abs(x),abs(y));
}

inline float limit (float in, float limit)
{
    return min(max(in,-limit),limit);   
}


inline double sqr (double x)
{
    return ( x*x );
}

inline double abs (double x)
{
    return ( x<0 ? -x:x);
}

inline double sign (double x)
{
    return (x<0 ? -1:1);   
}

inline double min (double x, double y)
{
    return ( x<y ? x:y);
}

inline double max (double x, double y)
{
    return ( x>y ? x:y);
}

inline double amp2DB (double amp)
{
    return ( log(amp) * cAmpDB_ );
}

inline double dB2Amp (double dB)
{
    return ( exp(dB * cDBAmp_ ) );
}

inline double absmax (double x, double y)
{
    return max(abs(x),abs(y));
}

inline double limit (double in, double limit)
{
    return min(max(in,-limit),limit);   
}




//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new VST (audioMaster);
}

//-------------------------------------------------------------------------------------------------------
VST::VST (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, 8)	// 1 program, 1 parameter only
{
	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('T','R','N','S') );	// identify
	canProcessReplacing ();	// supports replacing output
	canDoubleReplacing ();	// supports double precision processing

	attackFader = sustainFader = 0.5;
    attack = sustain = 0;

    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() );
    a0EnvLP = 1.0 + b1EnvLP;
    
    b1EnvAttLP = -exp(-2.0*cPi_*100 / getSampleRate() );
    a0EnvAttLP = 1.0 + b1EnvAttLP;
    
    b1EnvRelLP = -exp(-2.0*cPi_*1 / getSampleRate() );
    a0EnvRelLP = 1.0 + b1EnvRelLP;
  
    freqLPFader = 1;
    freqLP = 20000;
    w0LP = 2 * cPi_ * freqLP/getSampleRate();
    cosw0LP = cos(w0LP);
    sinw0LP = sin(w0LP);
    alphaLP = sinw0LP / cSqrt2_;
    b0LP = (1 - cosw0LP)/2;
    b1LP = 1 - cosw0LP;
    b2LP = (1 - cosw0LP)/2;
    a0LP = 1 + alphaLP;
    a1LP = -2 * cosw0LP;
    a2LP = 1 - alphaLP;
    b0LP /= a0LP;
    b1LP /= a0LP;
    b2LP /= a0LP;
    a1LP /= a0LP;
    a2LP /= a0LP;

    freqHPFader = 0;
    freqHP = 20;
    w0HP = 2 * cPi_ * freqHP/getSampleRate();
    cosw0HP = cos(w0HP);
    sinw0HP = sin(w0HP);
    alphaHP = sinw0HP / cSqrt2_;
    b0HP = (1 + cosw0HP)/2;
    b1HP = -(1 + cosw0HP);
    b2HP = (1 + cosw0HP)/2;
    a0HP = 1 + alphaHP;
    a1HP = -2 * cosw0HP;
    a2HP = 1 - alphaHP;
    b0HP /= a0HP;
    b1HP /= a0HP;
    b2HP /= a0HP;
    a1HP /= a0HP;
    a2HP /= a0HP;                           

    detMode = 0.5;
    detSide = 0.5;
    detMid = 0.5;
    
    mixMode = 0.5;
    procSide = 0.5;
    procMid = 0.5;


    smooth = 0.5;
    b1GainLP = -exp(-2.0*cPi_*9.999975 / getSampleRate() ); // 89.44....Hz
    a0GainLP = 1.0 + b1GainLP;
    
    outFader = 0.5;
    out = 1; // = 0 dB
   
	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name
}

//-------------------------------------------------------------------------------------------------------
VST::~VST ()
{
	// nothing to do here
}

//-------------------------------------------------------------------------------------------------------
void VST::resume()
{

    xl1LP = xl2LP = yl1LP = yl2LP =
    xr1LP = xr2LP = yr1LP = yr2LP = 0;
           
    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() );
    a0EnvLP = 1.0 + b1EnvLP;
    tmpEnvLP = 0;

    b1EnvAttLP = -exp(-2.0*cPi_*100 / getSampleRate() );
    a0EnvAttLP = 1.0 + b1EnvAttLP;
    tmpEnvAttLP = 0;

    b1EnvRelLP = -exp(-2.0*cPi_*1 / getSampleRate() );
    a0EnvRelLP = 1.0 + b1EnvRelLP;
    tmpEnvRelLP = 0;

    tmpGainLP = 0;
    
}

//-------------------------------------------------------------------------------------------------------
void VST::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void VST::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void VST::setParameter (VstInt32 index, float value)
{
    switch (index)
	{
		case 0 : 
            attackFader = value;
            attack = (-4 + attackFader * 8);
        break;

		case 1 :
            sustainFader = value;
            sustain = (-8 + sustainFader * 16);
        break;
        
        case 2 :
            freqLPFader = value;
            freqLP = min( (double) (20+pow(freqLPFader,4)*19980),(double) (getSampleRate()/2.1));
            w0LP = 2 * cPi_ * freqLP/getSampleRate();
            cosw0LP = cos(w0LP);
            sinw0LP = sin(w0LP);
            alphaLP = sinw0LP / cSqrt2_;
            b0LP = (1 - cosw0LP)/2;
            b1LP = 1 - cosw0LP;
            b2LP = (1 - cosw0LP)/2;
            a0LP = 1 + alphaLP;
            a1LP = -2 * cosw0LP;
            a2LP = 1 - alphaLP;
            b0LP /= a0LP;
            b1LP /= a0LP;
            b2LP /= a0LP;
            a1LP /= a0LP;
            a2LP /= a0LP;
        break;
 
        case 3 :
            freqHPFader = value;
            freqHP = min( (double) (20+pow(freqHPFader,4)*19980),(double) (getSampleRate()/2.1));
            w0HP = 2 * cPi_ * freqHP/getSampleRate();
            cosw0HP = cos(w0HP);
            sinw0HP = sin(w0HP);
            alphaHP = sinw0HP / cSqrt2_;
            b0HP = (1 + cosw0HP)/2;
            b1HP = -(1 + cosw0HP);
            b2HP = (1 + cosw0HP)/2;
            a0HP = 1 + alphaHP;
            a1HP = -2 * cosw0HP;
            a2HP = 1 - alphaHP;
            b0HP /= a0HP;
            b1HP /= a0HP;
            b2HP /= a0HP;
            a1HP /= a0HP;
            a2HP /= a0HP;
        break;

        case 4 :
            detMode = value;
            detSide = min(detMode,0.5);
            detMid = min(1-detMode,0.5);
            tmpEnvRelLP = 0.00001;
        break;
                        
        case 5 :
            mixMode = value;
            procSide = min(mixMode,0.5);
            procMid = min(1-mixMode,0.5);
        break;

        case 6 :
            smooth = value;
            b1GainLP = -exp(-2.0*cPi_* (  pow(9999.9,sqr(1-smooth))+0.1   ) / getSampleRate() ); 
            a0GainLP = 1.0 + b1GainLP;   
        break;
               
        case 7 :
            outFader = value;
            out = dB2Amp( sign(outFader -.5)* sqr((outFader -.5)*2)*12 );
        break;

	}
}

//-----------------------------------------------------------------------------------------
float VST::getParameter (VstInt32 index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = attackFader;     break;
		case 1 : v = sustainFader;    break;
		case 2 : v = freqLPFader;   break;
		case 3 : v = freqHPFader;   break;
		case 4 : v = detMode;    break;
		case 5 : v = mixMode;    break;
		case 6 : v = smooth;          break;
		case 7 : v = outFader;  break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void VST::getParameterName (VstInt32 index, char* label)
{
    switch (index)
	{
		case 0 : vst_strncpy (label, "Attack"    , kVstMaxParamStrLen);     break;
		case 1 : vst_strncpy (label, "Sustain"   , kVstMaxParamStrLen);     break;
		case 2 : vst_strncpy (label, "Lowpass"   , kVstMaxParamStrLen);     break;
		case 3 : vst_strncpy (label, "Highpass"   , kVstMaxParamStrLen);     break;
		case 4 : vst_strncpy (label, "Detector"    , kVstMaxParamStrLen);     break;
		case 5 : vst_strncpy (label, "Process"    , kVstMaxParamStrLen);     break;
		case 6 : vst_strncpy (label, "Smooth"    , kVstMaxParamStrLen);     break;
		case 7 : vst_strncpy (label, "Output"    , kVstMaxParamStrLen);     break;
	}
}

//-----------------------------------------------------------------------------------------
void VST::getParameterDisplay (VstInt32 index, char* text)
{
    switch (index)
	{
		case 0 : float2string (-100 + attackFader * 200, text, kVstMaxParamStrLen);    break;
		case 1 : float2string (-100 + sustainFader * 200, text, kVstMaxParamStrLen);    break;
        case 2 :
            if (freqLPFader == 1) vst_strncpy (text, "OFF", kVstMaxParamStrLen);
            else float2string (freqLP, text, kVstMaxParamStrLen);
        break;
        case 3 :
            if (freqHPFader == 0) vst_strncpy (text, "OFF", kVstMaxParamStrLen);
            else float2string (freqHP, text, kVstMaxParamStrLen);
        break;
		case 4 :
            float2string (abs(detMode-0.5)*200, text, kVstMaxParamStrLen);
        break;
		case 5 :
            float2string (abs(mixMode-0.5)*200, text, kVstMaxParamStrLen);
        break;
		case 6 : float2string (smooth*100, text, kVstMaxParamStrLen);    break;
		case 7 : dB2string    (out , text, kVstMaxParamStrLen);    break;
	}
}

//-----------------------------------------------------------------------------------------
void VST::getParameterLabel (VstInt32 index, char* label)
{
    switch (index)
	{
		case 0 : vst_strncpy (label, "%" , kVstMaxParamStrLen);	    break;
		case 1 : vst_strncpy (label, "%" , kVstMaxParamStrLen);	    break;
		case 2 : vst_strncpy (label, "Hz" , kVstMaxParamStrLen);	    break;
		case 3 : vst_strncpy (label, "Hz" , kVstMaxParamStrLen);	    break;
		case 4 : vst_strncpy (label, "M/S" , kVstMaxParamStrLen);	    break;
		case 5 : vst_strncpy (label, "M/S" , kVstMaxParamStrLen);	    break;
		case 6 : vst_strncpy (label, "%" , kVstMaxParamStrLen);	    break;
		case 7 : vst_strncpy (label, "dB", kVstMaxParamStrLen);	    break;
	}
}

//------------------------------------------------------------------------
bool VST::getEffectName (char* name)
{
	vst_strncpy (name, "TranShaper", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool VST::getProductString (char* text)
{
	vst_strncpy (text, "TranShaper", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool VST::getVendorString (char* text)
{
	vst_strncpy (text, "", kVstMaxVendorStrLen);
	return true;
}

//-----------------------------------------------------------------------------------------
VstInt32 VST::getVendorVersion ()
{ 
	return 1000; 
}

//-----------------------------------------------------------------------------------------
void VST::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
    float* in1  =  inputs[0];
    float* in2  =  inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];

    while (--sampleFrames >= 0)
    {
        dryL = 0;
        dryR = 0;
        if (freqLPFader != 1)
        {
            tmp = (*in1);
            (*in1) = b0LP * (*in1) + b1LP * xl1LP + b2LP * xl2LP - a1LP * yl1LP - a2LP * yl2LP;
            xl2LP = xl1LP; xl1LP = tmp; yl2LP = yl1LP; yl1LP = (*in1) + cDC_;
            dryL = tmp - (*in1);

            tmp = (*in2);
            (*in2) = b0LP * (*in2) + b1LP * xr1LP + b2LP * xr2LP - a1LP * yr1LP - a2LP * yr2LP;
            xr2LP = xr1LP; xr1LP = tmp; yr2LP = yr1LP; yr1LP = (*in2) + cDC_;
            dryR = tmp - (*in2);
        }
        if (freqHPFader)
        {
            tmp = (*in1);
            (*in1) = b0HP * (*in1) + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
            xl2HP = xl1HP; xl1HP = tmp; yl2HP = yl1HP; yl1HP = (*in1) + cDC_;
            if (freqLPFader != 1) dryL += tmp-(*in1);
            else dryL = tmp - (*in1);

            tmp = (*in2);
            (*in2) = b0HP * (*in2) + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
            xr2HP = xr1HP; xr1HP = tmp; yr2HP = yr1HP; yr1HP = (*in2) + cDC_;
            if (freqLPFader != 1) dryR += tmp-(*in2);
            else dryR = tmp - (*in2);
        }
        
        mid  = ( (*in1  ) + (*in2  ) )/2;
        side = ( (*in1++) - (*in2++) )/2;

//        if (detMode==0.5) maxSpls = absmax( (*in1) , (*in2) ) + 0.00001;
//        else
//        {
            maxSpls = absmax( mid*detMid , side*detSide ) + 0.00001;
//        }
   
        env = sqrt( (tmpEnvLP = a0EnvLP*maxSpls - b1EnvLP*tmpEnvLP + cDC_)  );
        envAtt = sqrt( (tmpEnvAttLP = a0EnvAttLP*maxSpls - b1EnvAttLP*tmpEnvAttLP + cDC_)  );
        envRel = sqrt( (tmpEnvRelLP = a0EnvRelLP*maxSpls - b1EnvRelLP*tmpEnvRelLP + cDC_)  );

        gain = exp( log( max(envAtt/env,1) ) *attack + log( max(envRel/env,1) ) *sustain );
        gain = min( sqrt( (tmpGainLP = a0GainLP*gain - b1GainLP*tmpGainLP + cDC_) ) , gain);
        gain = min(gain,64);
        
        mid  *= pow(gain,procMid );
        side *= pow(gain,procSide);

        (*out1++) = (dryL + (mid + side))*out;
        (*out2++) = (dryR + (mid - side))*out;

    }
}

//-----------------------------------------------------------------------------------------
void VST::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
    double* in1  =  inputs[0];
    double* in2  =  inputs[1];
    double* out1 = outputs[0];
    double* out2 = outputs[1];

    while (--sampleFrames >= 0)
    {
        dryL = 0;
        dryR = 0;
        if (freqLPFader != 1)
        {
            tmp = (*in1);
            (*in1) = b0LP * (*in1) + b1LP * xl1LP + b2LP * xl2LP - a1LP * yl1LP - a2LP * yl2LP;
            xl2LP = xl1LP; xl1LP = tmp; yl2LP = yl1LP; yl1LP = (*in1) + cDC_;
            dryL = tmp - (*in1);

            tmp = (*in2);
            (*in2) = b0LP * (*in2) + b1LP * xr1LP + b2LP * xr2LP - a1LP * yr1LP - a2LP * yr2LP;
            xr2LP = xr1LP; xr1LP = tmp; yr2LP = yr1LP; yr1LP = (*in2) + cDC_;
            dryR = tmp - (*in2);
        }
        if (freqHPFader)
        {
            tmp = (*in1);
            (*in1) = b0HP * (*in1) + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
            xl2HP = xl1HP; xl1HP = tmp; yl2HP = yl1HP; yl1HP = (*in1) + cDC_;
            if (freqLPFader != 1) dryL += tmp-(*in1);
            else dryL = tmp - (*in1);

            tmp = (*in2);
            (*in2) = b0HP * (*in2) + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
            xr2HP = xr1HP; xr1HP = tmp; yr2HP = yr1HP; yr1HP = (*in2) + cDC_;
            if (freqLPFader != 1) dryR += tmp-(*in2);
            else dryR = tmp - (*in2);
        }
        
        mid  = ( (*in1  ) + (*in2  ) )/2;
        side = ( (*in1++) - (*in2++) )/2;

//        if (detMode==0.5) maxSpls = absmax( (*in1) , (*in2) ) + 0.00001;
//        else
//        {
            maxSpls = absmax( mid*detMid , side*detSide ) + 0.00001;
//        }
   
        env = sqrt( (tmpEnvLP = a0EnvLP*maxSpls - b1EnvLP*tmpEnvLP + cDC_)  );
        envAtt = sqrt( (tmpEnvAttLP = a0EnvAttLP*maxSpls - b1EnvAttLP*tmpEnvAttLP + cDC_)  );
        envRel = sqrt( (tmpEnvRelLP = a0EnvRelLP*maxSpls - b1EnvRelLP*tmpEnvRelLP + cDC_)  );

        gain = exp( log( max(envAtt/env,1) ) *attack + log( max(envRel/env,1) ) *sustain );
        gain = min( sqrt( (tmpGainLP = a0GainLP*gain - b1GainLP*tmpGainLP + cDC_) ) , gain);
        gain = min(gain,64);
        
        mid  *= pow(gain,procMid );
        side *= pow(gain,procSide);

        (*out1++) = (dryL + (mid + side))*out;
        (*out2++) = (dryR + (mid - side))*out;

    }
}
