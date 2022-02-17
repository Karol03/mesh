/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <functional>
#include <vector>

#include "objects/edge.hpp"
#include "objects/imobject.hpp"
#include "objects/node.hpp"
#include "objects/types.hpp"


namespace mesh
{
template <typename Description>
class MeshBuilder;

namespace utils
{
template <typename Description>
class MeshPack;
}  // namespace utils

template <typename Description>
struct Mesh
{
    friend class MeshBuilder<Description>;
    friend class utils::MeshPack<Description>;

    using U32PairMap = objects::types::U32PairMap;
    using U32EdgeMap = objects::types::U32EdgeMap<Description>;
    using U32NodeMap = objects::types::U32NodeMap<Description>;
    using U32Pair = objects::types::U32Pair;
    using U32PairPriorityQueue = objects::types::U32PairPriorityQueue;
    using U32Set = objects::types::U32Set;

public:
    explicit Mesh()
        : m_nodes{}
        , m_edges{}
        , m_current{}
    {}

    void attach(Description nodeDescription = Description{},
                Description edgeDescription = Description{})
    {
        if (m_current == 0)
        {
            auto node = objects::Node{std::move(nodeDescription)};
            auto nodeId = nodeIdGenerator();
            m_current = nodeId;
            m_nodes.insert({nodeId, std::move(node)});
        }
        else
        {
            auto node = objects::Node{std::move(nodeDescription)};
            auto edge = objects::Edge{std::move(edgeDescription)};
            auto nodeId = nodeIdGenerator();
            auto edgeId = edgeIdGenerator();

            m_nodes[m_current].edges().insert(edgeId);
            node.edges().insert(edgeId);
            edge.nodes().first = m_current;
            edge.nodes().second = nodeId;

            m_current = nodeId;

            m_nodes.insert({nodeId, std::move(node)});
            m_edges.insert({edgeId, std::move(edge)});
        }
    }

    void tie(uint32_t leftNodeId, uint32_t rightNodeId,
             Description edgeDescription = Description{})
    {
        if (!contains(m_nodes, leftNodeId) ||
            !contains(m_nodes, rightNodeId))
        {
            return;
        }

        auto& leftNodeEdges = m_nodes[leftNodeId].edges();
        auto& rightNodeEdges = m_nodes[rightNodeId].edges();
        if (isIntersection(leftNodeEdges, rightNodeEdges))
        {
            return;
        }

        auto edge = objects::Edge{std::move(edgeDescription)};
        auto edgeId = edgeIdGenerator();
        edge.nodes().first = leftNodeId;
        edge.nodes().second = rightNodeId;
        leftNodeEdges.insert(edgeId);
        rightNodeEdges.insert(edgeId);
        m_edges.insert({edgeId, std::move(edge)});
    }

    void tie(uint32_t leftNodeId,
             std::function<bool(const objects::Node<Description>&)> rightNodePredicate,
             Description edgeDescription = Description{})
    {
        if (!contains(m_nodes, leftNodeId))
        {
            return;
        }

        auto rightNodeId = 0u;
        for (const auto& item : m_nodes)
        {
            if (rightNodePredicate(item.second))
            {
                rightNodeId = item.first;
                if (leftNodeId == rightNodeId)
                {
                    return;
                }
                tie(leftNodeId, rightNodeId, std::move(edgeDescription));
            }
        }
    }

    void tie(std::function<bool(const objects::Node<Description>&)> leftNodePredicate,
             std::function<bool(const objects::Node<Description>&)> rightNodePredicate,
             Description edgeDescription = Description{})
    {
        auto leftNodeId = 0u;
        auto rightNodeId = 0u;

        for (const auto& item : m_nodes)
        {
            if (leftNodePredicate(item.second))
            {
                leftNodeId = item.first;
                if (leftNodeId != 0 && rightNodeId != 0)
                {
                    if (leftNodeId == rightNodeId)
                    {
                        return;
                    }
                    tie(leftNodeId, rightNodeId, std::move(edgeDescription));
                }
            }
            else if (rightNodePredicate(item.second))
            {
                rightNodeId = item.first;
                if (leftNodeId != 0 && rightNodeId != 0)
                {
                    if (leftNodeId == rightNodeId)
                    {
                        return;
                    }
                    tie(leftNodeId, rightNodeId, std::move(edgeDescription));
                }
            }
        }
    }

    void detach()
    {
        if (m_current != 0)
        {
            detach(m_current);
        }
    }

