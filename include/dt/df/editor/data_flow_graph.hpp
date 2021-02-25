#pragma once
#include <filesystem>
#include <functional>
#include <dt/df/core/types.hpp>
#include "dtdatafloweditor_export.h"
#include "types.hpp"
namespace dt::df::editor
{
class GraphImpl;
class DTDATAFLOWEDITOR_EXPORT DataFlowGraph
{
  public:
    DataFlowGraph();
    DataFlowGraph(const DataFlowGraph &) = delete;
    DataFlowGraph &operator=(const DataFlowGraph &) = delete;
    void init();
    void addNode(const NodeKey &key, int preferred_x = 0, int preferred_y = 0, bool screen_space = false);
    void removeNode(const NodeId id);
    void addEdge(const NodeId from, const NodeId to);
    void removeEdge(const EdgeId id);

    void render();
    void renderNodeDisplayTree(const NodeDisplayDrawFnc &draw_fnc) const;
    void save(const std::filesystem::path &file);
    void clear();
    void clearAndLoad(const std::filesystem::path &file);

    virtual ~DataFlowGraph();

  private:
    GraphImpl *impl_;
    friend GraphImpl;
};
} // namespace dt::df::editor
