//------------------------------------------------------------------------
// Copyright(c) 2022 Andrew Holt.
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "base/source/fstreamer.h"

using namespace Steinberg;

namespace MyCompanyName {

//------------------------------------------------------------------------
// DelayController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Here you could register some parameters
	parameters.addParameter(STR16 ("Bypass"), nullptr, 1, 0, Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass, DelayParams::kBypassId);

	parameters.addParameter(STR16 ("Left Delay"), STR16 ("sec"), 0, 0.2, Vst::ParameterInfo::kCanAutomate, DelayParams::kParamLDelayId);
	parameters.addParameter(STR16 ("Right Delay"), STR16 ("sec"), 0, 0.2, Vst::ParameterInfo::kCanAutomate, DelayParams::kParamRDelayId);
	
	parameters.addParameter(STR16 ("Mix"), STR16 ("%"), 0, 1, Vst::ParameterInfo::kCanAutomate, DelayParams::kParamMixId);

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	Steinberg::IBStreamer streamer(state, kLittleEndian);

	// bypass
	int32 bypassState = 0;
	if (!streamer.readInt32(bypassState)) {
		// could be old version, continue
	}
	setParamNormalized(kBypassId, bypassState ? 1 : 0);

	// left delay
	float savedLDelay = 0;
	if (!streamer.readFloat(savedLDelay))
		return kResultFalse;
	setParamNormalized(kParamLDelayId, savedLDelay);

	// right delay
	float savedRDelay = 0;
	if (!streamer.readFloat(savedRDelay))
		return kResultFalse;
	setParamNormalized(kParamRDelayId, savedRDelay);

	float savedMix = 0;
	if (!streamer.readFloat(savedMix))
		return kResultFalse;
	setParamNormalized(kParamMixId, savedMix);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API DelayController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

//------------------------------------------------------------------------
} // namespace MyCompanyName
