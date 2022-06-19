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
    return min(max(in,-limit),limit);   
}

// eof Mich's formulas


#ifndef __AVST_H
#include "AVst.hpp"
#endif

//-------------------------------------------------------------------------------------------------------
AVst::AVst (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, 22) // 1 program, 2 parameter only
{

    threshFader = 1;
    threshDB = -.1;
    thresh = threshLow = threshHigh = 0.98851402;
    
    ratioFader = 0;
    ratio = 1;
    
    kneeFader = 0;
    kneeDB = 0;

    attackStart = attackStartSpls = 0; // 0 ms no lookAhead, no delayed attack
    lookAheadSize = lookAheadMaxSize = 0; // = lookAhead init length [samples]
	lookAheadBufferRms = new float[lookAheadMaxSize];
	lookAheadBufferIn1 = new float[lookAheadMaxSize];
	lookAheadBufferIn2 = new float[lookAheadMaxSize];

    attackFader = 0.376060309; // ~20 ms
    attackSpls = 0.02 * getSampleRate(); // =20 ms
    attack = dB2Amp( threshDB / max( attackSpls , 0) );
    
    releaseFader = 0.430886938; // ~200ms
    releaseSpls = 0.2 * getSampleRate(); // =200 ms
    release = dB2Amp( threshDB / max( releaseSpls , 0) );

    attackShape = releaseShape = 0;

    envDet = 1;

    freqEnvDetLPFader = 0.5;
    b1EnvLP = -exp(-2.0*cPi_*7.952707288 / getSampleRate() ); // 10Hz
    a0EnvLP = 1.0 + b1EnvLP;
    
    rmsFader = 0; // ~PEAK
    rmsSize = rmsMaxSize = 1; // = RMS init length [samples]
	rmsBuffer = new float[rmsMaxSize];

    freqLPFader = 1;
    freqHPFader = 0;

    freqLP = (getSampleRate()/2)-10;
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

    freqP1Fader = 0.470236784;
    freqP1 = 2500;
    dBgainP1 = 0;
    BWP1 = 1;
    dBgainP1Fader = 0.5;
    setP1();
    
    listen = 0;
      
    feed = 0; // =FF
    modeMakeUp = 0; // =ON
  
    outVolumeFader = .125; // ~ 0 dB
    outVolume = 1; // = 0 dB

    dryMixFader = 0;
    dryMix = 0;

    volume = 1; // = 0 dB
    
    lookAheadMaxTime = 0;
    
    saturation = 0;
    satC = 1;
  
	setNumInputs (4);		// stereo in + stereo sidechain (aux)
	setNumOutputs (2);
	setUniqueID (CCONST ('D','V','C','M') );
//    canMono ();
    canProcessReplacing ();	// supports both accumulating and replacing output
	strcpy (programName, "Default");	// default program name

	suspend(); // flush buffers and reset variables
}

