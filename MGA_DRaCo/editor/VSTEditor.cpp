#include <cmath>
#include <cstdio>

#ifndef __VSTEditor__
#include "VSTEditor.h"
#endif

#ifndef __VST__
#include "../VST.h"
#endif

//-----------------------------------------------------------------------------
// resource id's
enum {
    // bitmaps
	kBackgroundId = 128,
    kGROffId,
	
	// positions
    kControlWidth = 40,
    kControlHeight = 16,
	kThreshX = 8,
	kThreshY = 30,

	kRatioX = 8,
	kRatioY = 52,

	kAttackX = 68,
	kAttackY = 52,

	kReleaseX = 128,
	kReleaseY = 52,
    
    kOutputX = 188,
	kOutputY = 52,
    	
	kGRDisplayX = 188,
	kGRDisplayY = 29,
            	
	kGRMeterX = 68,
	kGRMeterY = 30
};

//-----------------------------------------------------------------------------
// prototype string convert float -> percent
void thresh2Display (float value, char* string);
void thresh2Display (float value, char* string)
{
    sprintf ( string, "%.1fdB", -40*(1-value) );
}

void ratio2Display (float value, char* string);
void ratio2Display (float value, char* string)
{
    if(value<1) sprintf ( string, "%.1f:1", value*99+1 );
    else sprintf ( string, "oo :1" );
}

void attack2Display (float value, char* string);
void attack2Display (float value, char* string)
{
    sprintf ( string, "%.1fms", value*500 );
}

void release2Display (float value, char* string);
void release2Display (float value, char* string)
{
    sprintf ( string, "%.0fms", value*2500 );
}

void amp2dBConvert (float value, char* string);
void amp2dBConvert (float value, char* string)
{
    if(value>0) sprintf ( string, "%.1fdB", -log(value)*8.685889638 );
    else sprintf(string, "oo dB");
}

void output2Display (float value, char* string);
void output2Display (float value, char* string)
{
    sprintf ( string, "%.1fdB", 40*value );
}

//-----------------------------------------------------------------------------
// Editor class implementation
//-----------------------------------------------------------------------------
VSTEditor::VSTEditor (AudioEffect *effect)
 : AEffGUIEditor (effect) 
{
	hBackground = new CBitmap (kBackgroundId);

	threshControl = 0;
	ratioControl = 0;
	attackControl = 0;
	releaseControl = 0;
	outputControl = 0;

    GRDisplay = 0;
    GRMeter = 0;
        
	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)hBackground->getWidth ();
	rect.bottom = (short)hBackground->getHeight ();

}

//-----------------------------------------------------------------------------
VSTEditor::~VSTEditor ()
{
	// free the background bitmap
	if (hBackground)
		hBackground->forget ();
	hBackground = 0;

}

