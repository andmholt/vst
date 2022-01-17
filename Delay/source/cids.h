//------------------------------------------------------------------------
// Copyright(c) 2022 Niche Sounds.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MyCompanyName {
//------------------------------------------------------------------------
static const Steinberg::FUID kDelayProcessorUID (0x0DB2A480, 0x6635518C, 0x8A440408, 0x4EF49066);
static const Steinberg::FUID kDelayControllerUID (0x82ACF490, 0xF5CD58B9, 0xA2E296A3, 0x2ABF71A2);

#define DelayVST3Category "Fx"

enum DelayParams : Steinberg::Vst::ParamID {
    kBypassId = 100,

    kParamLDelayId = 200,
    kParamRDelayId = 400,

    kParamMixId = 600
};

//------------------------------------------------------------------------
} // namespace MyCompanyName