//-------------------------------------------------------------------------------------------------------
AVst::~AVst ()
{
	if (rmsBuffer)
        delete[] rmsBuffer;
	if (lookAheadBufferRms)
        delete[] lookAheadBufferRms;
	if (lookAheadBufferIn1)
        delete[] lookAheadBufferIn1;
	if (lookAheadBufferIn2)
        delete[] lookAheadBufferIn2;
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

void AVst::FlushRmsBuffer ()
{
    memset (rmsBuffer, 0, rmsSize * sizeof(float));
}

void AVst::FlushLookAheadBuffers ()
{
    memset (lookAheadBufferRms, 0, lookAheadSize * sizeof(float));
    memset (lookAheadBufferIn1, 0, lookAheadSize * sizeof(float));
    memset (lookAheadBufferIn2, 0, lookAheadSize * sizeof(float));
}

void AVst::ReallocRmsBuffer ()
{
    rmsBuffer = (float*) realloc(rmsBuffer, rmsMaxSize * sizeof(float));
}

void AVst::ReallocLookAheadBuffers ()
{
    lookAheadBufferRms = (float*) realloc(lookAheadBufferRms, lookAheadMaxSize * sizeof(float));
    lookAheadBufferIn1 = (float*) realloc(lookAheadBufferIn1, lookAheadMaxSize * sizeof(float));
    lookAheadBufferIn2 = (float*) realloc(lookAheadBufferIn2, lookAheadMaxSize * sizeof(float));
}

void AVst::suspend()
{
    gain = seekGain = 1;
    underThresh = rmsSqrSum = rmsBufPos = 0;
    lookAheadMaxTime = 0;
    if (rmsMaxSize != rmsSize)
    { 
        rmsMaxSize = rmsSize;
        ReallocRmsBuffer();
    }
    FlushRmsBuffer();

    lookAheadBufPos = 0;
    if (lookAheadMaxSize != lookAheadSize)
    { 
        lookAheadMaxSize = lookAheadSize;
        ReallocLookAheadBuffers();
    }
    FlushLookAheadBuffers();
    
    xl1LP = xl2LP = yl1LP = yl2LP =
    xr1LP = xr2LP = yr1LP = yr2LP =
    xl1HP = xl2HP = yl1HP = yl2HP =
    xr1HP = xr2HP = yr1HP = yr2HP =
    xl1P1 = xl2P1 = yl1P1 = yl2P1 =
    xr1P1 = xr2P1 = yr1P1 = yr2P1 = 0;
    tmpEnvLP = 0;
    
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
            threshFader = value;
            threshDB = min( -90+ sqrt(sqrt(threshFader)) * 90 , -.1 );
            thresh = dB2Amp ( threshDB );
    // SOFTKNEE STUFF
            threshLowDB = threshDB - kneeDB;
            threshHighDB = threshDB + kneeDB;
            kneeSlopeDB = (threshDB + (threshHighDB - threshDB) * ratio - threshLowDB) / (threshHighDB-threshLowDB);
            threshLow = dB2Amp( threshLowDB );
            threshHigh = dB2Amp( threshHighDB );

            if (attackShape  == 0) attack = dB2Amp( threshDB / attackSpls );
            if (attackShape  == 1) attack = (1-thresh) / attackSpls;
            if (releaseShape == 0) release = dB2Amp( threshDB / releaseSpls );        
            if (releaseShape == 1) release = (1-thresh) / releaseSpls;        

    // Avoid long fades if thresh to low and thus attack/release too slow...
            if ( threshDB >= -.5 )
                gain = seekGain = 1; 
        break;
        	
        case  1 : 
            ratioFader = value;
            ratio = 1 - sqrt(ratioFader);
        break;

        case  2 : 
            kneeFader = value;
            kneeDB = kneeFader * 12; // * 'max softknee db' / 2 !!!
    // SOFTKNEE STUFF
            threshLowDB = threshDB - kneeDB;
            threshHighDB = threshDB + kneeDB;
            kneeSlopeDB = (threshDB + (threshHighDB - threshDB) * ratio - threshLowDB) / (threshHighDB-threshLowDB);
            threshLow = dB2Amp( threshLowDB );
            threshHigh = dB2Amp( threshHighDB );
        break;

		case  3 :
            attackStart = value-0.5;
// Clean Up Here!!!!!
            attackStartSpls = attackStart*attackStart*attackStart * 0.8 * getSampleRate();
            lookAheadSize = static_cast<unsigned long>(max ((-1)* attackStartSpls ,0) );
            if (lookAheadSize > lookAheadMaxSize)
            {
                lookAheadMaxSize = static_cast<unsigned long>( 0.8 * getSampleRate() ); // set 'lookAheadMaxSize' to max possible size !!!
                ReallocLookAheadBuffers();
            }
            FlushLookAheadBuffers();
            lookAheadBufPos = 0;
            setInitialDelay(lookAheadSize);
        break;

		case  4 :
            attackFader = value;
            attackSpls = (attackFader*attackFader*attackFader*attackFader) * getSampleRate();
            if (attackShape  == 0) attack = dB2Amp( threshDB / attackSpls );
            if (attackShape  == 1) attack = (1-thresh) / attackSpls;
        break;

        case 5:
            attackShape = floor(value);
        break;

		case  6 :
            releaseFader = value;
            releaseSpls = (releaseFader*releaseFader*releaseFader) * 2.5 * getSampleRate();
            if (releaseShape == 0) release = dB2Amp( threshDB / releaseSpls );        
            if (releaseShape == 1) release = (1-thresh) / releaseSpls;        
        break;

        case 7:
            releaseShape = floor(value);
        break;
       
        case  8 : 
            envDet = floor(value*2);
            b1EnvLP = -exp(-2.0*cPi_* (  pow(4000,sqr(1-freqEnvDetLPFader))   ) / getSampleRate() );
            a0EnvLP = 1.0 + b1EnvLP;
        break;
              
        case  9 :
            freqEnvDetLPFader = value;
            b1EnvLP = -exp(-2.0*cPi_* (  pow(4000,sqr(1-freqEnvDetLPFader))   ) / getSampleRate() ); // 10Hz
            a0EnvLP = 1.0 + b1EnvLP;    
        break;
               
        case  10 :
            rmsFader = value;
            rmsSize = static_cast<int>( rmsFader*rmsFader*rmsFader * getSampleRate() );
            if (rmsSize > rmsMaxSize)
            {
                rmsMaxSize = static_cast<unsigned long>( getSampleRate() ); // set 'rmsMaxSize' to max possible size !!!
                ReallocRmsBuffer();
            }
            FlushRmsBuffer();
            rmsSqrSum = rmsBufPos = 0;
        break;

        case  11 : 
        freqLPFader = value;
        freqLP = min(5+freqLPFader*freqLPFader*freqLPFader*23995,(getSampleRate()/2)-10);
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
        break;
                
        case 12 :
            freqP1Fader = value;
            freqP1 = min(5+freqP1Fader*freqP1Fader*freqP1Fader*23995,(getSampleRate()/2.205));
            setP1();    
        break;
        
        case 13 :
            BWP1 = 0.05 + sqr(value) * 4.95; setP1();
        break;
        
        case 14 :
            dBgainP1Fader = value;
            dBgainP1 = sign(dBgainP1Fader -.5)* sqr((dBgainP1Fader -.5)*2)*12;
            setP1();
        break;
              
        case 15 :
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

        case 16 :
            listen = floor(value);
        break;

        case 17 :
            feed = floor(value*2);
        break;

        case 18 :
            modeMakeUp = ceil(value);
        break;
              
		case 19 :
            outVolumeFader = value;
            outVolume = outVolumeFader * 8;
        break;
              
        case 20 :
            saturation = 0.5*value;
            satC = sin(saturation*cPi_);
        break; 

		case 21 :
            dryMixFader = value;
            dryMix = dryMixFader * 2;
        break;       
	}


// MAKING THE AUTO-MAKE-UP TRANSITION BETWEEN RMS AND PEAK SMOOOOTHER ...
    volume = outVolume / (modeMakeUp ? (  dB2Amp(threshDB -threshDB*ratio) * (  1 + min( rmsFader*2 , 1 ) * ( rmsSize>1 ) )  ): 1 ) ;
    // The '1 + min( blah blah) * ( blah)' is just random made up shit, suits good, tho

// Set state of no 'underThresh' to avoid stuck compression when not retriggered (i.e. no signal over thresh)
    underThresh = overThresh = 0;

}

