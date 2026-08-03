#pragma once
#include <engine/elog.h>

const elog::Colour red{255, 0, 0};
const elog::Colour muted_yellow{0, 160, 160};

ELOG_DECLARE_LOGGING_CATEGORY(lcEngineGraphics, "engine.graphics", muted_yellow)
ELOG_DECLARE_LOGGING_CATEGORY(lcEngineGraphicsBindings, "engine.graphics.bindings", red)
ELOG_DECLARE_LOGGING_CATEGORY(lcEngineResources, "engine.resources", red)

