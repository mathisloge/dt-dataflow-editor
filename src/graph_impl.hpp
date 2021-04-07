#pragma once
#include <atomic>
#include <filesystem>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>
#include <Corrade/PluginManager/Manager.h>

#include <dt/df/core/graph_manager.hpp>

#include <dt/df/plugin/plugin.hpp>
#include "bounded_buffer.hpp"
#include "node_display_tree.hpp"
#include "priv_types.hpp"
namespace dt::df::editor
{
class GraphImpl final : public IGraphManager
{
  public:
    GraphImpl();
    void init();
    void registerNodeFactory(const NodeKey &key,
                             const std::string &node_display_name,
                             NodeFactory &&factory,
                             NodeDeserializationFactory &&deser_factory) override;
    void registerSlotFactory(const SlotKey &key,
                             SlotFactory &&factory,
                             SlotDeserializationFactory &&deser_factory) override;
    const SlotFactory &getSlotFactory(const SlotKey &key) const override;
    const SlotDeserializationFactory &getSlotDeserFactory(const SlotKey &key) const override;

    NodeId generateNodeId() override;
    SlotId generateSlotId() override;
    bool registerSlot(const NodeId node_id, const SlotId slot_id, const SlotType type) override;
    bool unregisterSlot(const NodeId node_id, const SlotId slot_id) override;

    void createNode(const NodeKey &key, int preferred_x, int preferred_y, bool screen_space);
    void removeNode(const NodeId id);
    void addEdge(const VertexDesc from, const VertexDesc to);
    void removeEdge(const EdgeId id);
    VertexDesc findVertexById(const NodeId id) const;

    void renderNodes();
    void renderLinks();

    void save(const std::filesystem::path &file);
    void clearAndLoad(const std::filesystem::path &file);
    void clear();
    const NodeDisplayGraph &nodeDisplayNames() const;
    ~GraphImpl();

  private:
    void addNode(const NodePtr &node);
    VertexDesc addSlot(const NodePtr &node, const VertexDesc node_vert, const SlotPtr &slot, const SlotType type);
    void removeSlot(const SlotId slot_id);
    const NodeFactory &getNodeFactory(const NodeKey &key) const;
    const NodeDeserializationFactory &getNodeDeserializationFactory(const NodeKey &key) const;
    VertexDesc addVertex(const VertexDesc node_desc, const int id, const int parent_id, VertexType type);
    void removeNodeSlots(const Slots &slots);
    SlotPtr findSlotById(const SlotId) const;
    NodePtr findNodeById(const NodeId) const;

  private:
    Corrade::PluginManager::Manager<plugin::Plugin> manager_;
    std::vector<std::unique_ptr<plugin::Plugin>> loaded_plugins_;
    Graph graph_;
    std::atomic_int link_id_counter_;
    std::atomic_int vertex_id_counter_;
    std::unordered_map<NodeKey, NodeFactory> node_factories_;
    std::unordered_map<NodeKey, NodeDeserializationFactory> node_deser_factories_;
    std::unordered_map<SlotKey, SlotFactory> slot_factories_;
    std::unordered_map<SlotKey, SlotDeserializationFactory> slot_deser_factories_;
    NodeDisplayGraph node_display_names_;
    std::unordered_map<NodeId, NodePtr> nodes_;
};
} // namespace dt::df::editor
