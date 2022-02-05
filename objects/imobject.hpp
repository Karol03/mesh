/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <inttypes.h>
#include <memory>
#include <unordered_set>


namespace mesh
{
namespace objects
{

struct IMObject : public std::enable_shared_from_this<IMObject>
{
public:
    using U32Pair = std::pair<uint32_t, uint32_t>;
    using U32Set = std::unordered_set<uint32_t>;

protected:
    explicit IMObject() = default;

public:
    virtual ~IMObject() = default;
};

struct INode : public IMObject {};
struct IEdge : public IMObject {};

}  // namespace objects
}  // namespace mesh
