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
    kK12OffId,
    	
	// positions
    kControlWidth = 40,
    kControlHeight = 16,
	kGainX = 8,
	kGainY = 30,
   	
	kGRMeterX = 68,

	kGRMeterLY = 30,
	kGRMeterRY = 42,

    kVUDisplayWidth = 40,
    kVUDisplayHeight = 8,

	kVUDisplayX = 188,

	kGRDisplayLY = 30,
	kGRDisplayRY = 39,

    kK12DisplayLY = 52,
    kK12DisplayRY = 61,  

    kK12MeterX = 8,
    kK12MeterLY = 52,
    kK12MeterRY = 64
                	

};

//-----------------------------------------------------------------------------
// prototype string convert float -> percent
void gain2Display (float value, char* string);
void gain2Display (float value, char* string)
{
    sprintf ( string, "%.1fdB", 20*value );
}

void GRAmp2dBConvert (float value, char* string);
void GRAmp2dBConvert (float value, char* string)
{
    if(value>0) sprintf ( string, "%.1fdB", -log(value)*8.685889638 );
    else sprintf(string, "oo dB");
}

void amp2K12dBConvert (float value, char* string);
void amp2K12dBConvert (float value, char* string)
{
    if(value>0) sprintf ( string, "%.1fdB", log(value)*8.685889638 + 15.010299957 );
    else sprintf(string, "-oo dB");
}

