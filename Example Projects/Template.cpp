#define Blokk_Rendering_Enabled

#include <Blokk.hpp>

int main()
{
    // Initialize Blokk
    Blokk::Init(
        Blokk::InitFlags::Video |
        Blokk::InitFlags::Events
    );

    // Create an EngineCore
    Blokk::ManagerCreation CreationParams{
        Blokk::Vector2{1280, 720},
        60
    };

    Blokk::EngineCore MyManager(CreationParams);

    // Create the renderer and window
    Blokk::Renderer Renderer(
        MyManager,
        Blokk::CreateWindow(
            "Blokk Example",
            1280,
            720,
            Blokk::WindowFlags::Resizable
        )
    );

    // Set the background color
    Renderer.SetClearColor(
        0,
        0,
        0
    );

    // Create an animation
    Renderer.CreateAnimation(
        "PlayerIdle",
        {
            "Assets/Player/idle_0.png",
            "Assets/Player/idle_1.png",
            "Assets/Player/idle_2.png"
        }
    );

    bool Running = true;

    while (Running)
    {
        // Event handling will go here

        // Process the engine
        MyManager.EngineProcess();

        // Render
        Renderer.ClearScreen();
        Renderer.RenderObjects();
        Renderer.Present();
    }

    // Shut down SDL
    Blokk::Quit();

    return 0;
}
