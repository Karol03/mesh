/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <string>


namespace mesh
{
namespace objects
{

struct Description
{
public:
    explicit Description() = default;
    explicit Description(std::string data)
        : m_data{data}
    {}

    Description(const Description&) = default;
    Description& operator=(const Description&) = default;
    Description(Description&&) = default;
    Description& operator=(Description&&) = default;

    std::string to_string() const
    {
        return m_data;
    }

private:
    std::string m_data;
};

}  // namespace objects
}  // namespace mesh
