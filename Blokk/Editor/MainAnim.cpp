#pragma once

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cstdint>

namespace BlokkLogo
{

struct Square
{
    float StartX;
    float StartY;
    float EndX;
    float EndY;
    std::uint32_t DurationMs;
};

// Recreated from the 13 visible cyan square elements in the supplied SVG.
// Coordinates are the SVG's 1000x1000 coordinate space.
constexpr std::array<Square, 13> Squares = {{
    {453.053265f,  45.0f, 453.053265f, 675.973369f,  500},
    {455.0f,  45.0f, 455.234250f, 500.026633f,  700},
    {543.053265f,  45.0f, 542.053265f, 675.973369f,  600},
    {543.053265f,  45.0f, 543.053265f, 500.026632f,  800},
    {455.0f,  44.999997f, 454.946735f, 320.026629f, 1000},
    {543.053265f,  44.999997f, 543.234250f, 319.973365f,  900},
    { 45.0f, 589.973366f, 366.234250f, 586.973365f,  800},
    { 45.0f, 409.973365f, 366.053265f, 408.973365f,  600},
    { 45.0f, 679.973366f, 366.234250f, 675.973365f,  900},
    { 45.0f, 499.973365f, 366.234250f, 497.973365f,  700},
    { 45.0f, 319.973365f, 366.234250f, 319.973365f,  500},
    {955.784862f, 589.973367f, 633.234250f, 590.026630f, 1200},
    {955.784862f, 409.973366f, 635.053265f, 410.026629f, 1000}
}};

class Animation
{
public:
    static constexpr float CanvasSize = 1000.0f;
    static constexpr float SquareSize = 90.0f;
    static constexpr std::uint32_t DurationMs = 1200;

    void Reset()
    {
        StartTime = SDL_GetTicks();
        Playing = true;
    }

    void Update()
    {
        if (!Playing)
        {
            return;
        }

        if (SDL_GetTicks() - StartTime >= DurationMs)
        {
            Playing = false;
        }
    }

    void Render(SDL_Renderer* Renderer) const
    {
        int Width = 0;
        int Height = 0;
        SDL_GetRenderOutputSize(Renderer, &Width, &Height);

        const float Scale = std::min(
            static_cast<float>(Width),
            static_cast<float>(Height)
        ) / CanvasSize;

        const float OffsetX =
            (static_cast<float>(Width) - CanvasSize * Scale) * 0.5f;

        const float OffsetY =
            (static_cast<float>(Height) - CanvasSize * Scale) * 0.5f;

        const std::uint64_t Elapsed = SDL_GetTicks() - StartTime;

        SDL_SetRenderDrawColor(Renderer, 4, 245, 255, 255);

        for (const Square& Item : Squares)
        {
            float X = Item.EndX;
            float Y = Item.EndY;

            if (Elapsed < Item.DurationMs)
            {
                const float T = static_cast<float>(Elapsed) /
                                static_cast<float>(Item.DurationMs);

                X = Item.StartX + (Item.EndX - Item.StartX) * T;
                Y = Item.StartY + (Item.EndY - Item.StartY) * T;
            }

            SDL_FRect Rect {
                OffsetX + (X - SquareSize * 0.5f) * Scale,
                OffsetY + (Y - SquareSize * 0.5f) * Scale,
                SquareSize * Scale,
                SquareSize * Scale
            };

            SDL_RenderFillRect(Renderer, &Rect);
        }
    }

    [[nodiscard]] inline
    bool IsPlaying() const
    {
        return Playing;
    }

    [[nodiscard]] inline
    bool IsFinished() const 
    {
        return !Playing;
    }

private:
    std::uint64_t StartTime = 0;
    bool Playing = false;
};

} // namespace BlokkLogo
