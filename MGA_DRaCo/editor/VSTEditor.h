

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
	CParamControl* threshControl;
    CParamControl* ratioControl;
    CParamControl* attackControl;
    CParamControl* releaseControl;
    CParamControl* outputControl;

	CParamDisplay* GRDisplay;

	CVUMeter* GRMeter;
                	
	// Bitmap
	CBitmap* hBackground;
};

#endif
