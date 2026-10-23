#include "EngineClassData.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>


namespace Blokk
{

#ifdef Blokk_Rendering_Enabled

enum class InitFlags : uint32_t
{
    None    = 0,
    Audio   = SDL_INIT_AUDIO,
    Video   = SDL_INIT_VIDEO,
    Joystick = SDL_INIT_JOYSTICK,
    Haptic  = SDL_INIT_HAPTIC,
    Gamepad = SDL_INIT_GAMEPAD,
    Events  = SDL_INIT_EVENTS,
    Sensor  = SDL_INIT_SENSOR
};

constexpr InitFlags operator|(
    InitFlags A,
    InitFlags B
)
{
    return static_cast<InitFlags>(
        static_cast<uint32_t>(A) |
        static_cast<uint32_t>(B)
    );
}

void Init(InitFlags Flags)
{
    if (!SDL_Init(static_cast<SDL_InitFlags>(Flags)))
    {
        throw std::runtime_error(
            std::string("Failed to initialize SDL: ") +
            SDL_GetError()
        );
    }
}

enum class WindowFlags : uint32_t
{
    None        = 0,
    Fullscreen  = SDL_WINDOW_FULLSCREEN,
    Resizable   = SDL_WINDOW_RESIZABLE,
    Borderless  = SDL_WINDOW_BORDERLESS,
    Hidden      = SDL_WINDOW_HIDDEN,
    Maximized   = SDL_WINDOW_MAXIMIZED,
    Minimized   = SDL_WINDOW_MINIMIZED,
    HighDPI     = SDL_WINDOW_HIGH_PIXEL_DENSITY
};

constexpr WindowFlags operator|(
    WindowFlags A,
    WindowFlags B
)
{
    return static_cast<WindowFlags>(
        static_cast<uint32_t>(A) |
        static_cast<uint32_t>(B)
    );
}

void Quit()
{
    SDL_Quit();
}

SDL_Window* CreateWindow(
    const std::string& Title,
    int Width,
    int Height,
    WindowFlags Flags = WindowFlags::None
)
{
    SDL_Window* Window = SDL_CreateWindow(
        Title.c_str(),
        Width,
        Height,
        static_cast<SDL_WindowFlags>(Flags)
    );

    if (!Window)
    {
        throw std::runtime_error(
            std::string("Failed to create window: ") +
            SDL_GetError()
        );
    }

    return Window;
}

class Renderer
{
public:
    friend class EngineCore;

    Renderer(EngineCore& Manager_, SDL_Window* Window_) :
        Manager(Manager_),
        Window(Window_)
    {
        SDLRenderer = SDL_CreateRenderer(Window, nullptr);

        if (!SDLRenderer)
        {
            throw std::runtime_error(
                std::string("Failed to create renderer: ") + SDL_GetError()
            );
        }
    }

    ~Renderer()
    {
        for (auto& Animation : Manager.Frames)
        {
            for (SDL_Texture* Texture : Animation)
            {
                SDL_DestroyTexture(Texture);
            }
        }

        SDL_DestroyRenderer(SDLRenderer);
        SDL_DestroyWindow(Window);
    }

    void RenderObjects()
    {
        for (auto Idx : Manager.RenderObjectIdxs)
        {
            uint32_t Anim = Manager.AnimNums[Idx];
            uint32_t FrameCount = Manager.AnimFrameCounts[Anim];

            if (FrameCount == 0)
                continue;

            uint32_t Frame = Manager.FrameNums[Idx] % FrameCount;

            SDL_Texture* Texture =
                Manager.Frames[Anim][Frame];

            #ifdef Blokk_CamEnabled
                float x =
                    static_cast<float>(
                        Manager.XPositions[Idx] -
                        Manager.Camera.GetXPosition()
                    );

                float y =
                    static_cast<float>(
                        Manager.YPositions[Idx] -
                        Manager.Camera.GetYPosition()
                    );
            #else
                float x =
                    static_cast<float>(Manager.XPositions[Idx]);

                float y =
                    static_cast<float>(Manager.YPositions[Idx]);
            #endif

            SDL_FRect Destination{
                x,
                y,
                static_cast<float>(Manager.FrameWidths[Anim][Frame]),
                static_cast<float>(Manager.FrameHeights[Anim][Frame])
            };

            SDL_RenderTexture(
                SDLRenderer,
                Texture,
                nullptr,
                &Destination
            );
        }
    }

    void SetClearColor(
        uint8_t R,
        uint8_t G,
        uint8_t B,
        uint8_t A = 255
    )
    {
        SDL_SetRenderDrawColor(SDLRenderer, R, G, B, A );
    }

    
    [[nodiscard]] 
    double TimeRenderObjects()
    {
        // Start time
        auto StartTime = std::chrono::steady_clock::now();

        RenderObjects();

        // End time
        auto EndTime = std::chrono::steady_clock::now();

        // Calculate total time
        auto TotalTime =
            std::chrono::duration<double, std::milli>(
                EndTime - StartTime
            ).count();

        #ifdef Blokk_Diagnostics
            Manager.RenderTime = TotalTime;
        #endif

        return TotalTime;
    }

    void ClearScreen() {
        SDL_RenderClear(SDLRenderer);
    }

    void Present()
    {
        SDL_RenderPresent(SDLRenderer);
    }

private:

    EngineCore& Manager;
    SDL_Window* Window;
    SDL_Renderer* SDLRenderer;

    SDL_Texture* LoadTexture(const std::string& FilePath)
    {
        SDL_Texture* Texture =
            IMG_LoadTexture(SDLRenderer, FilePath.c_str());

        if (!Texture)
        {
            throw std::runtime_error(
                std::string("Failed to load texture: ") + FilePath +
                " - " + SDL_GetError()
            );
        }

        return Texture;
    }

public:

    void CreateNewEmptyAnimation(const std::string& Name)
    {
        Manager.AnimNames[Name] = Manager.Frames.size();

        Manager.Frames.emplace_back();
        Manager.FrameWidths.emplace_back();
        Manager.FrameHeights.emplace_back();
        Manager.AnimFrameCounts.push_back(0);
    }

    void AddFrameToAnimation(
        const std::string& Name,
        const std::string& FilePath
    )
    {
        uint32_t AnimIdx = Manager.AnimNames[Name];

        SDL_Texture* Texture = LoadTexture(FilePath);

        Manager.Frames[AnimIdx].push_back(Texture);

        float Width;
        float Height;

        SDL_GetTextureSize(
            Texture,
            &Width,
            &Height
        );

        Manager.FrameWidths[AnimIdx].push_back(
            static_cast<uint32_t>(Width)
        );

        Manager.FrameHeights[AnimIdx].push_back(
            static_cast<uint32_t>(Height)
        );

        Manager.AnimFrameCounts[AnimIdx]++;
    }

    void CreateAnimation(
        const std::string& Name,
        const std::vector<std::string>& FilePaths
    )
    {
        CreateNewEmptyAnimation(Name);

        for (const std::string& FilePath : FilePaths)
        {
            AddFrameToAnimation(Name, FilePath);
        }
    }
};

#endif // Blokk_Rendering_Enabled

}