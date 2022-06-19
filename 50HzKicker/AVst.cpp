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
	: AudioEffectX (audioMaster, 1, 5)	// 1 program, 15 parameters
{
   
    b1EnvAttLP = -exp(-2*cPi_*10 / getSampleRate() ); // 
    a0EnvAttLP = 1.0 + b1EnvAttLP;
    env = 1;
              
    freqDBPFader = 0.27272727272727;
    
    w0DBP = 2 * cPi_ * (freqDBPFader*220+20)/getSampleRate();
        cosw0DBP = cos(w0DBP);
        sinw0DBP = sin(w0DBP);
        alphaDBP = sinw0DBP / 0.5; // cSqrt2_;
    
        b0DBP =   alphaDBP         ;
        b1DBP =   0               ;
        b2DBP =  -alphaDBP         ;
        a0DBP =   1 + alphaDBP     ;
        a1DBP =  -2*cos(w0DBP)     ;
        a2DBP =   1 - alphaDBP     ;

        b0DBP /= a0DBP;
        b1DBP /= a0DBP;
        b2DBP /= a0DBP;
        a1DBP /= a0DBP;
    a2DBP /= a0DBP;  
    
    
    freqFader = 0.13636363636;
    adj = 2*cPi_*  (freqFader*220+20) / getSampleRate();    

    lengthFader = 0.5;
    attEnv = dB2Amp( -50 / ( 0.250 * getSampleRate() ) );    
    relEnv = dB2Amp( -50 / ( 0.125 * getSampleRate() ) );

    w0OBP = 2 * cPi_ * (freqFader*220+20)/getSampleRate();
        cosw0OBP = cos(w0OBP);
        sinw0OBP = sin(w0OBP);
        alphaOBP = sinw0OBP / 0.5; // cSqrt2_;
    
        b0OBP =   alphaOBP         ;
        b1OBP =   0               ;
        b2OBP =  -alphaOBP         ;
        a0OBP =   1 + alphaOBP     ;
        a1OBP =  -2*cos(w0OBP)     ;
        a2OBP =   1 - alphaOBP     ;

        b0OBP /= a0OBP;
        b1OBP /= a0OBP;
        b2OBP /= a0OBP;
        a1OBP /= a0OBP;
    a2OBP /= a0OBP;  
         
    wet = 0.25; // = -12 dB
    wetFader = 0.25;
    dry = 1; // = 0 dB
    dryFader = 0.5;

    gain = pos = 0;
    
	setNumInputs (4);		// stereo in + stereo sidechain (aux)
	setNumOutputs (2);
	setUniqueID (CCONST ('5','0','H','Z') );
//    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

	suspend(); // flush buffers and reset variables
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{

}

void AVst::suspend()
{

    b1EnvAttLP = -exp(-2*cPi_*10 / getSampleRate() ); // 
    a0EnvAttLP = 1.0 + b1EnvAttLP;
    tmpEnvAttLP = 0;
    
    gain = pos = x1DBP = x2DBP = y1DBP = y2DBP = 0;
    x1OBP = x2OBP = y1OBP = y2OBP = 0;
  
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
            freqDBPFader = value;
            w0DBP = 2 * cPi_ * (freqDBPFader*220+20)/getSampleRate();
            cosw0DBP = cos(w0DBP);
            sinw0DBP = sin(w0DBP);
            alphaDBP = sinw0DBP / 0.5; // cSqrt2_;
    
            b0DBP =   alphaDBP         ;
            b1DBP =   0               ;
            b2DBP =  -alphaDBP         ;
            a0DBP =   1 + alphaDBP     ;
            a1DBP =  -2*cos(w0DBP)     ;
            a2DBP =   1 - alphaDBP     ;

            b0DBP /= a0DBP;
            b1DBP /= a0DBP;
            b2DBP /= a0DBP;
            a1DBP /= a0DBP;
            a2DBP /= a0DBP;       
        break;
        
		case  1 : 
            freqFader = value;
            adj = 2*cPi_*  (freqFader*220+20) / getSampleRate();

            w0OBP = 2 * cPi_ * (freqFader*220+20)/getSampleRate();
            cosw0OBP = cos(w0OBP);
            sinw0OBP = sin(w0OBP);
            alphaOBP = sinw0OBP / 0.5; // cSqrt2_;
    
            b0OBP =   alphaOBP         ;
            b1OBP =   0               ;
            b2OBP =  -alphaOBP         ;
            a0OBP =   1 + alphaOBP     ;
            a1OBP =  -2*cos(w0OBP)     ;
            a2OBP =   1 - alphaOBP     ;

            b0OBP /= a0OBP;
            b1OBP /= a0OBP;
            b2OBP /= a0OBP;
            a1OBP /= a0OBP;
            a2OBP /= a0OBP;    
        break;

        case 2 :
            lengthFader = value;
            attEnv = dB2Amp( -50 / ( (0.050+0.450*lengthFader) * getSampleRate() ) );    
            relEnv = dB2Amp( -50 / ( (0.025+0.175*lengthFader) * getSampleRate() ) );
        break;

        case 3 :
            wetFader = value;
            wet = sqr(wetFader) * 4;
        break;
               
        case 4 :
            dryFader = value;
            dry = sqr(dryFader) * 4;
        break;

	}

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case 0 : v = freqDBPFader;      break;
		case 1 : v = freqFader;      break;
		case 2 : v = lengthFader;      break;
		case 3 : v = wetFader;     break;
		case 4 : v = dryFader;     break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case 0 : strcpy (label, "Detector");     break;
		case 1 : strcpy (label, "Frequency");     break;
		case 2 : strcpy (label, "Length");     break;
		case 3 : strcpy (label, "Wet");       break;
		case 4 : strcpy (label, "Dry");       break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case 0 :
            if (freqDBPFader) float2string ((freqDBPFader*220+20), text);
            else strcpy (text, "SIDE");
        break;
		case 1 : float2string (freqFader*220+20, text);           break;
		case 2 : float2string (25+175*lengthFader, text);           break;
		case 3 : dB2string (wet, text);       break;
		case 4 : dB2string (dry, text);       break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case 0 : strcpy (label, "Hz");	    break;
		case 1 : strcpy (label, "Hz");	    break;
		case 2 : strcpy (label, "ms");	    break;
		case 3 : strcpy (label, "dB");	    break;
		case 4 : strcpy (label, "dB");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "50 Hz Kicker");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "50 Hz Kicker");
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
  float *in3  =  inputs[2];
  float *in4  =  inputs[3];
  float *out1 = outputs[0];
  float *out2 = outputs[1];

  while (--sampleFrames >= 0)
  {

// bandpass
    if (freqDBPFader)
    {
        det = (*in1)+(*in2);
        tmp = det;
        det = b0DBP * det + b1DBP * x1DBP + b2DBP * x2DBP - a1DBP * y1DBP - a2DBP * y2DBP;
        x2DBP = x1DBP; x1DBP = tmp; y2DBP = y1DBP; y1DBP = det + cDC_ ;
    } else det = (*in3)+(*in4);

    envAtt = max( sqrt( tmpEnvAttLP = a0EnvAttLP*abs(det) - b1EnvAttLP*tmpEnvAttLP + cDC_ ) ,0.03) ;

    if (env < envAtt) env = min( env/attEnv , envAtt );
    else env = max( env*relEnv , envAtt );
   
    gain = min(max(envAtt/env,1)-1,1);
    
    if (gain == 0) pos=0;

    det = sin(pos) * gain * wet;

// bandpass
    tmp = det;
    det = b0OBP * det + b1OBP * x1OBP + b2OBP * x2OBP - a1OBP * y1OBP - a2OBP * y2OBP;
    x2OBP = x1OBP; x1OBP = tmp; y2OBP = y1OBP; y1OBP = det + cDC_ ;
                  
    (*out1++) += det + (*in1++) * dry;
    (*out2++) += det + (*in2++) * dry;

    pos += adj;
       
  } // eof while
       
}

