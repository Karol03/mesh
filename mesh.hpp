/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <functional>
#include <vector>

#include "objects/description.hpp"
#include "objects/edge.hpp"
#include "objects/imobject.hpp"
#include "objects/node.hpp"
#include "objects/types.hpp"


namespace mesh
{
class MeshBuilder;

namespace utils
{
class MeshPack;
}  // namespace utils

struct Mesh
{
    friend class MeshBuilder;
    friend class utils::MeshPack;

    using U32EdgeMap = objects::types::U32EdgeMap;
    using U32NodeMap = objects::types::U32NodeMap;
    using U32Pair = objects::types::U32Pair;
    using U32PairPriorityQueue = objects::types::U32PairPriorityQueue;
    using U32Set = objects::types::U32Set;

public:
    explicit Mesh();

    void attach(objects::Description nodeDescription = objects::Description{},
                objects::Description edgeDescription = objects::Description{});

    void tie(uint32_t leftNodeId, uint32_t rightNodeId,
             objects::Description edgeDescription = objects::Description{});
    void tie(uint32_t leftNodeId, std::function<bool(const objects::Node&)> rightNodePredicate,
             objects::Description edgeDescription = objects::Description{});
    void tie(std::function<bool(const objects::Node&)> leftNodePredicate,
             std::function<bool(const objects::Node&)> rightNodePredicate,
             objects::Description edgeDescription = objects::Description{});

    void detach();
    void detach(uint32_t id);
    void detach(std::vector<uint32_t> range);
    void detach(std::function<bool(const objects::Node&)> predicate);

    void visit(std::function<void(const objects::Node&)> nodePredicate,
               std::function<void(const objects::Edge&)> edgePredicate) const;

    void clear();

private:
    template <typename Container, typename T>
    bool contains(const Container& container, const T& element)
    {
        return container.find(element) != container.end();
    }

    static uint32_t nodeIdGenerator()
    {
        static uint32_t id = 0;
        return ++id;
    }

    static uint32_t edgeIdGenerator()
    {
        static uint32_t id = 0;
        return ++id;
    }

    bool isIntersection(const U32Set& lhs, const U32Set& rhs);
    std::pair<U32Set, U32Set> dfs(uint32_t nodeId);
    std::pair<bool, int> bidirectionalAStart(const uint32_t leftBranchRoot,
                                             const uint32_t rightBranchRoot,
                                             U32Set& leaves);
    std::vector<uint32_t> getConnectedNodes(uint32_t nodeId);
    void deleteBranch(uint32_t nodeId, U32Set& leaves);
    void rebranch(U32Set leaves);
    uint32_t insertNode(objects::Description description);
    uint32_t insertEdge(U32Pair endpointNodes, objects::Description description);

private:
    U32NodeMap m_nodes;
    U32EdgeMap m_edges;
    uint32_t m_current;
};

}  // namespace mesh
