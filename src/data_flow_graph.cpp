#include "dt/df/editor/data_flow_graph.hpp"
#include "graph_impl.hpp"
namespace dt::df::editor
{

DataFlowGraph::DataFlowGraph()
    : impl_{new GraphImpl()}
{}

void DataFlowGraph::init()
{
    impl_->init();
}

void DataFlowGraph::addNode(const NodeKey &key, int preferred_x, int preferred_y, bool screen_space)
{
    impl_->createNode(key, preferred_x, preferred_y, screen_space);
}

void DataFlowGraph::removeNode(const NodeId id)
{
    impl_->removeNode(id);
}

void DataFlowGraph::addEdge(const NodeId from, const NodeId to)
{
    try
    {
        auto from_vert = impl_->findVertexById(from);
        auto to_vert = impl_->findVertexById(to);
        impl_->addEdge(from_vert, to_vert);
    }
    catch (const std::out_of_range &)
    {
        //! \todo we need to log the error
    }
}

void DataFlowGraph::removeEdge(const EdgeId id)
{
    impl_->removeEdge(id);
}

void DataFlowGraph::render()
{
    impl_->renderNodes();
    impl_->renderLinks();
}

void DataFlowGraph::save(const std::filesystem::path &file)
{
    impl_->save(file);
}

void DataFlowGraph::clear()
{
    impl_->clear();
}

void DataFlowGraph::clearAndLoad(const std::filesystem::path &file)
{
    impl_->clearAndLoad(file);
}

void DataFlowGraph::renderNodeDisplayTree(const NodeDisplayDrawFnc &draw_fnc) const
{
    impl_->nodeDisplayNames().drawTree(draw_fnc);
}

DataFlowGraph::~DataFlowGraph()
{
    delete impl_;
}
} // namespace dt::df::editor
