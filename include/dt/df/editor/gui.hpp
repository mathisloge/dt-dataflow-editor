#pragma once
#include <Magnum/GL/Context.h>
#include <imgui.h>
#include <imnodes.h>
#include "dtdatafloweditor_export.h"

namespace dt::df::editor
{
void DTDATAFLOWEDITOR_EXPORT InitGui(Magnum::GL::Context &gl_ctx,
                                     ImGuiContext *imgui_ctx,
                                     imnodes::Context *imnodes_ctx);
void DTDATAFLOWEDITOR_EXPORT ShutdownGui();
} // namespace dt::df::editor