//-----------------------------------------------------------------------------------------
float AVst::getParameter (long index)
{
    float v = 0;
	switch (index)
	{
		case  0 : v = threshFader;      break;
		case  1 : v = ratioFader;       break;
		case  2 : v = kneeFader;        break;
		case  3 : v = attackStart+0.5;  break;
		case  4 : v = attackFader;      break;
		case  5 : v = attackShape;      break;
		case  6 : v = releaseFader;     break;
		case  7 : v = releaseShape;      break;
		case  8 : v = envDet/2;         break;
		case  9 : v = freqEnvDetLPFader;  break;
		case 10 : v = rmsFader;         break;
		case 11 : v = freqLPFader;      break;
		case 12 : v = freqP1Fader;   break;
		case 13 : v = sqrt((BWP1-0.05)/4.95);   break;
		case 14 : v = dBgainP1Fader;   break;
		case 15 : v = freqHPFader;      break;
		case 16 : v = listen;           break;
		case 17 : v = feed / 2;         break;
        case 18 : v = modeMakeUp;       break;
		case 19 : v = outVolumeFader;   break;
		case 20 : v = saturation*2;    break;
		case 21 : v = dryMixFader;      break;
	}
    return v;
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterName (long index, char *label)
{
    switch (index)
	{
		case  0 : strcpy (label, "Threshold");     break;
		case  1 : strcpy (label, "Ratio");         break;
		case  2 : strcpy (label, "Knee");          break;
		case  3 : strcpy (label, "Attack Start");  break;
		case  4 : strcpy (label, "Attack");        break;
		case  5 : strcpy (label, "Attack Shape");        break;
		case  6 : strcpy (label, "Release");       break;
		case  7 : strcpy (label, "Release Shape");        break;
		case  8 : strcpy (label, "Envelope Detector");    break;
		case  9 : strcpy (label, "Smoothen Envelope");    break;
		case 10 : strcpy (label, "RMS Size");      break;
		case 11 : strcpy (label, "Low-Pass");      break;
		case 12 : strcpy (label, "Peak");     break;
		case 13 : strcpy (label, "Width");               break;
		case 14 : strcpy (label, "Gain");     break;
		case 15 : strcpy (label, "High-Pass");     break;
		case 16 : strcpy (label, "Listen");        break;
		case 17 : strcpy (label, "Feed");          break;
		case 18 : strcpy (label, "Make-Up");       break;
		case 19 : strcpy (label, "Output");        break;
		case 20 : strcpy (label, "Saturation"); break;
		case 21 : strcpy (label, "Dry Mix");       break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterDisplay (long index, char *text)
{
	switch (index)
	{
		case  0 : dB2string (thresh, text);           break;
		case  1 :
            if (ratio>0) float2string ( 1 / ratio , text);
            else strcpy( text, "oo");
        break;
		case  2 : float2string ( kneeDB * 2 , text);       break;
		case  3 : ms2string (attackStartSpls, text);       break;
		case  4 : ms2string (attackSpls, text);            break;
		case  5 : 
                 if (attackShape == 0) {strcpy (text, "LOG");}
            else if (attackShape == 1) {strcpy (text, "LIN");}
        break;
		case  6 : ms2string (releaseSpls, text);       break;
		case  7 : 
                 if (releaseShape == 0) {strcpy (text, "LOG");}
            else if (releaseShape == 1) {strcpy (text, "LIN");}
        break;
		case  8 :
            if (envDet == 0) strcpy (text, "DISTR");
            else if (envDet == 1) strcpy (text, "COMP");
            else if (envDet == 2) strcpy (text, "LIMIT");
       break;
        case  9 : float2string (freqEnvDetLPFader*100, text); break;
		case 10 :
            if (rmsSize <= 1) strcpy (text, "PEAK");
            else ms2string (rmsSize, text);
        break;
        case 11 :
            if (freqLPFader < 1) float2string (freqLP, text);
            else if (freqLPFader == 1) strcpy (text, "OFF");
        break;
        case 12 :
            float2string (freqP1, text);
        break;
        case 13 :
            float2string (BWP1, text);
        break;
        case 14 :
            if (dBgainP1) float2string (dBgainP1, text);
            else strcpy (text, "OFF");
        break;
		case 15 :
            if (freqHPFader > 0) float2string (freqHP, text);
            else if (freqHPFader == 0) strcpy (text, "OFF");
        break;
		case 16 : 
                 if (listen == 0) {strcpy (text, "OUT");}
            else if (listen == 1) {strcpy (text, "DET+F");}
        break;
		case 17 : 
                 if (feed == 0) {strcpy (text, "FF");}
            else if (feed == 1) {strcpy (text, "FBQ");}
            else if (feed == 2) {strcpy (text, "SIDE");}
        break;
		case 18 :
                 if (modeMakeUp == 0) {strcpy (text, "OFF");}
            else if (modeMakeUp == 1) {strcpy (text, "ON");}
        break;
		case 19 : dB2string (outVolume, text);        break;
		case 20 : float2string (saturation*200, text);           break;
		case 21 : dB2string (dryMix, text);           break;
	}
}

//-----------------------------------------------------------------------------------------
void AVst::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case  0 : strcpy (label, "dB");	    break;
		case  1 : strcpy (label, ":1");	    break;
		case  2 : strcpy (label, "dB");	    break;
		case  3 : strcpy (label, "ms");	    break;
		case  4 : strcpy (label, "ms");	    break;
		case  5 : strcpy (label, "Curve");	    break;
		case  6 : strcpy (label, "ms");	    break;
		case  7 : strcpy (label, "Curve");	    break;
		case  8 : strcpy (label, "Mode");   break;
		case  9 : strcpy (label, "%");      break;
		case 10 : strcpy (label, "ms");     break;
		case 11 : strcpy (label, "Hz");   	break;
		case 12 : strcpy (label, "Hz");	break;
		case 13 : strcpy (label, "Oct");	break;
		case 14 : strcpy (label, "dB");	break;
		case 15 : strcpy (label, "Hz");	    break;
		case 16 : strcpy (label, "Mode");   break;
		case 17 : strcpy (label, "Mode");   break;
		case 18 : strcpy (label, "Mode");   break;
		case 19 : strcpy (label, "dB");	    break;
		case 20 : strcpy (label, "%");	    break;
		case 21 : strcpy (label, "dB");	    break;
	}
}

//------------------------------------------------------------------------
bool AVst::getEffectName (char* name)
{
	strcpy (name, "Digital Versatile Compressor MASTER");
	return true;
}

//------------------------------------------------------------------------
bool AVst::getProductString (char* text)
{
	strcpy (text, "Digital Versatile Compressor MASTER");
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

    }
       
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
// =========================================
// FEED: set detL/detR according to the feed
/*         if (feed == 0) maxSpls = absmax(    *in1 , *in2    ); // FF
    else if (feed == 1) maxSpls = absmax( spl0Out , spl1Out ); // FBQ
    else if (feed == 2) maxSpls = absmax(  *in3++ , *in4++  ); // SIDE
*/

         if (feed == 0) {detL = (*in1) +cDC_;     detR = (*in2) +cDC_; } // FF
    else if (feed == 1) {detL = (spl0Out) +cDC_;  detR = (spl1Out) +cDC_; } // FBQ
    else if (feed == 2) {detL = (*in3++) +cDC_;   detR = (*in4++) +cDC_; } // SIDE

// =========================================
// DETECTOR FILTERING

    if (freqLPFader < 1)
    {
        tmp = detL;
        detL = b0LP * detL + b1LP * xl1LP + b2LP * xl2LP - a1LP * yl1LP - a2LP * yl2LP;
        xl2LP = xl1LP; xl1LP = tmp; yl2LP = yl1LP; yl1LP = detL;

        tmp = detR;
        detR = b0LP * detR + b1LP * xr1LP + b2LP * xr2LP - a1LP * yr1LP - a2LP * yr2LP;
        xr2LP = xr1LP; xr1LP = tmp; yr2LP = yr1LP; yr1LP = detR;
    }
    if (dBgainP1 != 0)
    {
        tmp = detL;
        detL = b0P1 * detL + b1P1 * xl1P1 + b2P1 * xl2P1 - a1P1 * yl1P1 - a2P1 * yl2P1;
        xl2P1 = xl1P1; xl1P1 = tmp; yl2P1 = yl1P1; yl1P1 = detL;
    
        tmp = detR;
        detR = b0P1 * detR + b1P1 * xr1P1 + b2P1 * xr2P1 - a1P1 * yr1P1 - a2P1 * yr2P1;
        xr2P1 = xr1P1; xr1P1 = tmp; yr2P1 = yr1P1; yr1P1 = detR;
    }
    if (freqHPFader > 0)
    {
        tmp = detL;
        detL = b0HP * detL + b1HP * xl1HP + b2HP * xl2HP - a1HP * yl1HP - a2HP * yl2HP;
        xl2HP = xl1HP; xl1HP = tmp; yl2HP = yl1HP; yl1HP = detL +cDC_;
 
        tmp = detR;
        detR = b0HP * detR + b1HP * xr1HP + b2HP * xr2HP - a1HP * yr1HP - a2HP * yr2HP;
        xr2HP = xr1HP; xr1HP = tmp; yr2HP = yr1HP; yr1HP = detR +cDC_;
    }


// =========================================
// TRACK MAX: via absmax
    maxSpls = absmax( detL , detR );

// =========================================
// ENVELOPE FOLLOWER: smooth the detector signal, so it represents the envelope curve of the incoming waveform
    if (envDet >= 1 && listen == 0)
    {
        seekMaxSpls = sqrt( (tmpEnvLP = a0EnvLP*maxSpls - b1EnvLP*tmpEnvLP + cDcAdd_) - cDcAdd_ );
        if (envDet == 1) maxSpls = seekMaxSpls;
        else if (envDet == 2) maxSpls = max(seekMaxSpls,maxSpls);
    }

// =========================================
// RMS: calculate RMS via running sum (or not if PEAK)
    if (rmsSize <= 1) rms = maxSpls;
    else
    {
        rmsSqrSum = max( rmsSqrSum-rmsBuffer[rmsBufPos] , 0 ) + ( rmsBuffer[rmsBufPos] = (maxSpls*maxSpls) );
        if ( ++rmsBufPos >= rmsSize ) rmsBufPos = 0;
        rms = sqrt( rmsSqrSum / rmsSize );
    }
    
// =========================================
// LOOKAHEAD: read and write left/right + current rms value from/to the lookAheadBuffer
    if (lookAheadSize > 1)
    {
        if (listen!=0)
        {
            lookAheadBufferIn1[lookAheadBufPos] = detL;
            lookAheadBufferIn2[lookAheadBufPos] = detR;
        }
        else
        {
            lookAheadBufferIn1[lookAheadBufPos] = (*in1);
            lookAheadBufferIn2[lookAheadBufPos] = (*in2);
        }

        lookAheadBufferRms[lookAheadBufPos] = rms;
        if ( ++lookAheadBufPos >= lookAheadSize ) lookAheadBufPos = 0;
        (*in1) = lookAheadBufferIn1[lookAheadBufPos];
        (*in2) = lookAheadBufferIn2[lookAheadBufPos];
  // Track maximum in lookAheadBuffer:
    //  rms = max(lookAheadBufferRms[lookAheadBufPos],rms);
    // ^^ LowCPU approx approach / HQ version:
/*        t = 0;
        for (unsigned long i = 0; i < lookAheadSize; i++)
            t = (lookAheadBufferRms[i] > lookAheadBufferRms[t]) ? i:t;
        rms = lookAheadBufferRms[t];
*/    
    // a bit CPU optimzed:
    lookAheadMaxTime--;
        if ( rms >= lookAheadRms ) {lookAheadMaxTime = lookAheadSize; lookAheadRms = rms; }
        else if (lookAheadMaxTime < 0)
        {
            t = 0;
            for (unsigned long i = 0; i < lookAheadSize; i++)
                t = (lookAheadBufferRms[i] > lookAheadBufferRms[t]) ? i:t;
            lookAheadRms = lookAheadBufferRms[t];
            lookAheadMaxTime = t - lookAheadBufPos;
            if (lookAheadMaxTime < 0) lookAheadMaxTime += lookAheadSize;
        }
        rms = lookAheadRms;
    }
    
// =========================================
// GR-TRACKER: Calculate the current gain to be applied
    if (overThresh >0) overThresh++;
    if (rms > threshLow)
    {
        underThresh = 0;
        if (overThresh <= 0) ++overThresh;
        if (overThresh > attackStartSpls){
            if (rms >= threshHigh)
            {
                seekGain =  thresh * pow(rms/thresh,ratio ) / rms ;
            }
            else
            {
            // DO THE NEW SOFTKNEE CODE !!!! HERE BITCH !!!! BTW, MAKE IT BETTER THAN THE OLD ONE !!!!
            // BUT NO MATTER WHAT, MAKE IT ROUND !!!!
            // old two edges
                seekGain = dB2Amp( (threshLowDB + ( amp2DB(rms) - threshLowDB ) * kneeSlopeDB) ) / rms;
            }
        }
    }
    else
    {
        underThresh++;
        seekGain = 1;
        if ( underThresh > attackStartSpls ) overThresh = 0;
    }

// =========================================
// GAIN FOLLOWER: make 'gain' follow seekGain with the specified sttack and release speed
        if (gain > seekGain) 
        {
            if (attackShape == 0) gain = max( gain*attack , seekGain );
            if (attackShape == 1) gain = max( gain-attack , seekGain );
        }
        else 
        {
            if (releaseShape == 0) gain = min( gain/release , seekGain );
            if (releaseShape == 1) gain = min( gain+release , seekGain );
        }

// =========================================
// OUTPUT: Calculate the comp's output (plus make FBQ feed)
    if (listen == 0)
    {
    // out mode: wether normal or inverse detector deosn't matter
        spl0Out = (*in1) * gain * volume;
        spl1Out = (*in2) * gain * volume;

    if (saturation)
    {
        spl0Out = sin( limit( (spl0Out) , 1 ) * saturation * cPi_ ) / satC;
        spl1Out = sin( limit( (spl1Out) , 1 ) * saturation * cPi_ ) / satC;
    }

    }
    else if (listen == 1)
    {
    // detector procession: disable FBQ!!!!
        spl0Out = (*in1);
        spl1Out = (*in2);
    }


// =========================================
// MIXING: REPLACING!!!!!: mix output*volume + dry mix
    if (listen == 0)
    {
    // normal or invert detector
        (*out1++) = spl0Out + (*in1++) * dryMix;
        (*out2++) = spl1Out + (*in2++) * dryMix;
    }
    else if (listen == 1)
    {
    // only filtered output: no gain, nor volume etc..., i.e. no precession
        (*out1++) = detL, (*in1++);
        (*out2++) = detR, (*in2++);        
    }
    
  } // eof while

}