//-----------------------------------------------------------------------------------------
void AVst::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
  float *in1  =  inputs[0];
  float *in2  =  inputs[1];
  float *in3  =  inputs[2];
  float *in4  =  inputs[3];
  float *out1 = outputs[0];
  float *out2 = outputs[1];

  while (--sampleFrames >= 0)
  {

// bandpass
    if (freqDBPFader)
    {
        det = (*in1)+(*in2);
        tmp = det;
        det = b0DBP * det + b1DBP * x1DBP + b2DBP * x2DBP - a1DBP * y1DBP - a2DBP * y2DBP;
        x2DBP = x1DBP; x1DBP = tmp; y2DBP = y1DBP; y1DBP = det + cDC_ ;
    } else det = (*in3)+(*in4);

    envAtt = max( sqrt( tmpEnvAttLP = a0EnvAttLP*abs(det) - b1EnvAttLP*tmpEnvAttLP + cDC_ ) ,0.03) ;

    if (env < envAtt) env = min( env/attEnv , envAtt );
    else env = max( env*relEnv , envAtt );
   
    gain = min(max(envAtt/env,1)-1,1);
    
    if (gain == 0) pos=0;

    det = sin(pos) * gain * wet;

// bandpass
    tmp = det;
    det = b0OBP * det + b1OBP * x1OBP + b2OBP * x2OBP - a1OBP * y1OBP - a2OBP * y2OBP;
    x2OBP = x1OBP; x1OBP = tmp; y2OBP = y1OBP; y1OBP = det + cDC_ ;
                  
    (*out1++) = det + (*in1++) * dry;
    (*out2++) = det + (*in2++) * dry;

    pos += adj;
    
  } // eof while

}
