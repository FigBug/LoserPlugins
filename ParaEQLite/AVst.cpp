//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>

// Mich's Formulas V 0.3

#define cAmpDB_     8.656170245 // 6/log(2);
#define cDBAmp_     0.115524530 // log(2)/6;
#define cPi_        3.141592654
#define cSqrt2_     1.414213562 // sqrt(2);
#define cSqrt2h_    0.707106781 // sqrt(0.5f);
#define cDcAdd_     1e-30
#define cDC_     1e-30
#define cDCBig_     1e-10

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
    if (in >  limit) return  limit;
    if (in < -limit) return -limit;
    return in;
}

// eof Mich's formulas

#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 15)	// 1 program, 2 parameter only
{

    freqHPFader = 0;

    freqHP = 5;
                
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

    freqLSFader = 0.21395168;
    freqLS = 240;
    dBgainLS = 0;
    SLS = 1;
    dBgainLSFader = 0.5;
    setLS();

    freqHSFader = 0.793645392;
    freqHS = 12000;
    dBgainHS = 0;
    SHS = 0.5;
    dBgainHSFader = 0.5;
    setHS();

    freqP1Fader = 0.20105025;
    freqP1 = 200;
    dBgainP1 = 0;
    BWP1 = 1;
    dBgainP1Fader = 0.5;
    setP1();

    freqP2Fader = 0.32118022;
    freqP2 = 800;
    dBgainP2 = 0;
    BWP2 = 1;
    dBgainP2Fader = 0.5;
    setP2();

    freqP3Fader = 0.470236784;
    freqP3 = 2500;
    dBgainP3 = 0;
    BWP3 = 1;
    dBgainP3Fader = 0.5;
    setP3();

    outVolume = 1;
    outVolumeFader = 0.5;

///

	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('P','E','Q','L') );
	canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

    suspend();
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

}

void AVst::setLS()
{
    ALS = dB2Amp(dBgainLS/2);
    w0LS = 2*cPi_*freqLS/getSampleRate();
    alphaLS = sin(w0LS)/2 * sqrt( (ALS + 1/ALS)*(1/ SLS  - 1) + 2 );
//    alphaP1 = sin(w0P1)*sinh( log(2)/2 * BWP1 * w0P1/sin(w0P1) );

    b0LS =    ALS*( (ALS+1) - (ALS-1)*cos(w0LS) + 2*sqrt(ALS)*alphaLS );
    b1LS =  2*ALS*( (ALS-1) - (ALS+1)*cos(w0LS)                       );
    b2LS =    ALS*( (ALS+1) - (ALS-1)*cos(w0LS) - 2*sqrt(ALS)*alphaLS );
    a0LS =          (ALS+1) + (ALS-1)*cos(w0LS) + 2*sqrt(ALS)*alphaLS  ;
    a1LS =     -2*( (ALS-1) + (ALS+1)*cos(w0LS)                       );
    a2LS =          (ALS+1) + (ALS-1)*cos(w0LS) - 2*sqrt(ALS)*alphaLS  ;
    
    b0LS /= a0LS;
    b1LS /= a0LS;
    b2LS /= a0LS;
    a1LS /= a0LS;
    a2LS /= a0LS;
}

void AVst::setHS()
{
    AHS = dB2Amp(dBgainHS/2);
    w0HS = 2*cPi_*freqHS/getSampleRate();
    alphaHS = sin(w0HS)/2 * sqrt( (AHS + 1/AHS)*(1/ SHS - 1) + 2 );
//    alphaP1 = sin(w0P1)*sinh( log(2)/2 * BWP1 * w0P1/sin(w0P1) );
    b0HS =    AHS*( (AHS+1) + (AHS-1)*cos(w0HS) + 2*sqrt(AHS)*alphaHS );
    b1HS = -2*AHS*( (AHS-1) + (AHS+1)*cos(w0HS)                       );
    b2HS =    AHS*( (AHS+1) + (AHS-1)*cos(w0HS) - 2*sqrt(AHS)*alphaHS );
    a0HS =          (AHS+1) - (AHS-1)*cos(w0HS) + 2*sqrt(AHS)*alphaHS  ;
    a1HS =      2*( (AHS-1) - (AHS+1)*cos(w0HS)                       );
    a2HS =          (AHS+1) - (AHS-1)*cos(w0HS) - 2*sqrt(AHS)*alphaHS  ;
    
    b0HS /= a0HS;
    b1HS /= a0HS;
    b2HS /= a0HS;
    a1HS /= a0HS;
    a2HS /= a0HS;
}

