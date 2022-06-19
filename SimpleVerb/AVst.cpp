//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 1.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <algorithm>
#include <stdlib.h>


// Mich's Formulas V 0.4

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

#define $_C1_$  1
#define $_C2_$  1.09
#define $_C3_$  1.16
#define $_C4_$  1.23
#define $_C5_$  1.32
#define $_C6_$  1.41
#define $_C7_$  1.45
#define $_C8_$  1.56
#define $_C9_$  1.66
#define $_C10_$  1.71
#define $_C11_$  1.80
#define $_C12_$  1.90

#define $_AL1_$  1.0
#define $_AL2_$  2.5
#define $_AL3_$  5.0
#define $_AR1_$  1.0
#define $_AR2_$  2.5
#define $_AR3_$  5.0

#define $_SW_$  1.0

#define $_AP1FBQ_$  0.6
#define $_AP2FBQ_$  0.6
#define $_AP3FBQ_$  0.6


//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 7)	// 1 program, 7 parameters
{

    roomSizeFader = 0.5;
    roomSize = 55;

    preDelayFader = 0;

    preDelayLength = preDelayMaxLength = 0;
	preDelay = new float[preDelayLength];
    preDelayPos = 0;

    comb1Length = comb1MaxLength = static_cast<unsigned long>( $_C1_$ * roomSize * getSampleRate() / 1000 );
	comb1 = new float[comb1Length];
    comb1Pos = 0;

    comb2Length = comb2MaxLength = static_cast<unsigned long>( $_C2_$ * roomSize * getSampleRate() / 1000 );
	comb2 = new float[comb2Length];
    comb2Pos = 0;

    comb3Length = comb3MaxLength = static_cast<unsigned long>( $_C3_$ * roomSize * getSampleRate() / 1000 );
	comb3 = new float[comb3Length];
    comb3Pos = 0;

    comb4Length = comb4MaxLength = static_cast<unsigned long>( $_C4_$ * roomSize * getSampleRate() / 1000 );
	comb4 = new float[comb4Length];
    comb4Pos = 0;

    comb5Length = comb5MaxLength = static_cast<unsigned long>( $_C5_$ * roomSize * getSampleRate() / 1000 );
	comb5 = new float[comb5Length];
    comb5Pos = 0;

    comb6Length = comb6MaxLength = static_cast<unsigned long>( $_C6_$ * roomSize * getSampleRate() / 1000 );
	comb6 = new float[comb6Length];
    comb6Pos = 0;

    comb7Length = comb7MaxLength = static_cast<unsigned long>( $_C7_$ * roomSize * getSampleRate() / 1000 );
	comb7 = new float[comb7Length];
    comb7Pos = 0;

    comb8Length = comb8MaxLength = static_cast<unsigned long>( $_C8_$ * roomSize * getSampleRate() / 1000 );
	comb8 = new float[comb8Length];
    comb8Pos = 0;

    comb9Length = comb9MaxLength = static_cast<unsigned long>( $_C9_$ * roomSize * getSampleRate() / 1000 );
	comb9 = new float[comb9Length];
    comb9Pos = 0;

    comb10Length = comb10MaxLength = static_cast<unsigned long>( $_C10_$ * roomSize * getSampleRate() / 1000 );
	comb10 = new float[comb10Length];
    comb10Pos = 0;

    comb11Length = comb11MaxLength = static_cast<unsigned long>( $_C11_$ * roomSize * getSampleRate() / 1000 );
	comb11 = new float[comb11Length];
    comb11Pos = 0;

    comb12Length = comb12MaxLength = static_cast<unsigned long>( $_C12_$ * roomSize * getSampleRate() / 1000 );
	comb12 = new float[comb12Length];
    comb12Pos = 0;

    allpassL1Length = static_cast<unsigned long>( $_AL1_$ * getSampleRate() / 1000 );
	allpassL1 = new float[allpassL1Length];
    allpassL1Pos = 0;

    allpassL2Length = static_cast<unsigned long>( ( $_AL2_$  + $_SW_$ ) * getSampleRate() / 1000 );
	allpassL2 = new float[allpassL2Length];
    allpassL2Pos = 0;

    allpassL3Length = static_cast<unsigned long>( $_AL3_$ * getSampleRate() / 1000 );
	allpassL3 = new float[allpassL3Length];
    allpassL3Pos = 0;

    allpassR1Length = static_cast<unsigned long>( ( $_AR1_$ + $_SW_$ ) * getSampleRate() / 1000 );
	allpassR1 = new float[allpassR1Length];
    allpassR1Pos = 0;

    allpassR2Length = static_cast<unsigned long>( $_AR2_$ * getSampleRate() / 1000 );
	allpassR2 = new float[allpassR2Length];
    allpassR2Pos = 0;

    allpassR3Length = static_cast<unsigned long>( ( $_AR3_$  + $_SW_$ ) * getSampleRate() / 1000 );
	allpassR3 = new float[allpassR3Length];
    allpassR3Pos = 0;

    dampFader = 0.5;
    damp = 0.25; 

    freqLPFader = 1;
    freqHPFader = 0;

    freqLP = 24000;
    freqHP = 0;
    
    b1LP = -exp(-2.0*cPi_*freqLP/ getSampleRate() ); // 100Hz
    a0LP = 1.0 + b1LP;

    b1HP = -exp(-2.0*cPi_*freqHP/ getSampleRate() ); // 100Hz
    a0HP = 1.0 + b1HP;

    dry = 1;
    wet = 0.5;
    
	setNumInputs (2);
	setNumOutputs (2);
	setUniqueID (CCONST ('S','P','V','B') );
//    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

	suspend(); // flush buffers and reset variables
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{
	if (comb1)
        delete[] comb1;
	if (comb2)
        delete[] comb2;
	if (comb3)
        delete[] comb3;
	if (comb4)
        delete[] comb4;
	if (comb5)
        delete[] comb5;
	if (comb6)
        delete[] comb6;
	if (comb7)
        delete[] comb7;
	if (comb8)
        delete[] comb8;
	if (comb9)
        delete[] comb9;
	if (comb10)
        delete[] comb10;
	if (comb11)
        delete[] comb11;
	if (comb12)
        delete[] comb12;
    if (allpassL1)
        delete[] allpassL1;
    if (allpassL2)
        delete[] allpassL2;
    if (allpassL3)
        delete[] allpassL3;
    if (allpassR1)
        delete[] allpassR1;
    if (allpassR2)
        delete[] allpassR2;
    if (allpassR3)
        delete[] allpassR3;

}

void AVst::FlushPreDelay ()
{
    memset (preDelay, 0, preDelayLength * sizeof(float));
}

void AVst::FlushBuffers ()
{
    memset (comb1, 0, comb1Length * sizeof(float));
    memset (comb2, 0, comb2Length * sizeof(float));
    memset (comb3, 0, comb3Length * sizeof(float));
    memset (comb4, 0, comb4Length * sizeof(float));
    memset (comb5, 0, comb5Length * sizeof(float));
    memset (comb6, 0, comb6Length * sizeof(float));
    memset (comb7, 0, comb7Length * sizeof(float));
    memset (comb8, 0, comb8Length * sizeof(float));
    memset (comb9, 0, comb9Length * sizeof(float));
    memset (comb10, 0, comb10Length * sizeof(float));
    memset (comb11, 0, comb11Length * sizeof(float));
    memset (comb12, 0, comb12Length * sizeof(float));
    memset (allpassL1, 0, allpassL1Length * sizeof(float));
    memset (allpassL2, 0, allpassL2Length * sizeof(float));
    memset (allpassL3, 0, allpassL3Length * sizeof(float));
    memset (allpassR1, 0, allpassR1Length * sizeof(float));
    memset (allpassR2, 0, allpassR2Length * sizeof(float));
    memset (allpassR3, 0, allpassR3Length * sizeof(float));
}

void AVst::ReallocPreDelay ()
{
    preDelay = (float*) realloc(preDelay, preDelayMaxLength * sizeof(float));
}

void AVst::ReallocComb1 ()
{
    comb1 = (float*) realloc(comb1, comb1MaxLength * sizeof(float));
}

void AVst::ReallocComb2 ()
{
    comb2 = (float*) realloc(comb2, comb2MaxLength * sizeof(float));
}

void AVst::ReallocComb3 ()
{
    comb3 = (float*) realloc(comb3, comb3MaxLength * sizeof(float));
}

void AVst::ReallocComb4 ()
{
    comb4 = (float*) realloc(comb4, comb4MaxLength * sizeof(float));
}

void AVst::ReallocComb5 ()
{
    comb5 = (float*) realloc(comb5, comb5MaxLength * sizeof(float));
}

void AVst::ReallocComb6 ()
{
    comb6 = (float*) realloc(comb6, comb6MaxLength * sizeof(float));
}

void AVst::ReallocComb7 ()
{
    comb7 = (float*) realloc(comb7, comb7MaxLength * sizeof(float));
}

void AVst::ReallocComb8 ()
{
    comb8 = (float*) realloc(comb8, comb8MaxLength * sizeof(float));
}

void AVst::ReallocComb9 ()
{
    comb9 = (float*) realloc(comb9, comb9MaxLength * sizeof(float));
}

void AVst::ReallocComb10 ()
{
    comb10 = (float*) realloc(comb10, comb10MaxLength * sizeof(float));
}

void AVst::ReallocComb11 ()
{
    comb11 = (float*) realloc(comb11, comb11MaxLength * sizeof(float));
}

void AVst::ReallocComb12 ()
{
    comb12 = (float*) realloc(comb12, comb12MaxLength * sizeof(float));
}

void AVst::ReallocAllpassL1 ()
{
    allpassL1 = (float*) realloc(allpassL1, allpassL1Length * sizeof(float));
}

void AVst::ReallocAllpassL2 ()
{
    allpassL2 = (float*) realloc(allpassL2, allpassL2Length * sizeof(float));
}

void AVst::ReallocAllpassL3 ()
{
    allpassL3 = (float*) realloc(allpassL3, allpassL3Length * sizeof(float));
}

void AVst::ReallocAllpassR1 ()
{
    allpassR1 = (float*) realloc(allpassR1, allpassR1Length * sizeof(float));
}

void AVst::ReallocAllpassR2 ()
{
    allpassR2 = (float*) realloc(allpassR2, allpassR2Length * sizeof(float));
}

void AVst::ReallocAllpassR3 ()
{
    allpassR3 = (float*) realloc(allpassR3, allpassR3Length * sizeof(float));
}

void AVst::suspend()
{
 
    if (preDelayMaxLength != preDelayLength)
    { 
        preDelayMaxLength = preDelayLength;
        ReallocPreDelay();
    } 
    if (comb1MaxLength != comb1Length)
    { 
        comb1MaxLength = comb1Length;
        ReallocComb1();
    } 
    if (comb2MaxLength != comb2Length)
    { 
        comb2MaxLength = comb2Length;
        ReallocComb2();
    } 
    if (comb3MaxLength != comb3Length)
    { 
        comb3MaxLength = comb3Length;
        ReallocComb3();
    } 
    if (comb4MaxLength != comb4Length)
    { 
        comb4MaxLength = comb4Length;
        ReallocComb4();
    } 
    if (comb5MaxLength != comb5Length)
    { 
        comb5MaxLength = comb5Length;
        ReallocComb5();
    } 
    if (comb6MaxLength != comb6Length)
    { 
        comb6MaxLength = comb6Length;
        ReallocComb6();
    } 
    if (comb7MaxLength != comb7Length)
    { 
        comb7MaxLength = comb7Length;
        ReallocComb7();
    } 
    if (comb8MaxLength != comb8Length)
    { 
        comb8MaxLength = comb8Length;
        ReallocComb8();
    } 
    if (comb9MaxLength != comb9Length)
    { 
        comb9MaxLength = comb9Length;
        ReallocComb9();
    } 
    if (comb10MaxLength != comb10Length)
    { 
        comb10MaxLength = comb10Length;
        ReallocComb10();
    } 
    if (comb11MaxLength != comb11Length)
    { 
        comb11MaxLength = comb11Length;
        ReallocComb11();
    } 
    if (comb12MaxLength != comb12Length)
    { 
        comb12MaxLength = comb12Length;
        ReallocComb12();
    } 
    if (allpassL1Length != static_cast<unsigned long>( $_AL1_$ * getSampleRate() /1000 ) )
    { 
        allpassL1Length = static_cast<unsigned long>( $_AL1_$ * getSampleRate() / 1000 );
        ReallocAllpassL1();
    }
    if (allpassL2Length != static_cast<unsigned long>( ( $_AL2_$ + $_SW_$ ) * getSampleRate() /1000 ) )
    { 
        allpassL2Length = static_cast<unsigned long>( ( $_AL2_$ + $_SW_$ ) * getSampleRate() / 1000 );
        ReallocAllpassL2();
    }
    if (allpassL3Length != static_cast<unsigned long>( $_AL3_$ * getSampleRate() /1000 ) )
    { 
        allpassL3Length = static_cast<unsigned long>( $_AL3_$ * getSampleRate() / 1000 );
        ReallocAllpassL3();
    }
    if (allpassR1Length != static_cast<unsigned long>( ( $_AR1_$ + $_SW_$ ) * getSampleRate() /1000 ) )
    { 
        allpassR1Length = static_cast<unsigned long>( ( $_AR1_$ + $_SW_$ ) * getSampleRate() / 1000 );
        ReallocAllpassR1();
    }
    if (allpassR2Length != static_cast<unsigned long>( $_AR2_$ * getSampleRate() /1000 ) )
    { 
        allpassR2Length = static_cast<unsigned long>( $_AR2_$ * getSampleRate() / 1000 );
        ReallocAllpassR2();
    }
    if (allpassR3Length != static_cast<unsigned long>( ( $_AR3_$ + $_SW_$ ) * getSampleRate() /1000 ) )
    { 
        allpassR3Length = static_cast<unsigned long>( ( $_AR3_$ + $_SW_$ ) * getSampleRate() / 1000 );
        ReallocAllpassR3();
    }
    FlushBuffers();

    allpassL1Pos = allpassL2Pos = allpassL3Pos = allpassR1Pos = allpassR2Pos = allpassR3Pos = 0;
    tmp1LP = tmp2LP = tmp1HP = tmp2HP = 0;

    comb1Pos = comb2Pos = comb3Pos = comb4Pos = comb5Pos = comb6Pos = comb7Pos = comb8Pos = 0;
    comb9Pos = comb10Pos = comb11Pos = comb12Pos = 0;
    preDelayPos = 0;
    
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
            roomSizeFader = value;
            roomSize = 5+roomSizeFader*roomSizeFader*95;
#define $_roomMaxSize_$  100

            comb1Length = static_cast<unsigned long>( $_C1_$ * roomSize * getSampleRate() / 1000 );
            if (comb1Length > comb1MaxLength)
            {
                comb1MaxLength = static_cast<unsigned long>( $_C1_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb1();
            } comb1Pos = 0;

            comb2Length = static_cast<unsigned long>( $_C2_$ * roomSize * getSampleRate() / 1000 );
            if (comb2Length > comb2MaxLength)
            {
                comb2MaxLength = static_cast<unsigned long>( $_C2_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb2();
            } comb2Pos = 0;
            
            comb3Length = static_cast<unsigned long>( $_C3_$ * roomSize * getSampleRate() / 1000 );
            if (comb3Length > comb3MaxLength)
            {
                comb3MaxLength = static_cast<unsigned long>( $_C3_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb3();
            } comb3Pos = 0;
            
            comb4Length = static_cast<unsigned long>( $_C4_$ * roomSize * getSampleRate() / 1000 );
            if (comb4Length > comb4MaxLength)
            {
                comb4MaxLength = static_cast<unsigned long>( $_C4_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb4();
            } comb4Pos = 0;
            
            comb5Length = static_cast<unsigned long>( $_C5_$ * roomSize * getSampleRate() / 1000 );
            if (comb5Length > comb5MaxLength)
            {
                comb5MaxLength = static_cast<unsigned long>( $_C5_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb5();
            } comb5Pos = 0;
            
            comb6Length = static_cast<unsigned long>( $_C6_$ * roomSize * getSampleRate() / 1000 );
            if (comb6Length > comb6MaxLength)
            {
                comb6MaxLength = static_cast<unsigned long>( $_C6_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb6();
            } comb6Pos = 0;
            
            comb7Length = static_cast<unsigned long>( $_C7_$ * roomSize * getSampleRate() / 1000 );
            if (comb7Length > comb7MaxLength)
            {
                comb7MaxLength = static_cast<unsigned long>( $_C7_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb7();
            } comb7Pos = 0;
            
            comb8Length = static_cast<unsigned long>( $_C8_$ * roomSize * getSampleRate() / 1000 );
            if (comb8Length > comb8MaxLength)
            {
                comb8MaxLength = static_cast<unsigned long>( $_C8_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb8();
            } comb8Pos = 0;
            
            comb9Length = static_cast<unsigned long>( $_C9_$ * roomSize * getSampleRate() / 1000 );
            if (comb9Length > comb9MaxLength)
            {
                comb9MaxLength = static_cast<unsigned long>( $_C9_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb9();
            } comb9Pos = 0;
            
            comb10Length = static_cast<unsigned long>( $_C10_$ * roomSize * getSampleRate() / 1000 );
            if (comb10Length > comb10MaxLength)
            {
                comb10MaxLength = static_cast<unsigned long>( $_C10_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb10();
            } comb10Pos = 0;
            
            comb11Length = static_cast<unsigned long>( $_C11_$ * roomSize * getSampleRate() / 1000 );
            if (comb11Length > comb11MaxLength)
            {
                comb11MaxLength = static_cast<unsigned long>( $_C11_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb11();
            } comb11Pos = 0;
            
            comb12Length = static_cast<unsigned long>( $_C12_$ * roomSize * getSampleRate() / 1000 );
            if (comb12Length > comb12MaxLength)
            {
                comb12MaxLength = static_cast<unsigned long>( $_C12_$ * $_roomMaxSize_$ * getSampleRate() / 1000 );
                ReallocComb12();
            } comb12Pos = 0;
            FlushBuffers();
        break;
		case  1 : 
            dampFader = value;
            damp = min(1-dampFader*dampFader,0.95);
        break;
		case  2 : 
            preDelayFader = value;
            preDelayLength = static_cast<unsigned long>( preDelayFader * preDelayFader * 250 * getSampleRate() / 1000 );
            if (preDelayLength > preDelayMaxLength)
            {
                preDelayMaxLength = static_cast<unsigned long>( 250 * getSampleRate() / 1000 );
                ReallocPreDelay();
            } preDelayPos = 0;
            FlushPreDelay();
        break;
        case  3 : 
        freqLPFader = value;
        freqLP = freqLPFader*freqLPFader*freqLPFader*24000;
        b1LP = -exp(-2.0*cPi_*freqLP/ getSampleRate() ); // 100Hz
        a0LP = 1.0 + b1LP;
        break;
		case  4 : 
        freqHPFader = value;
        freqHP = freqHPFader*freqHPFader*freqHPFader*24000;
        b1HP = -exp(-2.0*cPi_*freqHP/ getSampleRate() ); // 100Hz
        a0HP = 1.0 + b1HP;
        break;
		case  5 : 
            dry = value*2;
        break;
		case  6 : 
            wet = value*2;
        break;
        	
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
        case  0 : v = roomSizeFader;   break;
		case  1 : v = dampFader;      break;
		case  2 : v = preDelayFader;     break;
		case  3 : v = freqLPFader;     break;
		case  4 : v = freqHPFader;     break;
		case  5 : v = dry/2;           break;
		case  6 : v = wet/2;           break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case  0 : strcpy (label, "Room Size");   break;
		case  1 : strcpy (label, "Damping");     break;
		case  2 : strcpy (label, "Pre-Delay");    break;
		case  3 : strcpy (label, "Low-Pass");    break;
		case  4 : strcpy (label, "High-Pass");   break;
		case  5 : strcpy (label, "Dry");         break;
		case  6 : strcpy (label, "Wet");         break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case  0 : float2string (roomSizeFader*100, text);  break;
		case  1 : float2string (dampFader*100, text);       break;
		case  2 : ms2string (preDelayLength, text);             break;
		case  3 : float2string (freqLP, text);             break;
		case  4 : float2string (freqHP, text);             break;
		case  5 : dB2string (dry, text);        break;
		case  6 : dB2string (wet, text);        break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case  0 : strcpy (label, "%");	    break;
		case  1 : strcpy (label, "%");	    break;
		case  2 : strcpy (label, "ms");	    break;
		case  3 : strcpy (label, "Hz");	    break;
		case  4 : strcpy (label, "Hz");	    break;
		case  5 : strcpy (label, "dB");	    break;
		case  6 : strcpy (label, "dB");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Simple Verb");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Simple Verb");
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

    if (preDelayLength <= 1) reverb = ( (*in1) + (*in2) ) / (1+damp) + cDC_;
    else 
    {
        preDelay[preDelayPos] = ( (*in1) + (*in2) ) / (1+damp) + cDC_;
        if( ++preDelayPos >= preDelayLength ) preDelayPos = 0;
        reverb = preDelay[preDelayPos];
    }
    
    comb1[comb1Pos]   = reverb*0.49 +   comb1[comb1Pos] * damp;
    comb2[comb2Pos]   = reverb*0.76 +   comb2[comb2Pos] * damp;
    comb3[comb3Pos]   = reverb*1.00 +   comb3[comb3Pos] * damp;
    comb4[comb4Pos]   = reverb*0.80 +   comb4[comb4Pos] * damp;
    comb5[comb5Pos]   = reverb*0.91 +   comb5[comb5Pos] * damp;
    comb6[comb6Pos]   = reverb*0.72 +   comb6[comb6Pos] * damp;
    comb7[comb7Pos]   = reverb*0.61 +   comb7[comb7Pos] * damp;
    comb8[comb8Pos]   = reverb*0.51 +   comb8[comb8Pos] * damp;
    comb9[comb9Pos]   = reverb*0.49 +   comb9[comb9Pos] * damp;
    comb10[comb10Pos] = reverb*0.42 + comb10[comb10Pos] * damp;
    comb11[comb11Pos] = reverb*0.30 + comb11[comb11Pos] * damp;
    comb12[comb12Pos] = reverb*0.23 + comb12[comb12Pos] * damp;
    if( ++comb1Pos >= comb1Length ) comb1Pos = 0;
    if( ++comb2Pos >= comb2Length ) comb2Pos = 0;
    if( ++comb3Pos >= comb3Length ) comb3Pos = 0;
    if( ++comb4Pos >= comb4Length ) comb4Pos = 0;
    if( ++comb5Pos >= comb5Length ) comb5Pos = 0;
    if( ++comb6Pos >= comb6Length ) comb6Pos = 0;
    if( ++comb7Pos >= comb7Length ) comb7Pos = 0;
    if( ++comb8Pos >= comb8Length ) comb8Pos = 0;
    if( ++comb9Pos >= comb9Length ) comb9Pos = 0;
    if( ++comb10Pos >= comb10Length ) comb10Pos = 0;
    if( ++comb11Pos >= comb11Length ) comb11Pos = 0;
    if( ++comb12Pos >= comb12Length ) comb12Pos = 0;

    reverb = (   comb1[comb1Pos]
                +comb2[comb2Pos]
                +comb3[comb3Pos]
                +comb4[comb4Pos]
                +comb5[comb5Pos]
                +comb6[comb6Pos]
                +comb7[comb7Pos]
                +comb8[comb8Pos]
                +comb9[comb9Pos]
                +comb10[comb10Pos]
                +comb11[comb11Pos]
                +comb12[comb12Pos]
            );
 
    allpassL1[allpassL1Pos] = reverb + allpassL1[allpassL1Pos] * $_AP1FBQ_$;
    left = (reverb - allpassL1[allpassL1Pos] * $_AP1FBQ_$);
    if( ++allpassL1Pos >= allpassL1Length ) allpassL1Pos = 0;
    allpassL2[allpassL2Pos] = left + allpassL2[allpassL2Pos] * $_AP2FBQ_$;
    left = (left - allpassL2[allpassL2Pos] * $_AP2FBQ_$);
    if( ++allpassL2Pos >= allpassL2Length ) allpassL2Pos = 0;
    allpassL3[allpassL3Pos] = left + allpassL3[allpassL3Pos] * $_AP3FBQ_$;
    left = (left - allpassL3[allpassL3Pos] * $_AP3FBQ_$);
    if( ++allpassL3Pos >= allpassL3Length ) allpassL3Pos = 0;

    allpassR1[allpassR1Pos] = reverb + allpassR1[allpassR1Pos] * $_AP1FBQ_$;
    right = (reverb - allpassR1[allpassR1Pos] * $_AP1FBQ_$);
    if( ++allpassR1Pos >= allpassR1Length ) allpassR1Pos = 0;
    allpassR2[allpassR2Pos] = right + allpassR2[allpassR2Pos] * $_AP2FBQ_$;
    right = (right - allpassR2[allpassR2Pos] * $_AP2FBQ_$);
    if( ++allpassR2Pos >= allpassR2Length ) allpassR2Pos = 0;
    allpassR3[allpassR3Pos] = right + allpassR3[allpassR3Pos] * $_AP3FBQ_$;
    right = (right - allpassR3[allpassR3Pos] * $_AP3FBQ_$);
    if( ++allpassR3Pos >= allpassR3Length ) allpassR3Pos = 0;

    if (freqHPFader != 0)
    {
        left -= (tmp1HP = a0HP * left - b1HP * tmp1HP + cDC_) - cDC_;
        right -= (tmp2HP = a0HP * right - b1HP * tmp2HP + cDC_) - cDC_;  
    }
    if (freqLPFader != 1)
    {
        left = (tmp1LP = a0LP * left - b1LP * tmp1LP + cDC_) - cDC_;
        right = (tmp2LP = a0LP * right - b1LP * tmp2LP + cDC_) - cDC_;
    }

    (*out1++) += (*in1++) * dry + left * wet;
    (*out2++) += (*in2++) * dry + right * wet; 
        

    } // eof while
       
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

    if (preDelayLength <= 1) reverb = ( (*in1) + (*in2) ) / (1+damp) + cDC_;
    else 
    {
        preDelay[preDelayPos] = ( (*in1) + (*in2) ) / (1+damp) + cDC_;
        if( ++preDelayPos >= preDelayLength ) preDelayPos = 0;
        reverb = preDelay[preDelayPos];
    }
    
    comb1[comb1Pos]   = reverb*0.49 +   comb1[comb1Pos] * damp;
    comb2[comb2Pos]   = reverb*0.76 +   comb2[comb2Pos] * damp;
    comb3[comb3Pos]   = reverb*1.00 +   comb3[comb3Pos] * damp;
    comb4[comb4Pos]   = reverb*0.91 +   comb4[comb4Pos] * damp;
    comb5[comb5Pos]   = reverb*0.79 +   comb5[comb5Pos] * damp;
    comb6[comb6Pos]   = reverb*0.71 +   comb6[comb6Pos] * damp;
    comb7[comb7Pos]   = reverb*0.59 +   comb7[comb7Pos] * damp;
    comb8[comb8Pos]   = reverb*0.51 +   comb8[comb8Pos] * damp;
    comb9[comb9Pos]   = reverb*0.42 +   comb9[comb9Pos] * damp;
    comb10[comb10Pos] = reverb*0.38 + comb10[comb10Pos] * damp;
    comb11[comb11Pos] = reverb*0.35 + comb11[comb11Pos] * damp;
    comb12[comb12Pos] = reverb*0.30 + comb12[comb12Pos] * damp;
    if( ++comb1Pos >= comb1Length ) comb1Pos = 0;
    if( ++comb2Pos >= comb2Length ) comb2Pos = 0;
    if( ++comb3Pos >= comb3Length ) comb3Pos = 0;
    if( ++comb4Pos >= comb4Length ) comb4Pos = 0;
    if( ++comb5Pos >= comb5Length ) comb5Pos = 0;
    if( ++comb6Pos >= comb6Length ) comb6Pos = 0;
    if( ++comb7Pos >= comb7Length ) comb7Pos = 0;
    if( ++comb8Pos >= comb8Length ) comb8Pos = 0;
    if( ++comb9Pos >= comb9Length ) comb9Pos = 0;
    if( ++comb10Pos >= comb10Length ) comb10Pos = 0;
    if( ++comb11Pos >= comb11Length ) comb11Pos = 0;
    if( ++comb12Pos >= comb12Length ) comb12Pos = 0;

    reverb = (   comb1[comb1Pos]
                +comb2[comb2Pos]
                +comb3[comb3Pos]
                +comb4[comb4Pos]
                +comb5[comb5Pos]
                +comb6[comb6Pos]
                +comb7[comb7Pos]
                +comb8[comb8Pos]
                +comb9[comb9Pos]
                +comb10[comb10Pos]
                +comb11[comb11Pos]
                +comb12[comb12Pos]
            );
 
    allpassL1[allpassL1Pos] = reverb + allpassL1[allpassL1Pos] * $_AP1FBQ_$;
    left = (reverb - allpassL1[allpassL1Pos] * $_AP1FBQ_$);
    if( ++allpassL1Pos >= allpassL1Length ) allpassL1Pos = 0;
    allpassL2[allpassL2Pos] = left + allpassL2[allpassL2Pos] * $_AP2FBQ_$;
    left = (left - allpassL2[allpassL2Pos] * $_AP2FBQ_$);
    if( ++allpassL2Pos >= allpassL2Length ) allpassL2Pos = 0;
    allpassL3[allpassL3Pos] = left + allpassL3[allpassL3Pos] * $_AP3FBQ_$;
    left = (left - allpassL3[allpassL3Pos] * $_AP3FBQ_$);
    if( ++allpassL3Pos >= allpassL3Length ) allpassL3Pos = 0;

    allpassR1[allpassR1Pos] = reverb + allpassR1[allpassR1Pos] * $_AP1FBQ_$;
    right = (reverb - allpassR1[allpassR1Pos] * $_AP1FBQ_$);
    if( ++allpassR1Pos >= allpassR1Length ) allpassR1Pos = 0;
    allpassR2[allpassR2Pos] = right + allpassR2[allpassR2Pos] * $_AP2FBQ_$;
    right = (right - allpassR2[allpassR2Pos] * $_AP2FBQ_$);
    if( ++allpassR2Pos >= allpassR2Length ) allpassR2Pos = 0;
    allpassR3[allpassR3Pos] = right + allpassR3[allpassR3Pos] * $_AP3FBQ_$;
    right = (right - allpassR3[allpassR3Pos] * $_AP3FBQ_$);
    if( ++allpassR3Pos >= allpassR3Length ) allpassR3Pos = 0;

    if (freqHPFader != 0)
    {
        left -= (tmp1HP = a0HP * left - b1HP * tmp1HP + cDC_) - cDC_;
        right -= (tmp2HP = a0HP * right - b1HP * tmp2HP + cDC_) - cDC_;  
    }
    if (freqLPFader != 1)
    {
        left = (tmp1LP = a0LP * left - b1LP * tmp1LP + cDC_) - cDC_;
        right = (tmp2LP = a0LP * right - b1LP * tmp2LP + cDC_) - cDC_;
    }

    (*out1++) = (*in1++) * dry + left * wet;
    (*out2++) = (*in2++) * dry + right * wet; 
    
  } // eof while

}
