#include "graph_impl.hpp"

#include <cassert>
#include <fstream>

#include <Corrade/Containers/PointerStl.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Debug.h>
#include <dt/df/core/base_node.hpp>
#include <dt/df/core/base_slot.hpp>
#include <dt/df/plugin/plugin.hpp>
#include <imnodes.h>
#include <nlohmann/json.hpp>

using namespace Corrade;
namespace dt::df::editor
{

GraphImpl::GraphImpl()
{}

void GraphImpl::init()
{

    for (const auto &plugin_name : manager_.pluginList())
    {
        if (!(manager_.load(plugin_name) & PluginManager::LoadState::Loaded))
        {
            Utility::Error{} << "The requested plugin" << plugin_name.c_str() << "cannot be loaded.";
            continue;
        }
        std::unique_ptr<plugin::Plugin> plugin = std::move(manager_.instantiate(plugin_name));

        plugin->setup(Magnum::GL::Context::current(), ImGui::GetCurrentContext(), imnodes::GetCurrentContext());
        plugin->registerSlotFactories(*this);
        loaded_plugins_.emplace_back(std::move(plugin));
    }
    // load after all slots have been registerd
    for (const auto &plugin : loaded_plugins_)
    {
        plugin->registerNodeFactories(*this);
    }
}

NodeId GraphImpl::generateNodeId()
{
    return vertex_id_counter_++;
}
SlotId GraphImpl::generateSlotId()
{
    return vertex_id_counter_++;
}
bool GraphImpl::registerSlot(const NodeId node_id, const SlotId slot_id, const SlotType type)
{
    auto node = findNodeById(node_id);
    if (!node)
        return false;

    auto slot = (type == SlotType::input) ? node->inputs(slot_id) : node->outputs(slot_id);
    if (!slot)
        return false;

    try
    {
        const auto node_vert = findVertexById(node->id());
        addSlot(node, node_vert, slot, type);
        return true;
    }
    catch (const std::out_of_range &)
    {
        return false;
    }
    return false;
}
bool GraphImpl::unregisterSlot(const NodeId node_id, const SlotId slot_id)
{
    auto node = findNodeById(node_id);
    if (!node)
        return false;
    // check if the slot id is in the node.
    if (!node->inputs(slot_id) && !node->outputs(slot_id))
        return false;
    removeSlot(slot_id);
    return true;
}

void GraphImpl::registerNodeFactory(const NodeKey &key,
                                    const std::string &node_display_name,
                                    NodeFactory &&factory,
                                    NodeDeserializationFactory &&deser_factory)
{
    node_factories_.emplace(key, std::forward<NodeFactory>(factory));
    node_deser_factories_.emplace(key, std::forward<NodeDeserializationFactory>(deser_factory));

    node_display_names_.addNode(key, node_display_name);
}

void GraphImpl::registerSlotFactory(const SlotKey &key,
                                    SlotFactory &&factory,
                                    SlotDeserializationFactory &&deser_factory)
{
    slot_factories_.emplace(key, std::forward<SlotFactory>(factory));
    slot_deser_factories_.emplace(key, std::forward<SlotDeserializationFactory>(deser_factory));
}

const NodeFactory &GraphImpl::getNodeFactory(const NodeKey &key) const
{
    auto factory_fnc_it = node_factories_.find(key);
    if (factory_fnc_it == node_factories_.end())
        throw std::out_of_range("node factory not found");
    return factory_fnc_it->second;
}

const NodeDeserializationFactory &GraphImpl::getNodeDeserializationFactory(const NodeKey &key) const
{
    auto factory_fnc_it = node_deser_factories_.find(key);
    if (factory_fnc_it == node_deser_factories_.end())
        throw std::out_of_range("node deserialization factory not found");
    return factory_fnc_it->second;
}

const SlotFactory &GraphImpl::getSlotFactory(const SlotKey &key) const
{
    auto factory_fnc_it = slot_factories_.find(key);
    if (factory_fnc_it == slot_factories_.end())
        throw std::out_of_range("slot factory not found");
    return factory_fnc_it->second;
}
const SlotDeserializationFactory &GraphImpl::getSlotDeserFactory(const SlotKey &key) const
{
    auto factory_fnc_it = slot_deser_factories_.find(key);
    if (factory_fnc_it == slot_deser_factories_.end())
        throw std::out_of_range("slot deserialization factory not found");
    return factory_fnc_it->second;
}

void GraphImpl::createNode(const NodeKey &key, int preferred_x, int preferred_y, bool screen_space)
{
    auto node = getNodeFactory(key)(*this);
    node->init(*this);
    addNode(node);
    node->setPosition(preferred_x, preferred_y, screen_space);
}

void GraphImpl::addNode(const NodePtr &node)
{
    nodes_.emplace(node->id(), node);
    const auto node_vertex = addVertex(0, node->id(), -1, VertexType::node);

    for (auto &slot : node->inputs())
        addSlot(node, node_vertex, slot.second, SlotType::input);

    for (auto &slot : node->outputs())
        addSlot(node, node_vertex, slot.second, SlotType::output);

    // -1 is reserved for the both flow slots!
    //addSlot(node, node_vertex, node->inputByLocalId(-1), SlotType::input);
    //addSlot(node, node_vertex, node->outputByLocalId(-1), SlotType::output);
}

void GraphImpl::removeNode(const NodeId id)
{
    auto node_it = nodes_.find(id);
    if (node_it == nodes_.end())
        return;

    try
    {
        // remove node
        auto node_vertex = findVertexById(id);
        boost::clear_vertex(node_vertex, graph_);
    }
    catch (const std::out_of_range &)
    {
        //! \todo add logger
    }

    removeNodeSlots(node_it->second->inputs());
    removeNodeSlots(node_it->second->outputs());

    nodes_.erase(node_it);
}

VertexDesc GraphImpl::addSlot(const NodePtr &node, const VertexDesc node_vert, const SlotPtr &slot, const SlotType type)
{
    return addVertex(
        node_vert, slot->id(), node->id(), type == SlotType::input ? VertexType::input : VertexType::output);
}

void GraphImpl::removeSlot(const SlotId slot_id)
{
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end, next;
    boost::tie(vi, vi_end) = boost::vertices(graph_);
    for (next = vi; vi != vi_end; vi = next)
    {
        ++next;
        if (graph_[*vi].id == slot_id)
        {
            if (graph_[*vi].type == VertexType::input)
            {
                const auto in_edges = boost::in_edges(*vi, graph_);
                for (auto it = in_edges.first; it != in_edges.second; it++)
                {
                    const auto &edge_prop = boost::get(EdgeInfo_t(), graph_, *it);
                    edge_prop.connection->connection.disconnect();
                }
            }
            else if (graph_[*vi].type == VertexType::output)
            {
                const auto out_edges = boost::out_edges(*vi, graph_);
                for (auto it = out_edges.first; it != out_edges.second; it++)
                {
                    const auto &edge_prop = boost::get(EdgeInfo_t(), graph_, *it);
                    edge_prop.connection->connection.disconnect();
                }
            }
            boost::clear_vertex(*vi, graph_);
        }
    }
}

VertexDesc GraphImpl::addVertex(const VertexDesc node_desc, const int id, const int parent_id, VertexType type)
{
    VertexInfo info{id, parent_id, type};
    const auto vertex_desc = boost::add_vertex(std::move(info), graph_);
    if (type != VertexType::node)
    {
        EdgeInfo edge_info{link_id_counter_++, nullptr};
        if (type == VertexType::input)
            boost::add_edge(vertex_desc, node_desc, std::move(edge_info), graph_);
        else if (type == VertexType::output)
            boost::add_edge(node_desc, vertex_desc, std::move(edge_info), graph_);
    }
    return vertex_desc;
}

void GraphImpl::addEdge(const VertexDesc from, const VertexDesc to)
{
    assert(("from needs to be an output", graph_[from].type == VertexType::output));
    assert(("to needs to be an input", graph_[to].type == VertexType::input));
    assert(("from parent isn't set", graph_[from].parent_id >= 0));
    assert(("to parent isn't set", graph_[to].parent_id >= 0));

    auto from_node = nodes_.find(graph_[from].parent_id);
    if (from_node == nodes_.end())
        assert("from parent not set correctly");

    auto to_node = nodes_.find(graph_[to].parent_id);
    if (to_node == nodes_.end())
        assert("to parent not set correctly");

    auto output_slot = from_node->second->outputs(graph_[from].id);
    if (!output_slot)
        assert("output is null. so id isn't correctly set");

    auto input_slot = to_node->second->inputs(graph_[to].id);
    if (!input_slot)
        assert("input is null. so id isn't correctly set");

    if (!output_slot->canConnectTo(input_slot->key()))
        return;

    auto connection = output_slot->connectTo(input_slot);

    const EdgeInfo egde_prop{link_id_counter_++, std::make_shared<RefCon>(std::move(connection))};
    boost::add_edge(from, to, std::move(egde_prop), graph_);
}

void GraphImpl::removeEdge(const EdgeId id)
{
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = boost::vertices(graph_); vi != vi_end; ++vi)
    {
        if (graph_[*vi].type == VertexType::output)
        {
            auto edges = boost::out_edges(*vi, graph_);
            for (auto eeit = edges.first; eeit != edges.second; ++eeit)
            {
                const auto &edge_prop = boost::get(EdgeInfo_t(), graph_, *eeit);
                if (edge_prop.id == id)
                {
                    edge_prop.connection->connection.disconnect();
                    boost::remove_edge(*eeit, graph_);
                    break;
                }
            }
        }
    }
}

