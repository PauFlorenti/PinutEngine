#pragma once

namespace common
{
class Color final
{
  public:
    constexpr Color(u32 InColor) : data(InColor) {};
    constexpr Color(u8 InR, u8 InG, u8 InB, u8 InA = 255)
    {
        data = InA << alphaShiftValue;
        data |= InB << blueShiftValue;
        data |= InG << greenShiftValue;
        data |= InR << redShiftValue;
    };

    constexpr Color(const glm::vec3& InColor)
    {
        data = 255 << alphaShiftValue;
        data |= (static_cast<u32>(InColor.b * 255) << blueShiftValue);
        data |= (static_cast<u32>(InColor.g * 255) << greenShiftValue);
        data |= (static_cast<u32>(InColor.r * 255) << redShiftValue);
    };

    constexpr Color(const glm::vec4& InColor)
    {
        data = (static_cast<u32>(InColor.a * 255) << alphaShiftValue);
        data |= (static_cast<u32>(InColor.b * 255) << blueShiftValue);
        data |= (static_cast<u32>(InColor.g * 255) << greenShiftValue);
        data |= (static_cast<u32>(InColor.r * 255) << redShiftValue);
    }

    constexpr u32 RGBA() const { return data; }

    constexpr u8 R() const { return (data & redMaskValue) >> redShiftValue; }
    constexpr u8 G() const { return (data & greenMaskValue) >> greenShiftValue; }
    constexpr u8 B() const { return (data & blueMaskValue) >> blueShiftValue; }
    constexpr u8 A() const { return (data & alphaMaskValue) >> alphaShiftValue; }

    constexpr float Rf() const { return static_cast<float>(R()) / 255.f; }
    constexpr float Gf() const { return static_cast<float>(G()) / 255.f; }
    constexpr float Bf() const { return static_cast<float>(B()) / 255.f; }
    constexpr float Af() const { return static_cast<float>(A()) / 255.f; }

    static constexpr Color White() { return Color(255, 255, 255, 255); }
    static constexpr Color Black() { return Color(0, 0, 0, 255); }
    static constexpr Color Red() { return Color(255, 0, 0, 255); }
    static constexpr Color Green() { return Color(0, 255, 0, 255); }
    static constexpr Color Blue() { return Color(0, 0, 255, 255); }

  private:
    u32 data;

    // 0xAABBGGRR
    static constexpr u8 redShiftValue   = 0;
    static constexpr u8 greenShiftValue = 8;
    static constexpr u8 blueShiftValue  = 16;
    static constexpr u8 alphaShiftValue = 24;

    static constexpr u32 redMaskValue   = 0x000000FF;
    static constexpr u32 greenMaskValue = 0x0000FF00;
    static constexpr u32 blueMaskValue  = 0x00FF0000;
    static constexpr u32 alphaMaskValue = 0xFF000000;
};

static constexpr u32 WhiteRGBA = Color::White().RGBA();
static constexpr u32 BlackRGBA = Color::Black().RGBA();
static constexpr u32 RedRGBA   = Color::Red().RGBA();
static constexpr u32 GreenRGBA = Color::Green().RGBA();
static constexpr u32 BlueRGBA  = Color::Blue().RGBA();
} // namespace common
