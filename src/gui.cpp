#include "dt/df/editor/gui.hpp"
#include <imgui.h>
#include <imnodes.h>
namespace dt::df::editor
{
void InitGui(ImGuiContext *imgui_ctx, imnodes::Context *imnodes_ctx)
{
#ifndef DTDFEDITOR_STATIC_DEFINE
#ifdef WIN32
    ImGui::SetCurrentContext(imgui_ctx);
    imnodes::SetCurrentContext(imnodes_ctx);
#endif
#endif
}

void ShutdownGui()
{
#ifndef DTDFEDITOR_STATIC_DEFINE
#ifdef WIN32
    ImGui::SetCurrentContext(nullptr);
#endif
#endif
}
} // namespace dt::df::editor
