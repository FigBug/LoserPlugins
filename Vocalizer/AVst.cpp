//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>
#include <stdlib.h>


// Mich's Formulas V 0.1

#define cAmpDB_     8.656170245 // 6/log(2);
#define cDBAmp_     0.115524530 // log(2)/6;
#define cPi_        3.141592654
#define cSqrt2_     1.414213562 // sqrt(2);
#define cSqrt2h_    0.707106781 // sqrt(0.5f);
#define cDcAdd_     1e-30
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
    return (x>0 ? 1:-1);   
}

inline float min (float x, float y)
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

// eof Mich's formulas


#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 8)	// 1 program, 8 parameters
{

    threshC1 = threshC1Fader = 1;
    threshC2 = threshC2Fader = 1;
    
    attackC2 = exp( -60 / (0.008 * getSampleRate()) / cAmpDB_ );
    releaseC2 = exp( -60 / (0.1 * getSampleRate()) / cAmpDB_ );

    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() ); // 10Hz
    a0EnvLP = 1.0 + b1EnvLP;

    b1DEEnvLP = -exp(-2.0*cPi_*50 / getSampleRate() ); // 10Hz
    a0DEEnvLP = 1.0 + b1DEEnvLP;

    filters = 0;
    
        freqLP = min(12000,(getSampleRate()/2)-10);
        w0LP = 2 * cPi_ * freqLP/getSampleRate();
        cosw0LP = cos(w0LP);
        sinw0LP = sin(w0LP);
        alphaLP = sinw0LP / cSqrt2_;
        b0LP = (1 - cosw0LP)/2;
        b1LP = (1 - cosw0LP);
        b2LP = (1 - cosw0LP)/2;
        a0LP = 1 + alphaLP;
        a1LP = -2 * cosw0LP;
        a2LP = 1 - alphaLP;
        b0LP /= a0LP;
        b1LP /= a0LP;
        b2LP /= a0LP;
        a1LP /= a0LP;
        a2LP /= a0LP;

        freqHP = min(60,(getSampleRate()/2)-10);
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


    threshDE = threshDEFader = 1;
 
        freqHPDE = min(3000,(getSampleRate()/2)-10);
        w0HPDE = 2 * cPi_ * freqHPDE/getSampleRate();
        cosw0HPDE = cos(w0HPDE);
        sinw0HPDE = sin(w0HPDE);
 //       alphaHPDE = sinw0HPDE / cSqrt2_;
        alphaHPDE = sinw0HPDE /2 * sqrt( (1/2 - 1) + 2 );
        b0HPDE = (1 + cosw0HPDE)/2;
        b1HPDE = -(1 + cosw0HPDE);
        b2HPDE = (1 + cosw0HPDE)/2;
        a0HPDE = 1 + alphaHPDE;
        a1HPDE = -2 * cosw0HPDE;
        a2HPDE = 1 - alphaHPDE;
        b0HPDE /= a0HPDE;
        b1HPDE /= a0HPDE;
        b2HPDE /= a0HPDE;
        a1HPDE /= a0HPDE;
        a2HPDE /= a0HPDE; 
     
  
    outVolumeFader = 0.5; // ~ 0 dB
    outVolume = 1; // = 0 dB


	setNumInputs (2);		// stereo in + stereo sidechain (aux)
	setNumOutputs (2);
	setUniqueID (CCONST ('V','O','C','L') );
//    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

	suspend(); // flush buffers and reset variables
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

}