//-----------------------------------------------------------------------------
// Editor class implementation
//-----------------------------------------------------------------------------
VSTEditor::VSTEditor (AudioEffect *effect)
 : AEffGUIEditor (effect) 
{
	hBackground = new CBitmap (kBackgroundId);

	gainControl = 0;

    GRMeterL = 0;
    GRMeterR = 0;

    GRDisplayL = 0;
    GRDisplayR = 0;

    K12DisplayL = 0;
    K12DisplayR = 0;

    K12MeterL = 0;
    K12MeterR = 0;
      
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
	CBitmap* hK12Off   = new CBitmap (kK12OffId);
             
	//--init background frame-----------------------------------------------
	// We use a local CFrame object so that calls to setParameter won't call into objects which may not exist yet. 
	// If all GUI objects are created we assign our class member to this one. See bottom of this method.
	CRect size (0, 0, hBackground->getWidth (), hBackground->getHeight ());
	CFrame* lFrame = new CFrame (size, ptr, this);
	lFrame->setBackground (hBackground);
	
    //--init/make display/controls
	
	size (kGainX, kGainY,
          kGainX + kControlWidth, kGainY + kControlHeight);
    gainControl = new CParamControl (size, this, kGain, 0);
    gainControl->setTransparency(true); 
    gainControl->setFontColor (kWhiteCColor);
    gainControl->setFont (kNormalFontSmaller); 
    gainControl->setHoriAlign(kRightText);
	gainControl->setValue (effect->getParameter (kGain));
	gainControl->setDefaultValue (0.0f);
	gainControl->setRange(20);
	gainControl->setFineRange(200);
    gainControl->setStringConvert( gain2Display );
//    threshControl->setTag( kGain );
    lFrame->addView( gainControl );
        

    size(kGRMeterX, kGRMeterLY,
         kGRMeterX+hGROff->getWidth(),kGRMeterLY+hGROff->getHeight());
    GRMeterL = new CVUMeter(size, hGROff, 0, kHorizontal);
    GRMeterL->setValue ( (10  +  max( (log( effect->getVU( kGRL ) ) * 8.685889638) , -10 ))/10  );
    GRMeterL->setTag( kGRL+kNumParams );
    lFrame->addView(GRMeterL);

    size(kGRMeterX, kGRMeterRY,
         kGRMeterX+hGROff->getWidth(),kGRMeterRY+hGROff->getHeight());
    GRMeterR = new CVUMeter(size, hGROff, 0, kHorizontal);
    GRMeterR->setValue ( (10  +  max( (log( effect->getVU( kGRR ) ) * 8.685889638) , -10 ))/10  );
    GRMeterR->setTag( kGRR+kNumParams );
    lFrame->addView(GRMeterR);

	size (kVUDisplayX, kGRDisplayLY,
          kVUDisplayX + kVUDisplayWidth, kGRDisplayLY + kVUDisplayHeight);
	GRDisplayL = new CParamDisplay (size, 0, kRightText);
	GRDisplayL->setFont (kNormalFontVerySmall);
	GRDisplayL->setFontColor (kWhiteCColor);
	GRDisplayL->setValue ( effect->getVU( kGRPeakL ) );
	GRDisplayL->setTransparency(true);
	GRDisplayL->setHoriAlign(kRightText);
	GRDisplayL->setStringConvert( GRAmp2dBConvert );
    GRDisplayL->setTag( kGRPeakL+kNumParams );
	lFrame->addView (GRDisplayL);


	size (kVUDisplayX, kGRDisplayRY,
          kVUDisplayX + kVUDisplayWidth, kGRDisplayRY + kVUDisplayHeight);
	GRDisplayR = new CParamDisplay (size, 0, kRightText);
	GRDisplayR->setFont (kNormalFontVerySmall);
	GRDisplayR->setFontColor (kWhiteCColor);
	GRDisplayR->setValue ( effect->getVU( kGRPeakR ) );
	GRDisplayR->setTransparency(true);
	GRDisplayR->setHoriAlign(kRightText);
	GRDisplayR->setStringConvert( GRAmp2dBConvert );
    GRDisplayR->setTag( kGRPeakR+kNumParams );
	lFrame->addView (GRDisplayR);


// K-12
    size(kK12MeterX, kK12MeterLY,
         kK12MeterX+hK12Off->getWidth(),kK12MeterLY+hK12Off->getHeight());
    K12MeterL = new CVUMeter(size, hK12Off, 0, kHorizontal);
    K12MeterL->setValue (  0  );
    K12MeterL->setTag( kKMeterL+kNumParams );
    lFrame->addView(K12MeterL);

    size(kK12MeterX, kK12MeterRY,
         kK12MeterX+hK12Off->getWidth(),kK12MeterRY+hK12Off->getHeight());
    K12MeterR = new CVUMeter(size, hK12Off, 0, kHorizontal);
    K12MeterR->setValue (  0  );
    K12MeterR->setTag( kKMeterR+kNumParams );
    lFrame->addView(K12MeterR);

	size (kVUDisplayX, kK12DisplayLY,
          kVUDisplayX + kVUDisplayWidth, kK12DisplayLY + kVUDisplayHeight);
	K12DisplayL = new CParamDisplay (size, 0, kRightText);
	K12DisplayL->setFont (kNormalFontVerySmall);
	K12DisplayL->setFontColor (kBlackCColor);
	K12DisplayL->setValue ( effect->getVU( kKMeterL ) );
	K12DisplayL->setTransparency(true);
	K12DisplayL->setHoriAlign(kRightText);
	K12DisplayL->setStringConvert( amp2K12dBConvert );
    K12DisplayL->setTag( kKMeterL+kNumParams );
	lFrame->addView (K12DisplayL);
                      	
	size (kVUDisplayX, kK12DisplayRY,
          kVUDisplayX + kVUDisplayWidth, kK12DisplayRY + kVUDisplayHeight);
	K12DisplayR = new CParamDisplay (size, 0, kRightText);
	K12DisplayR->setFont (kNormalFontVerySmall);
	K12DisplayR->setFontColor (kBlackCColor);
	K12DisplayR->setValue ( effect->getVU( kKMeterR ) );
	K12DisplayR->setTransparency(true);
	K12DisplayR->setHoriAlign(kRightText);
	K12DisplayR->setStringConvert( amp2K12dBConvert );
    K12DisplayR->setTag( kKMeterR+kNumParams );
	lFrame->addView (K12DisplayR);

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
    if(frame)
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
		case kGain :
            if(gainControl)
	           gainControl->setValue ( effect->getParameter (index));
			break;
	}

}

//-----------------------------------------------------------------------------
void VSTEditor::valueChanged (CDrawContext* context, CControl* control)
{
	long tag = control->getTag ();
	switch (tag)
	{
		case kGain :
			effect->setParameterAutomated (tag, control->getValue ());
			control->setDirty ();
        break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void VSTEditor::idle ()
{

    GRDisplayL->setValue ( effect->getVU( kGRPeakL ) );
    GRDisplayR->setValue ( effect->getVU( kGRPeakR ) );
    GRMeterL->setValue (  (10  +  max( (log( effect->getVU( kGRL ) ) * 8.685889638) , -10 ))/10   );
    GRMeterR->setValue (  (10  +  max( (log( effect->getVU( kGRR ) ) * 8.685889638) , -10 ))/10   );

    K12MeterL->setValue ( (24 +  max( log( effect->getVU( kKMeterL ) ) * 8.685889638 + 15.010299957, -24) )/36.f );
    K12DisplayL->setValue ( effect->getVU( kKMeterL ) );

    K12MeterR->setValue ( (24 +  max( log( effect->getVU( kKMeterR ) ) * 8.685889638 + 15.010299957, -24) )/36.f );
    K12DisplayR->setValue ( effect->getVU( kKMeterR ) );

    effect->resetLastIdleCall();

    AEffGUIEditor::idle();    
}
