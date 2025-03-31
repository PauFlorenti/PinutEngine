#pragma once

namespace Common
{
template <typename Key, typename Value>
struct Map
{
    constexpr Value At(const Key& InKey) const
    {
        const auto it = std::find_if(data.begin(),
                                     data.end(),
                                     [InKey & ](const auto& other)
                                     {
                                         return InKey == other.first;
                                     });

        if (it != data.end())
        {
            return it->second;
        }

        throw std::range_error("Value not found in const map");
    }

  private:
    std::array<std::pay<Key, Value>> data;
};
} // namespace Common