void AVst::resume()
{
        freqLP = min(12000,(getSampleRate()/2)-10);
        w0LP = 2 * cPi_ * freqLP/getSampleRate();
        cosw0LP = cos(w0LP);
        sinw0LP = sin(w0LP);
        alphaLP = sinw0LP / cSqrt2_;
        b0LP = (1 - cosw0LP)/2;
        b1LP = (1 - cosw0LP);
        b2LP = (1 - cosw0LP)/2;
        a0LP = 1 + alphaLP;
        a1LP = -2 * cosw0LP;
        a2LP = 1 - alphaLP;
        b0LP /= a0LP;
        b1LP /= a0LP;
        b2LP /= a0LP;
        a1LP /= a0LP;
        a2LP /= a0LP;

        freqHP = min(60,(getSampleRate()/2)-10);
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

        freqHPDE = min(3000,(getSampleRate()/2)-10);
        w0HPDE = 2 * cPi_ * freqHPDE/getSampleRate();
        cosw0HPDE = cos(w0HPDE);
        sinw0HPDE = sin(w0HPDE);
//       alphaHPDE = sinw0HPDE / cSqrt2_;
        alphaHPDE = sinw0HPDE /2 * sqrt( (1/2 - 1) + 2 );
        b0HPDE = (1 + cosw0HPDE)/2;
        b1HPDE = -(1 + cosw0HPDE);
        b2HPDE = (1 + cosw0HPDE)/2;
        a0HPDE = 1 + alphaHPDE;
        a1HPDE = -2 * cosw0HPDE;
        a2HPDE = 1 - alphaHPDE;
        b0HPDE /= a0HPDE;
        b1HPDE /= a0HPDE;
        b2HPDE /= a0HPDE;
        a1HPDE /= a0HPDE;
        a2HPDE /= a0HPDE; 

    attackC2 = exp( -60 / (0.02 * getSampleRate()) / cAmpDB_ );
    releaseC2 = exp( -60 / (0.1 * getSampleRate()) / cAmpDB_ );

    b1EnvLP = -exp(-2.0*cPi_*10 / getSampleRate() ); // 10Hz
    a0EnvLP = 1.0 + b1EnvLP;
    
    b1DEEnvLP = -exp(-2.0*cPi_*50 / getSampleRate() ); // 10Hz
    a0DEEnvLP = 1.0 + b1DEEnvLP;

}

void AVst::suspend()
{
    gainC1 = gainC2 = seekGainC2 = 1;   
    gainDE = 1;
    
    GRC1 = GRC1 = GRDE = 0;

    xl1HP = xl2HP = yl1HP = yl2HP =
    xr1HP = xr2HP = yr1HP = yr2HP = 0;

    xl1HPDE = xl2HPDE = yl1HPDE = yl2HPDE =
    xr1HPDE = xr2HPDE = yr1HPDE = yr2HPDE = 0;

    tmpEnvLP = 0;
    tmpDEEnvLP = 0;
}

//-------------------------------------------------------------------------------------------------------
void AVst::setProgramName (char *name)
{
	strcpy (programName, name);
}

//-----------------------------------------------------------------------------------------
void AVst::getProgramName (char *name)
{
	strcpy (name, programName);
}

