/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <unordered_map>
#include <queue>

#include "node.hpp"
#include "edge.hpp"


namespace mesh
{
namespace objects
{
namespace types
{

using U32PairMap = std::unordered_map<uint32_t, uint32_t>;

template <typename Description>
using U32EdgeMap = std::unordered_map<uint32_t, Edge<Description>>;

template <typename Description>
using U32NodeMap = std::unordered_map<uint32_t, Node<Description>>;

using U32Pair = IMObject::U32Pair;
using U32U32Map = std::unordered_map<uint32_t, uint32_t>;
using U32PairPriorityQueue = std::priority_queue<U32Pair, std::vector<U32Pair>, std::greater<U32Pair>>;
using U32Set = IMObject::U32Set;

}  // namespace types
}  // namespace objects
}  // namespace mesh
