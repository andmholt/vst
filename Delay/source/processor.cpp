//------------------------------------------------------------------------
// Copyright(c) 2022 Niche Sounds.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <fstream>

using namespace Steinberg;

namespace MyCompanyName {
//------------------------------------------------------------------------
// DelayProcessor
//------------------------------------------------------------------------
DelayProcessor::DelayProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kDelayControllerUID);
}

//------------------------------------------------------------------------
DelayProcessor::~DelayProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!
	delete mLBuffer;
	delete mRBuffer;

	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::setActive (TBool state)
{
	// get default bus arrangement
	Steinberg::Vst::SpeakerArrangement arr;
	if (getBusArrangement(Steinberg::Vst::kOutput, 0, arr) != kResultTrue)
		return kResultFalse;

	// make sure there are active channels
	int32 numChannels = Steinberg::Vst::SpeakerArr::getChannelCount(arr);
	if (numChannels == 0)
		return kResultFalse;

	/*if (state) {
		mBuffer = (float**)std::malloc(numChannels * sizeof(float*));

		size_t size = (size_t)(processSetup.sampleRate * sizeof(float) + 0.5);
		for (int32 channel = 0; channel < numChannels; channel++) {
			mBuffer[channel] = (float*)std::malloc (size);	// 1 second delay max
			if (mBuffer[channel])
				memset (mBuffer[channel], 0, size);
		}
		mBufferPos = 0;
	} else {
		if (mBuffer) {
			for (int32 channel = 0; channel < numChannels; channel++) {
				std::free (mBuffer[channel]);
			}
			std::free (mBuffer);
			mBuffer = nullptr;
		}
	}*/

	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData(index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount();
                switch (paramQueue->getParameterId())
                {
					case kBypassId :
						if (paramQueue->getPoint(numPoints-1, sampleOffset, value) == kResultTrue)
							mBypass = (value > 0.5f);
						break;
					case kParamLDelayId :
						if (paramQueue->getPoint(numPoints-1, sampleOffset, value) == kResultTrue) {
							mLDelay = value;
							mLDelayNorm = (int)(mLDelay * 100000);
						}
						break;
					case kParamRDelayId :
						if (paramQueue->getPoint(numPoints-1, sampleOffset, value) == kResultTrue)
							mRDelay = value;
							mRDelayNorm = (int)(mRDelay * 100000);
						break;
					case kParamMixId :
						if (paramQueue->getPoint(numPoints-1, sampleOffset, value) == kResultTrue)
							mMix = value;
						break;
				}
			}
		}
	}
	
	//--- Here you have to implement your processing

	if (data.numSamples > 0) {

		// get info
		Steinberg::Vst::SpeakerArrangement arr;
		getBusArrangement(Steinberg::Vst::kOutput, 0, arr);
		int32 numChannels = Steinberg::Vst::SpeakerArr::getChannelCount(arr);

		// apply delay
		for (int32 channel = 0; channel < numChannels; channel++) {
			float* inputChannel = data.inputs[0].channelBuffers32[channel];
			float* outputChannel = data.outputs[0].channelBuffers32[channel];

			// process by samples
			for (int32 sample = 0; sample < data.numSamples; sample++) {

				float tempSample = inputChannel[sample];

				// set current channel data
				std::list<float>* currBuffer;
				float currDelayNorm;
				if (channel == 0) {
					currBuffer = mLBuffer;
					currDelayNorm = mLDelayNorm;
				}
				else if (channel == 1) {
					currBuffer = mRBuffer;
					currDelayNorm = mRDelayNorm;
				}

				// save data for future delays
				currBuffer->push_back(tempSample);

				// check for current delay
				if ((int)currBuffer->size()-1 >= currDelayNorm) {
					// if more samples than there should be, correct and continue
					while ((int)currBuffer->size()-1 > currDelayNorm)
						currBuffer->pop_front();

					float delayedSample = currBuffer->front();
					currBuffer->pop_front();

					// account for mix
					delayedSample *= mMix;
					tempSample *= (1-mMix);
					tempSample += delayedSample;

					// set output
					outputChannel[sample] = tempSample;
				}
				// else, copy to output and continue
				else
					outputChannel[sample] = inputChannel[sample];
			}
		}
	}	

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DelayProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace MyCompanyName