//-----------------------------------------------------------------------------------------
void AVst::setParameter (long index, float value)
{

// Get the values from faders(sliders) and convert and stuff
    switch (index)
	{
		case  0 : 
            threshC1Fader = value;
            threshC1 = dB2Amp(-60 + threshC1Fader * 60);
        break;
        	
        case  1 : 
            threshC2Fader = value;
            threshC2 = dB2Amp(-60 + threshC2Fader * 60);
        break;

        case  2 : 
            threshDEFader = value;
            threshDE = dB2Amp(-60 + threshDEFader * 60);
        break;

        case  3 : 
            filters = ceil(value);
        break;

		case 4 :
            outVolumeFader = value;
            outVolume = dB2Amp( sign(outVolumeFader -.5)* sqr((outVolumeFader -.5)*2)*30 );
        break;

	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case  0 : v = threshC1Fader;      break;
		case  1 : v = threshC2Fader;       break;
		case  2 : v = threshDEFader;       break;
		case  3 : v = filters;        break;
		case  4 : v = outVolumeFader;  break;
		
		case  5 : v = GRC1;   break;
		case  6 : v = GRC2;   break;
		case  7 : v = GRDE;   break;

	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case  0 : strcpy (label, "Threshold (C1)");     break;
		case  1 : strcpy (label, "Threshold (C2)");         break;
		case  2 : strcpy (label, "Threshold (DE)");         break;
		case  3 : strcpy (label, "Filters");          break;
		case  4 : strcpy (label, "Out Volume");  break;
		
		case  5 : strcpy (label, "GR (C1)");        break;
		case  6 : strcpy (label, "GR (C2)");        break;
		case  7 : strcpy (label, "GR (DE)");        break;	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case  0 : float2string (amp2DB(threshC1), text);
            if (amp2DB(threshC1)>=0 && amp2DB(threshC1)<10 ) text[3] = NULL;       
            else if (((amp2DB(threshC1)<0)&&(amp2DB(threshC1)>-10)) || (amp2DB(threshC1)>10)) text[4] = NULL;       
            else text[5] = NULL;
        break;
		case  1 : float2string (amp2DB(threshC2), text);
            if (amp2DB(threshC2)>=0 && amp2DB(threshC2)<10 ) text[3] = NULL;       
            else if (((amp2DB(threshC2)<0)&&(amp2DB(threshC2)>-10)) || (amp2DB(threshC2)>10)) text[4] = NULL;
            else text[5] = NULL;
        break;
		case  2 : float2string (amp2DB(threshDE), text);
            if (amp2DB(threshDE)>=0 && amp2DB(threshDE)<10 ) text[3] = NULL;       
            else if (((amp2DB(threshDE)<0)&&(amp2DB(threshDE)>-10)) || (amp2DB(threshDE)>10)) text[4] = NULL;
            else text[5] = NULL;
        break;
		case  3 : float2string ( filters , text);       break;
		case  4 : float2string ( amp2DB(outVolume), text);
            if (amp2DB(outVolume)>=0 && amp2DB(outVolume)<10 ) text[3] = NULL;       
            else if (((amp2DB(outVolume)<0)&&(amp2DB(outVolume)>-10)) || (amp2DB(outVolume)>10)) text[4] = NULL;
            else text[5] = NULL;
        break;
		case  5 : float2string (GRC1, text);        break;
		case  6 : float2string (GRC2, text);        break;
		case  7 : float2string (GRDE, text);        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case  0 : strcpy (label, "dB");	    break;
		case  1 : strcpy (label, "dB");	    break;
		case  2 : strcpy (label, "dB");	    break;
		case  3 : strcpy (label, "Mode");	    break;
		case  4 : strcpy (label, "dB");	    break;

		case  5 : strcpy (label, "N/A");	    break;
		case  6 : strcpy (label, "N/A");	    break;
		case  7 : strcpy (label, "N/A");	    break;

	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Vocalizer");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Vocalizer");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getVendorString (char* text)
{
	strcpy (text, "");
	return true;
}

//-----------------------------------------------------------------------------------------
void AVst::process (float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    GRC1 = GRC2 = GRDE = 1;

  while (--sampleFrames >= 0)
  {
// =========================================
// FILTERING

    if (filters != 0)
    {
        tmpL = (*in1);
        (*in1) = b0LP * (*in1) + b1LP * xl1LP + b2LP * xl2LP - a1LP * yl1LP - a2LP * yl2LP;
        xl2LP = xl1LP;
        xl1LP = tmpL;
        yl2LP = yl1LP;
        yl1LP = (*in1) + cDcAdd_;
        tmpR = (*in2);
        (*in2) = b0LP * (*in2) + b1LP * xr1LP + b2LP * xr2LP - a1LP * yr1LP - a2LP * yr2LP;
        xr2LP = xr1LP;
        xr1LP = tmpR;
        yr2LP = yr1LP;
        yr1LP = (*in2) + cDcAdd_;

        tmpL = (*in1);
        (*in1) = b0HP * (*in1) + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
        xl2HP = xl1HP;
        xl1HP = tmpL;
        yl2HP = yl1HP;
        yl1HP = (*in1) + cDcAdd_;
        tmpR = (*in2);
        (*in2) = b0HP * (*in2) + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
        xr2HP = xr1HP;
        xr1HP = tmpR;
        yr2HP = yr1HP;
        yr1HP = (*in2) + cDcAdd_;
    }


// =========================================
// TRACK MAX: via absmax
    maxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    seekMaxSpls = sqrt( (tmpEnvLP = a0EnvLP*maxSpls - b1EnvLP*tmpEnvLP + cDcAdd_) - cDcAdd_ );

// =========================================
// GR-TRACKER: Calculate the current gain to be applied

    if (seekMaxSpls > threshC1) gainC1 = (threshC1 + (seekMaxSpls - threshC1) * 0.25) / seekMaxSpls ;
    else gainC1 = 1;
 
    seekMaxSpls = max(seekMaxSpls,maxSpls) * gainC1;

    if (seekMaxSpls > threshC2) seekGainC2 = threshC2 * pow(seekMaxSpls/threshC2,0.1f) / seekMaxSpls ;
    else gainC2 = 1;

// =========================================
// GAIN FOLLOWER: make 'gainC2' follow seekGain with the specified sttack and release speed
    if (gainC2 > seekGainC2) gainC2 = max( gainC2*attackC2 , seekGainC2 );
    else gainC2 = min( gainC2/releaseC2 , seekGainC2 );

    (*in1) *= (gainC1*gainC2);
    (*in2) *= (gainC1*gainC2);

    gTmpoL = (*in1);
    gTmpoR = (*in2);

    if (threshDE < 1)
    {

        tmpL = (*in1);
        (*in1) = b0HPDE * (*in1) + b1HPDE * xl1HPDE + b2HPDE * xl2HPDE - a1HPDE * yl1HPDE - a2HPDE * yl2HPDE;
        xl2HPDE = xl1HPDE;
        xl1HPDE = tmpL;
        yl2HPDE = yl1HPDE;
        yl1HPDE = (*in1) + cDcAdd_;
        tmpR = (*in2);
        (*in2) = b0HPDE * (*in2) + b1HPDE * xr1HPDE + b2HPDE * xr2HPDE - a1HPDE * yr1HPDE - a2HPDE * yr2HPDE;
        xr2HPDE = xr1HPDE;
        xr1HPDE = tmpR;
        yr2HPDE = yr1HPDE;
        yr1HPDE = (*in2) + cDcAdd_;

    }

    gTmpoL -= (*in1);
    gTmpoR -= (*in2);

    maxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    seekMaxSpls = max( sqrt( (tmpDEEnvLP = a0DEEnvLP*maxSpls - b1DEEnvLP*tmpDEEnvLP + cDcAdd_) - cDcAdd_ ) , maxSpls);

// =========================================
// GR-TRACKER: Calculate the current gain to be applied

    if (seekMaxSpls > threshDE) gainDE = threshDE / seekMaxSpls ;
    else gainDE = 1;
 

// =========================================
// MIXING: REPLACING!!!!!: mix output*volume + dry mix

    (*out1++) += ( gTmpoL + (*in1++)*gainDE ) * outVolume;
    (*out2++) += ( gTmpoR + (*in2++)*gainDE ) * outVolume;
    
    GRC1 = min(gainC1, GRC1);
    GRC2 = min(gainC2, GRC2);
    GRDE = min(gainDE, GRDE);
  } // eof while

    setParameterAutomated(5,GRC1=1-floor(GRC1));
    setParameterAutomated(6,GRC2=1-floor(GRC2+0.01));
    setParameterAutomated(7,GRDE=1-floor(GRDE+0.25));
       
}

//-----------------------------------------------------------------------------------------
void AVst::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
  float *in1  =  inputs[0];
  float *in2  =  inputs[1];
  float *out1 = outputs[0];
  float *out2 = outputs[1];

    GRC1 = GRC2 = GRDE = 1;

  while (--sampleFrames >= 0)
  {
// =========================================
// FILTERING

    if (filters != 0)
    {
        tmpL = (*in1);
        (*in1) = b0LP * (*in1) + b1LP * xl1LP + b2LP * xl2LP - a1LP * yl1LP - a2LP * yl2LP;
        xl2LP = xl1LP;
        xl1LP = tmpL;
        yl2LP = yl1LP;
        yl1LP = (*in1) + cDcAdd_;
        tmpR = (*in2);
        (*in2) = b0LP * (*in2) + b1LP * xr1LP + b2LP * xr2LP - a1LP * yr1LP - a2LP * yr2LP;
        xr2LP = xr1LP;
        xr1LP = tmpR;
        yr2LP = yr1LP;
        yr1LP = (*in2) + cDcAdd_;

        tmpL = (*in1);
        (*in1) = b0HP * (*in1) + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
        xl2HP = xl1HP;
        xl1HP = tmpL;
        yl2HP = yl1HP;
        yl1HP = (*in1) + cDcAdd_;
        tmpR = (*in2);
        (*in2) = b0HP * (*in2) + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
        xr2HP = xr1HP;
        xr1HP = tmpR;
        yr2HP = yr1HP;
        yr1HP = (*in2) + cDcAdd_;
    }


// =========================================
// TRACK MAX: via absmax
    maxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    seekMaxSpls = sqrt( (tmpEnvLP = a0EnvLP*maxSpls - b1EnvLP*tmpEnvLP + cDcAdd_) - cDcAdd_ );

// =========================================
// GR-TRACKER: Calculate the current gain to be applied

    if (seekMaxSpls > threshC1) gainC1 = (threshC1 + (seekMaxSpls - threshC1) * 0.25) / seekMaxSpls ;
    else gainC1 = 1;
 
    seekMaxSpls = max(seekMaxSpls,maxSpls) * gainC1;

    if (seekMaxSpls > threshC2) seekGainC2 = threshC2 * pow(seekMaxSpls/threshC2,0.1f) / seekMaxSpls ;
    else gainC2 = 1;

// =========================================
// GAIN FOLLOWER: make 'gainC2' follow seekGain with the specified sttack and release speed
    if (gainC2 > seekGainC2) gainC2 = max( gainC2*attackC2 , seekGainC2 );
    else gainC2 = min( gainC2/releaseC2 , seekGainC2 );

    (*in1) *= (gainC1*gainC2);
    (*in2) *= (gainC1*gainC2);

    gTmpoL = (*in1);
    gTmpoR = (*in2);

    if (threshDE < 1)
    {

        tmpL = (*in1);
        (*in1) = b0HPDE * (*in1) + b1HPDE * xl1HPDE + b2HPDE * xl2HPDE - a1HPDE * yl1HPDE - a2HPDE * yl2HPDE;
        xl2HPDE = xl1HPDE;
        xl1HPDE = tmpL;
        yl2HPDE = yl1HPDE;
        yl1HPDE = (*in1) + cDcAdd_;
        tmpR = (*in2);
        (*in2) = b0HPDE * (*in2) + b1HPDE * xr1HPDE + b2HPDE * xr2HPDE - a1HPDE * yr1HPDE - a2HPDE * yr2HPDE;
        xr2HPDE = xr1HPDE;
        xr1HPDE = tmpR;
        yr2HPDE = yr1HPDE;
        yr1HPDE = (*in2) + cDcAdd_;

    }

    gTmpoL -= (*in1);
    gTmpoR -= (*in2);

    maxSpls = absmax( (*in1) , (*in2) );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    seekMaxSpls = max( sqrt( (tmpDEEnvLP = a0DEEnvLP*maxSpls - b1DEEnvLP*tmpDEEnvLP + cDcAdd_) - cDcAdd_ ) , maxSpls);

// =========================================
// GR-TRACKER: Calculate the current gain to be applied

    if (seekMaxSpls > threshDE) gainDE = threshDE / seekMaxSpls ;
    else gainDE = 1;
 

// =========================================
// MIXING: REPLACING!!!!!: mix output*volume + dry mix

    (*out1++) = ( gTmpoL + (*in1++)*gainDE ) * outVolume;
    (*out2++) = ( gTmpoR + (*in2++)*gainDE ) * outVolume;
    
    GRC1 = min(gainC1, GRC1);
    GRC2 = min(gainC2, GRC2);
    GRDE = min(gainDE, GRDE);
  } // eof while

    setParameterAutomated(5,GRC1=1-floor(GRC1));
    setParameterAutomated(6,GRC2=1-floor(GRC2+0.01));
    setParameterAutomated(7,GRDE=1-floor(GRDE+0.25));

}
