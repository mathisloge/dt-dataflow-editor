#pragma once
#include <imgui.h>
#include <imnodes.h>
#include "dtdatafloweditor_export.h"

namespace dt::df::editor
{
void DTDATAFLOWEDITOR_EXPORT InitGui(ImGuiContext *imgui_ctx, imnodes::Context *imnodes_ctx);
void DTDATAFLOWEDITOR_EXPORT ShutdownGui();
} // namespace dt::df::editor
