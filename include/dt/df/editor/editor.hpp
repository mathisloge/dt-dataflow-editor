#pragma once
#include "data_flow_graph.hpp"
#include "dtdatafloweditor_export.h"
#include "types.hpp"

namespace dt::df::editor
{
class DTDATAFLOWEDITOR_EXPORT Editor
{
  public:
    static constexpr std::string_view kDndTarget = "DND_DATAFLOW";

  public:
    Editor();
    Editor(const Editor &) = delete;
    Editor &operator=(const Editor &) = delete;
    void init();
    void render();
    void renderNodeDisplayTree(const NodeDisplayDrawFnc &draw_fnc) const;
    DataFlowGraph &graph();
    const DataFlowGraph &graph() const;

    virtual ~Editor();

  private:
    class Impl;
    Impl *impl_;
};
} // namespace dt::df::editor