void AVst::setP1()
{
    AP1 = dB2Amp(dBgainP1/2);
    w0P1 = 2*cPi_*freqP1/getSampleRate();
    alphaP1 = sin(w0P1)*sinh( log(2)/2 * BWP1 * w0P1/sin(w0P1) );
//    alphaP1 = sin(w0P1)/(2*QP1);
    
    b0P1 =   1 + alphaP1*AP1;
    b1P1 =  -2*cos(w0P1)    ;
    b2P1 =   1 - alphaP1*AP1;
    a0P1 =   1 + alphaP1/AP1;
    a1P1 =  -2*cos(w0P1)    ;
    a2P1 =   1 - alphaP1/AP1;
    
    b0P1 /= a0P1;
    b1P1 /= a0P1;
    b2P1 /= a0P1;
    a1P1 /= a0P1;
    a2P1 /= a0P1;
    
}

void AVst::setP2()
{
    AP2 = dB2Amp(dBgainP2/2);
    w0P2 = 2*cPi_*freqP2/getSampleRate();
    alphaP2 = sin(w0P2)*sinh( log(2)/2 * BWP2 * w0P2/sin(w0P2) );
//    alphaP2 = sin(w0P2)/(2*QP2);
    
    b0P2 =   1 + alphaP2*AP2;
    b1P2 =  -2*cos(w0P2)    ;
    b2P2 =   1 - alphaP2*AP2;
    a0P2 =   1 + alphaP2/AP2;
    a1P2 =  -2*cos(w0P2)    ;
    a2P2 =   1 - alphaP2/AP2;
    
    b0P2 /= a0P2;
    b1P2 /= a0P2;
    b2P2 /= a0P2;
    a1P2 /= a0P2;
    a2P2 /= a0P2;
    
}

void AVst::setP3()
{
    AP3 = dB2Amp(dBgainP3/2);
    w0P3 = 2*cPi_*freqP3/getSampleRate();
    alphaP3 = sin(w0P3)*sinh( log(2)/2 * BWP3 * w0P3/sin(w0P3) );
//    alphaP3 = sin(w0P3)/(2*QP3);
    
    b0P3 =   1 + alphaP3*AP3;
    b1P3 =  -2*cos(w0P3)    ;
    b2P3 =   1 - alphaP3*AP3;
    a0P3 =   1 + alphaP3/AP3;
    a1P3 =  -2*cos(w0P3)    ;
    a2P3 =   1 - alphaP3/AP3;
    
    b0P3 /= a0P3;
    b1P3 /= a0P3;
    b2P3 /= a0P3;
    a1P3 /= a0P3;
    a2P3 /= a0P3;
    
}