    void detach(uint32_t id)
    {
        const auto nodeItemIt = m_nodes.find(id);
        if (nodeItemIt == m_nodes.end())
        {
            return;
        }

        const auto itemEdgeIds = nodeItemIt->second.edges();
        if (itemEdgeIds.empty())
        {
            m_nodes.erase(id);
        }
        else if (itemEdgeIds.size() == 1)
        {
            const auto edgeId = *itemEdgeIds.begin();
            auto nodeFirst = m_edges[edgeId].nodes().first;
            auto nodeSecond = m_edges[edgeId].nodes().second;

            m_nodes[nodeFirst].edges().erase(edgeId);
            m_nodes[nodeSecond].edges().erase(edgeId);
            m_edges.erase(edgeId);
            m_nodes.erase(id);
        }
        else
        {
            auto relatedNodes = U32Set{};
            for (const auto edgeId : itemEdgeIds)
            {
                auto nodeFirst = m_edges[edgeId].nodes().first;
                auto nodeSecond = m_edges[edgeId].nodes().second;

                m_nodes[nodeFirst].edges().erase(edgeId);
                m_nodes[nodeSecond].edges().erase(edgeId);
                m_edges.erase(edgeId);
                m_nodes.erase(id);

                if (nodeFirst == id)
                {
                    relatedNodes.insert(nodeSecond);
                }
                else
                {
                    relatedNodes.insert(nodeFirst);
                }
            }
            rebranch(std::move(relatedNodes));
        }

        if (m_current == id)
        {
            m_current = 0;
        }
        else if (m_nodes.find(m_current) == m_nodes.end())
        {
            m_current = 0;
        }
    }

    void detach(std::vector<uint32_t> range)
    {
        for (const auto& id : range)
        {
            detach(id);
        }
    }

    void detach(std::function<bool(const objects::Node<Description>&)> predicate)
    {
        auto detachRange = std::vector<uint32_t>{};
        for (const auto& item : m_nodes)
        {
            if (predicate(item.second))
            {
                detachRange.push_back(item.first);
            }
        }
        detach(std::move(detachRange));
    }

    void visit(std::function<void(const objects::Node<Description>&)> nodePredicate,
               std::function<void(const objects::Edge<Description>&)> edgePredicate) const
    {
        for (const auto& node : m_nodes)
        {
            nodePredicate(node.second);
        }

        for (const auto& edge : m_edges)
        {
            edgePredicate(edge.second);
        }
    }

    void clear()
    {
        m_current = 0;
        m_nodes.clear();
        m_edges.clear();
    }

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

    bool isIntersection(const U32Set& lhs, const U32Set& rhs)
    {
        return intersectionPoint(lhs, rhs) != 0;
    }

    uint32_t intersectionPoint(const U32Set& lhs, const U32Set& rhs)
    {
        if (lhs.size() < rhs.size())
        {
            for (const auto& x : lhs)
            {
                if (rhs.find(x) != rhs.end())
                {
                    return x;
                }
            }
        }
        else
        {
            for (const auto& x : rhs)
            {
                if (lhs.find(x) != lhs.end())
                {
                    return x;
                }
            }
        }
        return 0;
    }

    std::pair<U32Set, U32Set> dfs(uint32_t nodeId)
    {
        auto visitedNodes = U32Set{};
        auto visitedEdges = U32Set{};

        auto toVisit = std::vector<uint32_t>{nodeId};
        while (!toVisit.empty())
        {
            const auto nodeId = toVisit.back();
            toVisit.pop_back();
            visitedNodes.insert(nodeId);

            for (const auto edgeId : m_nodes[nodeId].edges())
            {
                if (contains(visitedEdges, edgeId))
                {
                    continue;
                }
                visitedEdges.insert(edgeId);

                auto nextNode = m_edges[edgeId].nodes().first == nodeId ?
                                m_edges[edgeId].nodes().second :
                                m_edges[edgeId].nodes().first;

                if (!contains(visitedNodes, nextNode))
                {
                    toVisit.push_back(nextNode);
                }
            }
        }

        return {visitedNodes, visitedEdges};
    }

    std::pair<bool, int> bidirectionalAStart(const uint32_t leftBranchRoot,
                                             const uint32_t rightBranchRoot,
                                             U32Set& leaves)
    {
        constexpr auto pathExist = true;
        constexpr auto pathNotExist = false;
        constexpr auto BOTH_BRANCHES_EQUAL = 0;
        constexpr auto LEFT_BRANCH_IS_BIGGER = -1;
        constexpr auto RIGHT_BRANCH_IS_BIGGER = 1;

        auto visitedBegin = U32Set{};
        auto visitedEnd = U32Set{};

        auto pQueueLeftBranch = U32PairPriorityQueue{};
        auto pQueueRightBranch = U32PairPriorityQueue{};

        pQueueLeftBranch.push({0, leftBranchRoot});
        pQueueRightBranch.push({0, rightBranchRoot});

        while (!pQueueLeftBranch.empty() && !pQueueRightBranch.empty())
        {
            auto [bPrio, begin] = pQueueLeftBranch.top();
            pQueueLeftBranch.pop();
            auto [ePrio, end] = pQueueRightBranch.top();
            pQueueRightBranch.pop();

            visitedBegin.insert(begin);
            visitedEnd.insert(end);

            if (contains(leaves, begin))
            {
                leaves.erase(begin);
            }

            if (contains(leaves, end))
            {
                leaves.erase(end);
            }

            if (isIntersection(visitedBegin, visitedEnd))
            {
                return {pathExist, BOTH_BRANCHES_EQUAL};
            }

            for (const auto bConnect : getConnectedNodes(begin))
            {
                if (!contains(visitedBegin, bConnect))
                {
                    pQueueLeftBranch.push({bPrio + 1, bConnect});
                }
            }

            for (const auto eConnect : getConnectedNodes(end))
            {
                if (!contains(visitedEnd, eConnect))
                {
                    pQueueRightBranch.push({ePrio + 1, eConnect});
                }
            }
        }

        if (pQueueLeftBranch.empty())
        {
            return {pathNotExist, RIGHT_BRANCH_IS_BIGGER};
        }
        else
        {
            return {pathNotExist, LEFT_BRANCH_IS_BIGGER};
        }
    }

