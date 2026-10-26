#pragma once

#include <cstdint>

enum class LightColor : std::uint8_t
{
    None = 0,
    Red = 1 << 0,
    Green = 1 << 1,
    Blue = 1 << 2,
    Yellow = Red | Green,
    Magenta = Red | Blue,
    Cyan = Green | Blue,
    White = Red | Green | Blue,
};

constexpr LightColor operator|(LightColor a, LightColor b)
{
    return static_cast<LightColor>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}

constexpr LightColor operator&(LightColor a, LightColor b)
{
    return static_cast<LightColor>(
        static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b));
}

constexpr LightColor invert(LightColor c)
{
    // 1) Convert Color to unsigned int (e.g. Color::Red = 0b001U)
    // 2) ~ (bitwise NOT) flips every bit (e.g. 0b111..110U)
    // 3) & (bitwise mask) keeps only the 3 bits we care about
    return static_cast<LightColor>(~static_cast<unsigned>(c) & 0b111U);
}

// Compile-time tests..
static_assert((LightColor::Red | LightColor::Green) == LightColor::Yellow);
static_assert((LightColor::Red | LightColor::Blue) == LightColor::Magenta);
static_assert((LightColor::Green | LightColor::Blue) == LightColor::Cyan);
static_assert((LightColor::Blue | LightColor::Yellow) == LightColor::White);
static_assert((LightColor::Red & LightColor::Green) == LightColor::None);
static_assert((LightColor::Cyan & LightColor::Blue) == LightColor::Blue);
static_assert(invert(LightColor::Blue) == LightColor::Yellow);
static_assert(invert(LightColor::Red) == LightColor::Cyan);
static_assert(invert(LightColor::Green) == LightColor::Magenta);