void AVst::suspend()
{
 
    xl1HP = xl2HP = yl1HP = yl2HP =
    xr1HP = xr2HP = yr1HP = yr2HP = 0;

    xl1LS = xl2LS = yl1LS = yl2LS =
    xr1LS = xr2LS = yr1LS = yr2LS =
    xl1HS = xl2HS = yl1HS = yl2HS =
    xr1HS = xr2HS = yr1HS = yr2HS = 0;
    
    xl1P1 = xl2P1 = yl1P1 = yl2P1 =
    xr1P1 = xr2P1 = yr1P1 = yr2P1 = 0;

    xl1P2 = xl2P2 = yl1P2 = yl2P2 =
    xr1P2 = xr2P2 = yr1P2 = yr2P2 = 0;

    xl1P3 = xl2P3 = yl1P3 = yl2P3 =
    xr1P3 = xr2P3 = yr1P3 = yr2P3 = 0;

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

    switch (index)
	{     	

        case 0 :
        freqHPFader = value;
        freqHP = min(5+freqHPFader*freqHPFader*freqHPFader*23995,(getSampleRate()/2)-10);
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
        
        case 1 :
            freqLSFader = value;
            freqLS = min(5+freqLSFader*freqLSFader*freqLSFader*23995,(getSampleRate()/2.205));
            setLS();    
        break;

        case 2 :
            dBgainLSFader = value;
            dBgainLS = sign(dBgainLSFader -.5)* sqr((dBgainLSFader -.5)*2)*24;
            setLS();
        break;

        case 3 :
            freqP1Fader = value;
            freqP1 = min(5+freqP1Fader*freqP1Fader*freqP1Fader*23995,(getSampleRate()/2.205));
            setP1();    
        break;
                
        case 4 :
            BWP1 = 0.05 + sqr(value) * 4.95; setP1();
        break;

        case 5 :
            dBgainP1Fader = value;
            dBgainP1 = sign(dBgainP1Fader -.5)* sqr((dBgainP1Fader -.5)*2)*24;
            setP1();
        break;


        case 6 :
            freqP2Fader = value;
            freqP2 = min(5+freqP2Fader*freqP2Fader*freqP2Fader*23995,(getSampleRate()/2.205));
            setP2();    
        break;
                
        case 7 :
            BWP2 = 0.05 + sqr(value) * 4.95; setP2();
        break;

        case 8 :
            dBgainP2Fader = value;
            dBgainP2 = sign(dBgainP2Fader -.5)* sqr((dBgainP2Fader -.5)*2)*24;
            setP2();
        break;

        case 9 :
            freqP3Fader = value;
            freqP3 = min(5+freqP3Fader*freqP3Fader*freqP3Fader*23995,(getSampleRate()/2.205));
            setP3();    
        break;
                
        case 10 :
            BWP3 = 0.05 + sqr(value) * 4.95; setP3();
        break;

        case 11 :
            dBgainP3Fader = value;
            dBgainP3 = sign(dBgainP3Fader -.5)* sqr((dBgainP3Fader -.5)*2)*24;
            setP3();
        break;

        case 12 :
            freqHSFader = value;
            freqHS = min(5+freqHSFader*freqHSFader*freqHSFader*23995,(getSampleRate()/2.205));
            setHS();    
        break;

        case 13 :
            dBgainHSFader = value;
            dBgainHS = sign(dBgainHSFader -.5)* sqr((dBgainHSFader -.5)*2)*24;
            setHS();
        break;
              
        case 14 :
            outVolumeFader = value;
            outVolume = dB2Amp( sign(outVolumeFader -.5)* sqr((outVolumeFader -.5)*2)*12 );
        break;
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = freqHPFader;   break;
		case 1 : v = freqLSFader;   break;
		case 2 : v = dBgainLSFader;   break;
		case 3 : v = freqP1Fader;   break;
		case 4 : v = sqrt((BWP1-0.05)/4.95);   break;
		case 5 : v = dBgainP1Fader;   break;
		case 6 : v = freqP2Fader;   break;
		case 7 : v = sqrt((BWP2-0.05)/4.95);   break;
		case 8 : v = dBgainP2Fader;   break;
		case 9 : v = freqP3Fader;   break;
		case 10 : v = sqrt((BWP3-0.05)/4.95);   break;
		case 11 : v = dBgainP3Fader;   break;
		case 12 : v = freqHSFader;   break;
		case 13 : v = dBgainHSFader;   break;
		case 14 : v = outVolumeFader;        break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
     	switch (index)
	{
		case 0 : strcpy (label, "Highpass - - - -");               break;
		case 1 : strcpy (label, "Low Shelving - - - -");               break;
		case 2 : strcpy (label, "Gain");               break;
		case 3 : strcpy (label, "Peaking - - - -");               break;
		case 4 : strcpy (label, "Width");               break;
		case 5 : strcpy (label, "Gain");               break;
		case 6 : strcpy (label, "Peaking - - - -");               break;
		case 7 : strcpy (label, "Width");               break;
		case 8 : strcpy (label, "Gain");               break;
		case 9  : strcpy (label, "Peaking - - - -");               break;
		case 10 : strcpy (label, "Width");               break;
		case 11 : strcpy (label, "Gain");               break;
		case 12 : strcpy (label, "High Shelving - - - -");               break;
		case 13 : strcpy (label, "Gain");               break;
		case 14 : strcpy (label, "Output - - - -");               break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 :
            if (freqHPFader > 0) float2string (freqHP, text);
            else if (freqHPFader == 0) strcpy (text, "OFF");
        break;
        case 1 :
            float2string (freqLS, text);
        break;
        case 2 :
            if (dBgainLS) float2string (dBgainLS, text);
            else strcpy (text, "OFF");
        break;
        case 3 :
            float2string (freqP1, text);
        break;
        case 4 :
            float2string (BWP1, text);
        break;
        case 5 :
            if (dBgainP1) float2string (dBgainP1, text);
            else strcpy (text, "OFF");
        break;
        case 6 :
            float2string (freqP2, text);
        break;
        case 7 :
            float2string (BWP2, text);
        break;
        case 8 :
            if (dBgainP2) float2string (dBgainP2, text);
            else strcpy (text, "OFF");
        break;
        case 9 :
            float2string (freqP3, text);
        break;
        case 10 :
            float2string (BWP3, text);
        break;
        case 11 :
            if (dBgainP3) float2string (dBgainP3, text);
            else strcpy (text, "OFF");
        break;
        case 12 :
            float2string (freqHS, text);
        break;
        case 13 :
            if (dBgainHS) float2string (dBgainHS, text);
            else strcpy (text, "OFF");
        break;
		case 14 :
            float2string (amp2DB(outVolume), text);
        break;
	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "Hz");	break;
		case 1 : strcpy (label, "Hz");	break;
		case 2 : strcpy (label, "dB");	break;
		case 3 : strcpy (label, "Hz");	break;
		case 4 : strcpy (label, "Oct");	break;
		case 5 : strcpy (label, "dB");	break;
		case 6 : strcpy (label, "Hz");	break;
		case 7 : strcpy (label, "Oct");	break;
		case 8 : strcpy (label, "dB");	break;
		case 9 : strcpy (label, "Hz");	break;
		case 10 : strcpy (label, "Oct");	break;
		case 11 : strcpy (label, "dB");	break;
		case 12 : strcpy (label, "Hz");	break;
		case 13 : strcpy (label, "dB");	break;
		case 14 : strcpy (label, "dB");	break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Parametric EQ LITE");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Parametric EQ LITE");
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

    while (--sampleFrames >= 0)
    {
    (*in1) += cDC_;
    (*in2) += cDC_;

    if (dBgainP1 != 0)
    {
        tmp = (*in1);
        (*in1) = b0P1 * (*in1) + b1P1 * xl1P1 + b2P1 * xl2P1 - a1P1 * yl1P1 - a2P1 * yl2P1;
        xl2P1 = xl1P1; xl1P1 = tmp; yl2P1 = yl1P1; yl1P1 = (*in1);
    
        tmp = (*in2);
        (*in2) = b0P1 * (*in2) + b1P1 * xr1P1 + b2P1 * xr2P1 - a1P1 * yr1P1 - a2P1 * yr2P1;
        xr2P1 = xr1P1; xr1P1 = tmp; yr2P1 = yr1P1; yr1P1 = (*in2);
    }

    if (dBgainP2 != 0)
    {
        tmp = (*in1);
        (*in1) = b0P2 * (*in1) + b1P2 * xl1P2 + b2P2 * xl2P2 - a1P2 * yl1P2 - a2P2 * yl2P2;
        xl2P2 = xl1P2; xl1P2 = tmp; yl2P2 = yl1P2; yl1P2 = (*in1);
    
        tmp = (*in2);
        (*in2) = b0P2 * (*in2) + b1P2 * xr1P2 + b2P2 * xr2P2 - a1P2 * yr1P2 - a2P2 * yr2P2;
        xr2P2 = xr1P2; xr1P2 = tmp; yr2P2 = yr1P2; yr1P2 = (*in2);
    }

    if (dBgainP3 != 0)
    {
        tmp = (*in1);
        (*in1) = b0P3 * (*in1) + b1P3 * xl1P3 + b2P3 * xl2P3 - a1P3 * yl1P3 - a2P3 * yl2P3;
        xl2P3 = xl1P3; xl1P3 = tmp; yl2P3 = yl1P3; yl1P3 = (*in1);
    
        tmp = (*in2);
        (*in2) = b0P3 * (*in2) + b1P3 * xr1P3 + b2P3 * xr2P3 - a1P3 * yr1P3 - a2P3 * yr2P3;
        xr2P3 = xr1P3; xr1P3 = tmp; yr2P3 = yr1P3; yr1P3 = (*in2);
    }

    if (dBgainLS != 0)
    {
        tmp = (*in1);
        (*in1) = b0LS * (*in1) + b1LS * xl1LS + b2LS * xl2LS - a1LS * yl1LS - a2LS * yl2LS;
        xl2LS = xl1LS; xl1LS = tmp; yl2LS = yl1LS; yl1LS = (*in1);
    
        tmp = (*in2);
        (*in2) = b0LS * (*in2) + b1LS * xr1LS + b2LS * xr2LS - a1LS * yr1LS - a2LS * yr2LS;
        xr2LS = xr1LS; xr1LS = tmp; yr2LS = yr1LS; yr1LS = (*in2);
    }
    
    if (dBgainHS != 0)
    {
        tmp = (*in1);
        (*in1) = b0HS * (*in1) + b1HS * xl1HS + b2HS * xl2HS - a1HS * yl1HS - a2HS * yl2HS;
        xl2HS = xl1HS; xl1HS = tmp; yl2HS = yl1HS; yl1HS = (*in1);
    
        tmp = (*in2);
        (*in2) = b0HS * (*in2) + b1HS * xr1HS + b2HS * xr2HS - a1HS * yr1HS - a2HS * yr2HS;
        xr2HS = xr1HS; xr1HS = tmp; yr2HS = yr1HS; yr1HS = (*in2);
    }

    if (freqHPFader > 0)
    {
            tmp = (*in1);
            (*in1) = b0HP * (*in1) + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
            xl2HP = xl1HP; xl1HP = tmp; yl2HP = yl1HP; yl1HP = (*in1) +cDC_;
            
            tmp = (*in2);
            (*in2) = b0HP * (*in2) + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
            xr2HP = xr1HP; xr1HP = tmp; yr2HP = yr1HP; yr1HP = (*in2) +cDC_;
    }

    (*out1++) += (*in1++)*outVolume;
    (*out2++) += (*in2++)*outVolume;

    }
}