VertexDesc GraphImpl::findVertexById(const NodeId id) const
{
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = boost::vertices(graph_); vi != vi_end; ++vi)
    {
        if (graph_[*vi].id == id)
            return *vi;
    }
    throw std::out_of_range("vertex with id not found");
}

void GraphImpl::removeNodeSlots(const SlotMap &slots)
{
    for (const auto &slot : slots)
    {
        removeSlot(slot.second->id());
    }
}

SlotPtr GraphImpl::findSlotById(const SlotId id) const
{
    try
    {
        const auto slot_desc = findVertexById(id);
        assert(("id is not an slot id", graph_[slot_desc].type != VertexType::node));
        assert(("parent isn't set", graph_[slot_desc].parent_id >= 0));
        if (auto nit = nodes_.find(graph_[slot_desc].parent_id); nit != nodes_.end())
        {
            if (graph_[slot_desc].type == VertexType::input)
                return nit->second->inputs(id);
            else if (graph_[slot_desc].type == VertexType::output)
                return nit->second->outputs(id);
        }
    }
    catch (...)
    {
        //! \todo log
    }
    return nullptr;
}

NodePtr GraphImpl::findNodeById(const NodeId node_id) const
{
    auto it = nodes_.find(node_id);
    return it != nodes_.end() ? it->second : nullptr;
}