    std::vector<uint32_t> bidirectionalAStart(const uint32_t leftBranchRoot,
                                              const uint32_t rightBranchRoot)
    {
        auto visitedBegin = U32Set{};
        auto visitedEnd = U32Set{};
        auto nodeToParentMapBegin = U32PairMap{};
        auto nodeToParentMapEnd = U32PairMap{};

        auto pQueueLeftBranch = U32PairPriorityQueue{};
        auto pQueueRightBranch = U32PairPriorityQueue{};

        pQueueLeftBranch.push({0, leftBranchRoot});
        pQueueRightBranch.push({0, rightBranchRoot});

        while (!pQueueLeftBranch.empty() && !pQueueRightBranch.empty())
        {
            auto [bPrio, begin] = pQueueLeftBranch.top();
            pQueueLeftBranch.pop();
            auto [ePrio, end] = pQueueRightBranch.top();
            pQueueRightBranch.pop();

            visitedBegin.insert(begin);
            visitedEnd.insert(end);

            auto commonNode = intersectionPoint(visitedBegin, visitedEnd);
            if (commonNode != 0)
            {
                auto result = std::vector<uint32_t>{};

                while (nodeToParentMapBegin[commonNode] != 0)
                {
                    result.push_back(nodeToParentMapBegin[begin]);
                }

                result = std::vector(result.rbegin(), result.rend());

                while (nodeToParentMapBegin[commonNode] != 0)
                {
                    result.push_back(nodeToParentMapBegin[begin]);
                }

                return result;
            }

            for (const auto bConnect : getConnectedNodes(begin))
            {
                if (!contains(visitedBegin, bConnect))
                {
                    pQueueLeftBranch.push({bPrio + 1, bConnect});
                    nodeToParentMapBegin[bConnect] = begin;
                }
            }

            for (const auto eConnect : getConnectedNodes(end))
            {
                if (!contains(visitedEnd, eConnect))
                {
                    pQueueRightBranch.push({ePrio + 1, eConnect});
                    nodeToParentMapEnd[eConnect] = begin;
                }
            }
        }

        return {};
    }

    std::vector<uint32_t> getConnectedNodes(uint32_t nodeId)
    {
        auto result = std::vector<uint32_t>{};
        result.reserve(m_nodes[nodeId].edges().size());

        for (const auto edgeId : m_nodes[nodeId].edges())
        {
            if (m_edges[edgeId].nodes().first == nodeId)
            {
                result.push_back(m_edges[edgeId].nodes().second);
            }
            else
            {
                result.push_back(m_edges[edgeId].nodes().first);
            }
        }

        return result;
    }

    void deleteBranch(uint32_t nodeId, U32Set& leaves)
    {
        auto [visitedNodes, visitedEdges] = dfs(nodeId);

        for (const auto edgeId : visitedEdges)
        {
            m_edges.erase(edgeId);
        }

        for (const auto nodeId : visitedNodes)
        {
            m_nodes.erase(nodeId);
            leaves.erase(nodeId);
        }
    }

    void rebranch(U32Set leaves)
    {
        auto leftBranch = *leaves.begin();
        leaves.erase(leaves.begin());

        while (!leaves.empty())
        {
            auto rightBranch = *leaves.begin();
            leaves.erase(leaves.begin());

            const auto [isPathExist, biggerBranch] = bidirectionalAStart(leftBranch, rightBranch, leaves);
            if (!isPathExist)
            {
                constexpr auto LEFT_BRANCH_NO = -1;
                if (biggerBranch != LEFT_BRANCH_NO)
                {
                    std::swap(leftBranch, rightBranch);
                }
                deleteBranch(rightBranch, leaves);
            }
        }
    }

    uint32_t insertNode(Description description)
    {
        auto node = objects::Node{std::move(description)};
        auto nodeId = nodeIdGenerator();
        m_nodes.insert({nodeId, std::move(node)});
        return nodeId;
    }

    uint32_t insertEdge(U32Pair endpointNodes, Description description)
    {
        auto edge = objects::Edge{std::move(description)};
        auto edgeId = edgeIdGenerator();
        edge.nodes() = endpointNodes;
        m_edges.insert({edgeId, std::move(edge)});
        return edgeId;
    }

private:
    U32NodeMap m_nodes;
    U32EdgeMap m_edges;
    uint32_t m_current;
};

}  // namespace mesh
