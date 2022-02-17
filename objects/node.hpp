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
struct Node : public INode
{
    using U32Set = IMObject::U32Set;

public:
    explicit Node() = default;
    explicit Node(Description description)
        : m_description{std::move(description)}
    {}

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&) = default;
    Node& operator=(Node&&) = default;

    auto& edit() { return m_description; }
    const auto& value() const { return m_description; }
    auto& edges() { return m_edges; }
    const auto& edges() const { return m_edges; }

    std::string to_string() const
    {
        auto result = std::stringstream{};
        result << "node ";

        if (m_edges.empty())
        {
            result << "[]";
        }
        else
        {
            auto cbegin = m_edges.cbegin();
            result << "[" << *cbegin;
            for (++cbegin; cbegin != m_edges.cend(); ++cbegin)
            {
                result << ", " << *cbegin;
            }
            result << "]";
        }
        result << " {"
               << m_description
               << "}";
        return result.str();
    }

private:
    U32Set m_edges;
    Description m_description;
};

}  // namespace objects
}  // namespace mesh
