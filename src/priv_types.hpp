#pragma once
#include <dt/df/core/types.hpp>
#include <atomic>
#include <boost/graph/adjacency_list.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <shared_mutex>

namespace dt::df::editor
{
enum class VertexType
{
    node,
    input,
    output
};
struct VertexInfo
{
    int id;
    NodeId parent_id; //! -1 when type == VertexType::node
    VertexType type;
};

struct RefCon
{
    boost::signals2::connection connection;
    ~RefCon();
};
struct EdgeInfo
{
    EdgeId id;
    std::shared_ptr<RefCon> connection;
};
struct EdgeInfo_t
{
    typedef boost::edge_property_tag kind;
};

using EdgeProperty = boost::property<EdgeInfo_t, EdgeInfo>;
using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexInfo, EdgeProperty>;

using VertexDesc = Graph::vertex_descriptor;
using EdgeDesc = Graph::edge_descriptor;

struct NodeDisplayVertex
{
    std::string node_key; //! might be empty if a group
    std::string display_name;
};
using NodeTree = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, NodeDisplayVertex>;
} // namespace dt::df::editor