void GraphImpl::renderNodes()
{
    for (auto &node : nodes_)
    {
        node.second->render();
    }
}

void GraphImpl::renderLinks()
{
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = boost::vertices(graph_); vi != vi_end; ++vi)
    {
        if (graph_[*vi].type == VertexType::output)
        {
            const auto output_it = boost::out_edges(*vi, graph_);
            for (auto eeit = output_it.first; eeit != output_it.second; ++eeit)
            {
                const auto &edge_prop = boost::get(EdgeInfo_t(), graph_, *eeit);
                imnodes::Link(edge_prop.id, graph_[*vi].id, graph_[boost::target(*eeit, graph_)].id);
            }
        }
    }
}

void GraphImpl::save(const std::filesystem::path &file)
{
#if 0
    using json = nlohmann::json;

    json all_json;
    json nodes_json = json::array();
    for (const auto &node : nodes_)
    {
        //nodes_json.push_back(*node.second);
    }
    all_json["nodes"] = std::move(nodes_json);

    json edges_json = json::array();
    boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(graph_); ei != ei_end; ++ei)
    {
        const auto &source_info = graph_[boost::source(*ei, graph_)];
        const auto &target_info = graph_[boost::target(*ei, graph_)];
        // only save outside linkage
        if (source_info.type != VertexType::node && target_info.type != VertexType::node)
            edges_json.emplace_back(json{graph_[boost::source(*ei, graph_)].id, graph_[boost::target(*ei, graph_)].id});
    }
    all_json["links"] = std::move(edges_json);

    std::ofstream o(file);
    o << all_json << std::endl;
#endif
}

void GraphImpl::clearAndLoad(const std::filesystem::path &file)
{
#if 0
    using nlohmann::json;
    if (!std::filesystem::exists(file) || !std::filesystem::is_regular_file(file))
    {
        return;
    }
    clear();

    json j;
    {
        std::ifstream file_input{file};
        file_input >> j;
    }

    const json &node_arr = j["nodes"];
    for (const auto &node_j : node_arr)
    {
        auto node_factory = getNodeDeserializationFactory(node_j["key"]);
        addNode(node_factory(*this, node_j));
    }
    const json &link_arr = j["links"];
    for (const auto &link_j : link_arr)
    {
        if (link_j.size() == 2)
        {
            try
            {
                addEdge(findVertexById(link_j.at(0)), findVertexById(link_j.at(1)));
            }
            catch (...)
            {}
        }
    }
    for (const auto &link_j : link_arr)
    {
        if (link_j.size() == 2)
        {
            auto source = findSlotById(link_j.at(0));
        }
    }
    int highest_vertex_id = 0;
    for (const auto &node : nodes_)
    {
        if (node.second->id() > highest_vertex_id)
            highest_vertex_id = node.second->id();
        for (const auto &slot : node.second->inputs())
        {
            if (slot.second->id() > highest_vertex_id)
                highest_vertex_id = slot.second->id();
        }
        for (const auto &slot : node.second->outputs())
        {
            if (slot.second->id() > highest_vertex_id)
                highest_vertex_id = slot.second->id();
        }
    }
    vertex_id_counter_ = highest_vertex_id + 1;
#endif
}

void GraphImpl::clear()
{
    graph_.clear();
    nodes_.clear();
    link_id_counter_ = 0;
    vertex_id_counter_ = 0;
}

const NodeDisplayGraph &GraphImpl::nodeDisplayNames() const
{
    return node_display_names_;
}

GraphImpl::~GraphImpl()
{}
} // namespace dt::df::editor
