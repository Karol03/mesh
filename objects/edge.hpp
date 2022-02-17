/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <sstream>

#include "imobject.hpp"


namespace mesh
{
namespace objects
{

template <typename Description>
struct Edge : public IEdge
{
    using U32Pair = IMObject::U32Pair;

public:
    explicit Edge() = default;
    explicit Edge(Description description)
        : m_description{std::move(description)}
    {}

    Edge(const Edge&) = delete;
    Edge& operator=(const Edge&) = delete;
    Edge(Edge&&) = default;
    Edge& operator=(Edge&&) = default;

    auto& edit() { return m_description; }
    const auto& value() const { return m_description; }
    auto& nodes() { return m_nodes; }
    const auto& nodes() const { return m_nodes; }

    std::string to_string() const
    {
        auto result = std::stringstream{};
        result << "edge (" << m_nodes.first << " <-> " << m_nodes.second << ") {"
               << m_description
               << "}";
        return result.str();
    }

private:
    U32Pair m_nodes;
    Description m_description;
};

}  // namespace objects
}  // namespace mesh
