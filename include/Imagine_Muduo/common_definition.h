#ifndef IMAGINE_MUDUO_COMMON_DEFINITION_H
#define IMAGINE_MUDUO_COMMON_DEFINITION_H

#include <functional>

namespace Imagine_Muduo
{

class HashPair
{
 public:
    template <typename first, typename second>
    size_t operator()(const std::pair<first, second> &p) const
    {
        auto hash1 = std::hash<first>()(p.first);
        auto hash2 = std::hash<second>()(p.second);
        return hash1 ^ hash2;
    }
};

class EqualPair
{
 public:
    template <typename first, typename second>
    bool operator()(const std::pair<first, second> &a, const std::pair<first, second> &b) const
    {
        return a.first == b.first && a.second == b.second;
    }
};

} // namespace Imagine_Muduo

#endif