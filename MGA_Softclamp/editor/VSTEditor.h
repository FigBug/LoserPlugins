

#ifndef __VSTeditor__
#define __VSTeditor__

// include VSTGUI
#ifndef __vstgui__
#include "vstgui.h"
#endif



//-----------------------------------------------------------------------------
class VSTEditor : public AEffGUIEditor, public CControlListener
{
public:
	VSTEditor (AudioEffect* effect);
	virtual ~VSTEditor ();

    virtual void idle ();
public:
	virtual bool open (void* ptr);
	virtual void close ();

	virtual void setParameter (VstInt32 index, float value);
	virtual void valueChanged (CDrawContext* context, CControl* control);

private:
       
    // Displays
	CParamControl* gainControl;
	CParamControl* kneeControl;
	
	CParamDisplay* GRDisplayL;
	CParamDisplay* GRDisplayR;
	CVUMeter* GRMeterL;
	CVUMeter* GRMeterR;

	CVUMeter* K12MeterL;
	CVUMeter* K12MeterR;
		
	CParamDisplay* K12DisplayL;                	
	CParamDisplay* K12DisplayR;
	
	// Bitmap
	CBitmap* hBackground;
};

#endif
