#pragma once
#include "dtdfeditor_export.h"
#include <imgui.h>
#include <imnodes.h>

namespace dt::df::editor
{
void DTDFEDITOR_EXPORT InitGui(ImGuiContext *imgui_ctx, imnodes::Context* imnodes_ctx);
void DTDFEDITOR_EXPORT ShutdownGui();
} // namespace dt::df::editor
