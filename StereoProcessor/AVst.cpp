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
	: AudioEffectX (audioMaster, 1, 8)	// 1 program, 5 parameter biatch
{

    rotation = 0;
    rotationFader = 0.5;
    
    width1 = 0.5;
    widthCoeff1 = 1;

    center1 = side1 = 1;
    centerFader1 = 0.5;
    
    width2 = 0.5;
    widthCoeff2 = 1;

    center2 = side2 = 1;
    centerFader2 = 0.5;

    pan1 = pan2 = 0;
    pan1Left = pan1Right = pan2Left = pan2Right = 1;
    
    outVolFader = .5;
    outVol = 1;

///

	setNumInputs (2);		// stereo in
	setNumOutputs (2);		// stereo out
	setUniqueID (CCONST ('0','1','0','8') );	// identify
	canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

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
        width1 = value;
        widthCoeff1 = max(width1,1);
        break;

		case 1 :
        centerFader1 = value;
        center1 = min(centerFader1*2,1);
        side1 = 2-centerFader1*2;
        break;
        
        case 2 :
            pan1 = (value-0.5)*2;
            pan1Left = (pan1 > 0 ? 1-pan1:1);
            pan1Right = (pan1 < 0 ? 1+pan1:1);
        break;
        
		case 3 : 
        rotationFader = value;
        rotation = (rotationFader-0.5)*180*0.017453292;
        break;

        case 4 :
            pan2 = (value-0.5)*2;
            pan2Left = (pan2 > 0 ? 1-pan2:1);
            pan2Right = (pan2 < 0 ? 1+pan2:1);
        break;

		case 5 :
        centerFader2 = value;
        center2 = min(centerFader2*2,1);
        side2 = 2-centerFader2*2;
        break;

		case 6 : 
        width2 = value;
        widthCoeff2 = max(width2,1);
        break;
        
        case 7 :
        outVolFader = value;
        outVol = dB2Amp( sign(outVolFader -.5)* sqr((outVolFader -.5)*2)*18 );
        break;
	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = width1;       break;
		case 1 : v = centerFader1;       break;
		case 2 : v = pan1/2+0.5;       break;
		case 3 : v = rotationFader;       break;
		case 4 : v = pan2/2+0.5;       break;
		case 5 : v = centerFader2;       break;
		case 6 : v = width2;       break;
		case 7 : v = outVolFader;   break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case 0 : strcpy (label, "Width");       break;
		case 1 : strcpy (label, "Center");       break;
		case 2 : strcpy (label, "Pan");       break;
		case 3 : strcpy (label, "Rotation");       break;
		case 4 : strcpy (label, "Pan");       break;
		case 5 : strcpy (label, "Center");       break;
		case 6 : strcpy (label, "Width");       break;
		case 7 : strcpy (label, "Output"); break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 : float2string (width1*200, text);      break;
		case 1 : float2string ((centerFader1-0.5)*200, text);      break;
		case 2 : float2string (pan1*100, text);      break;
		case 3 : float2string ((rotationFader-0.5)*180, text);      break;
		case 4 : float2string (pan2*100, text);      break;
		case 5 : float2string ((centerFader2-0.5)*200, text);      break;
		case 6 : float2string (width2*200, text);      break;
		case 7 : dB2string (outVol, text);      break;        
	}

}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "%");	break;
		case 1 : strcpy (label, "%");	break;
		case 2 : strcpy (label, "%");	break;
		case 3 : strcpy (label, "Deg");	break;
		case 4 : strcpy (label, "%");	break;
		case 5 : strcpy (label, "%");	break;
		case 6 : strcpy (label, "%");	break;
		case 7 : strcpy (label, "dB");	break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Stereo Processor");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Stereo Processor");
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

// Width 1  
    mono   = ( ((*in1) + (*in2)) / 2.0f ) * center1;
    stereo = (  (*in1) - (*in2)         ) * side1;       
    (*in1) = (mono + stereo * width1) / widthCoeff1;
    (*in2) = (mono - stereo * width1) / widthCoeff1;

// Pan 1
    (*in1) *= pan1Left;
    (*in2) *= pan1Right;
    
// Rotate
    if (rotation)
    {
        s1 = sign((*in1));
        s2 = sign((*in2));
        angle = atan( (*in1) / (*in2) );
        if ( ( s1 == 1 && s2 == -1) || (s1 == -1 && s2 == -1) ) angle += 3.141592654;
        if ( ( s1 == -1 && s2 == 1)                           ) angle += 6.283185307;
        if ( (*in2) == 0 ) 
        {
            if ((*in1) > 0) angle = 1.570796327;
            else angle = 4.71238898;
        }
        if ( (*in1) == 0 )
        {
            if ((*in2) > 0) angle = 0;
            else angle = 3.141592654;
        }
        radius = sqrt( sqr((*in1))+sqr((*in2)) ) ;    
        angle -= rotation;    
        (*in1) = sin(angle)*radius;
        (*in2) = cos(angle)*radius;
    }    

// Pan 2
    (*in1) *= pan2Left;
    (*in2) *= pan2Right;

// Width 2       
    mono   = ( ((*in1) + (*in2)) / 2.0f ) * center2;
    stereo = (  (*in1) - (*in2)         ) * side2;       
    (*in1) = (mono + stereo * width2) / widthCoeff2;
    (*in2) = (mono - stereo * width2) / widthCoeff2;
  
    (*out1++) += (*in1++) * outVol;
    (*out2++) += (*in2++) * outVol;

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

// Width 1  
    mono   = ( ((*in1) + (*in2)) / 2.0f ) * center1;
    stereo = (  (*in1) - (*in2)         ) * side1;       
    (*in1) = (mono + stereo * width1) / widthCoeff1;
    (*in2) = (mono - stereo * width1) / widthCoeff1;

// Pan 1
    (*in1) *= pan1Left;
    (*in2) *= pan1Right;
    
// Rotate
    if (rotation)
    {
        s1 = sign((*in1));
        s2 = sign((*in2));
        angle = atan( (*in1) / (*in2) );
        if ( ( s1 == 1 && s2 == -1) || (s1 == -1 && s2 == -1) ) angle += 3.141592654;
        if ( ( s1 == -1 && s2 == 1)                           ) angle += 6.283185307;
        if ( (*in2) == 0 ) 
        {
            if ((*in1) > 0) angle = 1.570796327;
            else angle = 4.71238898;
        }
        if ( (*in1) == 0 )
        {
            if ((*in2) > 0) angle = 0;
            else angle = 3.141592654;
        }
        radius = sqrt( sqr((*in1))+sqr((*in2)) ) ;    
        angle -= rotation;    
        (*in1) = sin(angle)*radius;
        (*in2) = cos(angle)*radius;
    }    

// Pan 2
    (*in1) *= pan2Left;
    (*in2) *= pan2Right;

// Width 2       
    mono   = ( ((*in1) + (*in2)) / 2.0f ) * center2;
    stereo = (  (*in1) - (*in2)         ) * side2;       
    (*in1) = (mono + stereo * width2) / widthCoeff2;
    (*in2) = (mono - stereo * width2) / widthCoeff2;

    (*out1++) = (*in1++) * outVol;
    (*out2++) = (*in2++) * outVol;

    }
}
