#pragma once

namespace Common
{
template <typename Key, typename Enum, Key maxEnum = Key::MAX_VALUE>
struct EnumMap
{
  private:
    static constexpr size_t arraySize = static_cast<size_t>(maxEnum) + 1;

  public:
    constexpr EnumMap(std::initializer_list<std::pair<Key, Enum>> InValues = {})
    {
        for (const auto& [key, value] : InValues)
        {
            data[static_cast<size_t>(key)] = value;
        }
    }

    constexpr Enum& operator[](const Key& InKey)
    {
        const auto index = static_cast<size_t>(InKey);
        assert(index < arraySize);
        return data[index];
    }

    constexpr const Enum& operator[](const Key& InKey) const
    {
        const auto index = static_cast<size_t>(InKey);
        assert(index < arraySize);
        return data[index];
    }

  private:
    std::array<Enum, arraySize> data{};
};
} // namespace Common