//-----------------------------------------------------------------------------------------
void AVst::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    while (--sampleFrames >= 0)
    {

    (*in1) += cDC_;
    (*in2) += cDC_;

    if (dBgainP1 != 0)
    {
        tmp = (*in1);
        (*in1) = b0P1 * (*in1) + b1P1 * xl1P1 + b2P1 * xl2P1 - a1P1 * yl1P1 - a2P1 * yl2P1;
        xl2P1 = xl1P1; xl1P1 = tmp; yl2P1 = yl1P1; yl1P1 = (*in1);
    
        tmp = (*in2);
        (*in2) = b0P1 * (*in2) + b1P1 * xr1P1 + b2P1 * xr2P1 - a1P1 * yr1P1 - a2P1 * yr2P1;
        xr2P1 = xr1P1; xr1P1 = tmp; yr2P1 = yr1P1; yr1P1 = (*in2);
    }

    if (dBgainP2 != 0)
    {
        tmp = (*in1);
        (*in1) = b0P2 * (*in1) + b1P2 * xl1P2 + b2P2 * xl2P2 - a1P2 * yl1P2 - a2P2 * yl2P2;
        xl2P2 = xl1P2; xl1P2 = tmp; yl2P2 = yl1P2; yl1P2 = (*in1);
    
        tmp = (*in2);
        (*in2) = b0P2 * (*in2) + b1P2 * xr1P2 + b2P2 * xr2P2 - a1P2 * yr1P2 - a2P2 * yr2P2;
        xr2P2 = xr1P2; xr1P2 = tmp; yr2P2 = yr1P2; yr1P2 = (*in2);
    }

    if (dBgainP3 != 0)
    {
        tmp = (*in1);
        (*in1) = b0P3 * (*in1) + b1P3 * xl1P3 + b2P3 * xl2P3 - a1P3 * yl1P3 - a2P3 * yl2P3;
        xl2P3 = xl1P3; xl1P3 = tmp; yl2P3 = yl1P3; yl1P3 = (*in1);
    
        tmp = (*in2);
        (*in2) = b0P3 * (*in2) + b1P3 * xr1P3 + b2P3 * xr2P3 - a1P3 * yr1P3 - a2P3 * yr2P3;
        xr2P3 = xr1P3; xr1P3 = tmp; yr2P3 = yr1P3; yr1P3 = (*in2);
    }

    if (dBgainLS != 0)
    {
        tmp = (*in1);
        (*in1) = b0LS * (*in1) + b1LS * xl1LS + b2LS * xl2LS - a1LS * yl1LS - a2LS * yl2LS;
        xl2LS = xl1LS; xl1LS = tmp; yl2LS = yl1LS; yl1LS = (*in1);
    
        tmp = (*in2);
        (*in2) = b0LS * (*in2) + b1LS * xr1LS + b2LS * xr2LS - a1LS * yr1LS - a2LS * yr2LS;
        xr2LS = xr1LS; xr1LS = tmp; yr2LS = yr1LS; yr1LS = (*in2);
    }
    
    if (dBgainHS != 0)
    {
        tmp = (*in1);
        (*in1) = b0HS * (*in1) + b1HS * xl1HS + b2HS * xl2HS - a1HS * yl1HS - a2HS * yl2HS;
        xl2HS = xl1HS; xl1HS = tmp; yl2HS = yl1HS; yl1HS = (*in1);
    
        tmp = (*in2);
        (*in2) = b0HS * (*in2) + b1HS * xr1HS + b2HS * xr2HS - a1HS * yr1HS - a2HS * yr2HS;
        xr2HS = xr1HS; xr1HS = tmp; yr2HS = yr1HS; yr1HS = (*in2);
    }

    if (freqHPFader > 0)
    {
            tmp = (*in1);
            (*in1) = b0HP * (*in1) + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
            xl2HP = xl1HP; xl1HP = tmp; yl2HP = yl1HP; yl1HP = (*in1) +cDC_;
            
            tmp = (*in2);
            (*in2) = b0HP * (*in2) + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
            xr2HP = xr1HP; xr1HP = tmp; yr2HP = yr1HP; yr1HP = (*in2) +cDC_;
    }

    (*out1++) = (*in1++)*outVolume;
    (*out2++) = (*in2++)*outVolume;

    }
}
