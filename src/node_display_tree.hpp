#pragma once
#include "priv_types.hpp"
#include "dt/df/editor/types.hpp"

namespace dt::df::editor
{
class NodeDisplayGraph
{
    using Desc = NodeTree::vertex_descriptor;

  public:
    NodeDisplayGraph();
    void addNode(const std::string &node_key, const std::string &node_name);
    void drawTree(const NodeDisplayDrawFnc &draw_fnc) const;

  private:
    Desc addNodeToGraph(const std::string &node_key, const std::string &node_name, const Desc parent);

  private:
    NodeTree node_tree_;
    NodeTree::vertex_descriptor root_node_;
};
} // namespace dt::df::editor