//-----------------------------------------------------------------------------
bool VSTEditor::open (void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open (ptr);

	//--load some bitmaps
	CBitmap* hGROff   = new CBitmap (kGROffId);
         
	//--init background frame-----------------------------------------------
	// We use a local CFrame object so that calls to setParameter won't call into objects which may not exist yet. 
	// If all GUI objects are created we assign our class member to this one. See bottom of this method.
	CRect size (0, 0, hBackground->getWidth (), hBackground->getHeight ());
	CFrame* lFrame = new CFrame (size, ptr, this);
	lFrame->setBackground (hBackground);
	
    //--init/make display/controls
	
	size (kThreshX, kThreshY,
          kThreshX + kControlWidth, kThreshY + kControlHeight);
    threshControl = new CParamControl (size, this, kThresh, 0);
    threshControl->setTransparency(true); 
    threshControl->setFontColor (kWhiteCColor);
    threshControl->setFont (kNormalFontSmaller); 
    threshControl->setHoriAlign(kRightText);
	threshControl->setValue (effect->getParameter (kThresh));
	threshControl->setDefaultValue (1.0f);
	threshControl->setRange(40);
	threshControl->setFineRange(400);
    threshControl->setStringConvert( thresh2Display );
//    threshControl->setTag( kThresh );
    lFrame->addView( threshControl );
        
	size (kRatioX, kRatioY,
          kRatioX + kControlWidth, kRatioY + kControlHeight);
    ratioControl = new CParamControl (size, this, kRatio, 0);
    ratioControl->setTransparency(true); 
    ratioControl->setFontColor (kWhiteCColor);
    ratioControl->setFont (kNormalFontSmaller); 
    ratioControl->setHoriAlign(kRightText);
	ratioControl->setValue (effect->getParameter (kRatio));
	ratioControl->setDefaultValue (0.0f);
	ratioControl->setRange(99);
	ratioControl->setFineRange(990);
    ratioControl->setStringConvert( ratio2Display );
//    ratioControl->setTag( kRatio );
    lFrame->addView( ratioControl);

	size (kAttackX, kAttackY,
          kAttackX + kControlWidth, kAttackY + kControlHeight);
    attackControl = new CParamControl (size, this, kAttack, 0);
    attackControl->setTransparency(true); 
    attackControl->setFontColor (kWhiteCColor);
    attackControl->setFont (kNormalFontSmaller); 
    attackControl->setHoriAlign(kRightText);
	attackControl->setValue (effect->getParameter (kAttack));
	attackControl->setDefaultValue (0.02f);
	attackControl->setRange(500);
	attackControl->setFineRange(5000);
    attackControl->setStringConvert( attack2Display );
//    attackControl->setTag( kAttack );
    lFrame->addView( attackControl);
    
	size (kReleaseX, kReleaseY,
          kReleaseX + kControlWidth, kReleaseY + kControlHeight);
    releaseControl = new CParamControl (size, this, kRelease, 0);
    releaseControl->setTransparency(true); 
    releaseControl->setFontColor (kWhiteCColor);
    releaseControl->setFont (kNormalFontSmaller); 
    releaseControl->setHoriAlign(kRightText);
	releaseControl->setValue (effect->getParameter (kRelease));
	releaseControl->setDefaultValue (0.08f);
	releaseControl->setRange(250);
	releaseControl->setFineRange(2500);
    releaseControl->setStringConvert( release2Display );
//    releaseControl->setTag( kRelease );
    lFrame->addView( releaseControl);

	size (kOutputX, kOutputY,
          kOutputX + kControlWidth, kOutputY + kControlHeight);
    outputControl = new CParamControl (size, this, kOutput, 0);
    outputControl->setTransparency(true); 
    outputControl->setFontColor (kWhiteCColor);
    outputControl->setFont (kNormalFontSmaller); 
    outputControl->setHoriAlign(kRightText);
	outputControl->setValue (effect->getParameter (kOutput));
	outputControl->setDefaultValue (0.0f);
	outputControl->setRange(40);
	outputControl->setFineRange(400);
    outputControl->setStringConvert( output2Display );
//    outputControl->setTag( kOutput );
    lFrame->addView( outputControl);



    size(kGRMeterX, kGRMeterY,
         kGRMeterX+hGROff->getWidth(),kGRMeterY+hGROff->getHeight());
    GRMeter = new CVUMeter(size, hGROff, 20, kHorizontal);
    GRMeter->setValue ( (20  +  max( (log( effect->getVU( kGR ) ) * 8.685889638) , -20 ))/20  );
//    VUMeter->setTag( kGR+kNumParams );
    lFrame->addView(GRMeter);


	size (kGRDisplayX, kGRDisplayY,
          kGRDisplayX + kControlWidth, kGRDisplayY + kControlHeight);
	GRDisplay = new CParamDisplay (size, 0, kRightText);
	GRDisplay->setFont (kNormalFontVerySmall);
	GRDisplay->setFontColor (kWhiteCColor);
	GRDisplay->setValue ( effect->getVU( kGRPeak ) );
	GRDisplay->setTransparency(true);
	GRDisplay->setHoriAlign(kRightText);
	GRDisplay->setStringConvert( amp2dBConvert );
//    VUDisplay->setTag( kPeak+kNumParams );
	lFrame->addView (GRDisplay);
              	
	// Note : in the constructor of a CBitmap, the number of references is set to 1.
	// Then, each time the bitmap is used (for hinstance in a vertical slider), this
	// number is incremented.
	// As a consequence, everything happens as if the constructor by itself was adding
	// a reference. That's why we need til here a call to forget ().
	// You mustn't call delete here directly, because the bitmap is used by some CControls...
	// These "rules" apply to the other VSTGUI objects too.

	hGROff->forget();

	frame = lFrame;
	return true;
}

//-----------------------------------------------------------------------------
void VSTEditor::close ()
{
//    if (frame)
        delete frame;
    frame = 0;
}

//-----------------------------------------------------------------------------
void VSTEditor::setParameter (VstInt32 index, float value)
{
	if (frame == 0)
		return;

	// called from ADelayEdit
	switch (index)
	{
		case kThresh :
            if(threshControl)
	           threshControl->setValue ( effect->getParameter (index));
			break;

		case kRatio :
            if(ratioControl)
	           ratioControl->setValue ( effect->getParameter (index));
			break;

		case kAttack :
            if(attackControl)
	           attackControl->setValue ( effect->getParameter (index));
			break;

		case kRelease :
            if(releaseControl)
	           releaseControl->setValue ( effect->getParameter (index));
			break;

		case kOutput :
            if(outputControl)
	           outputControl->setValue ( effect->getParameter (index));
			break;
	}

}

//-----------------------------------------------------------------------------
void VSTEditor::valueChanged (CDrawContext* context, CControl* control)
{
    if(frame == 0)
        return;

	long tag = control->getTag ();
	switch (tag)
	{
		case kThresh :
		case kRatio :
		case kAttack :
        case kRelease :
		case kOutput :
        	effect->setParameterAutomated (tag, control->getValue ());
        	control->setDirty ();
		break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void VSTEditor::idle ()
{
    AEffGUIEditor::idle();  
	if (frame == 0)
		return;
    GRDisplay->setValue ( effect->getVU( kGRPeak ) );
    GRMeter->setValue (  (20  +  max( (log( effect->getVU( kGR ) ) * 8.685889638) , -20 ))/20   );
    effect->resetLastIdleCall(); 

}
