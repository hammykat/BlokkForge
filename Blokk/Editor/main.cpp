#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "Paths.cpp"
#include "Hierarchy.cpp"
#include "MainAnim.cpp"

#include <filesystem>

enum class Scenes {
    MainAnimation,
    ProjectSelection,
    Editor
};

struct Camera {
    float x, y;
    float Zoom;
};

struct Vector2 {
    float x, y;
};

struct MouseData 
{
    SDL_MouseButtonFlags State; // Current state

    bool LeftDragging = false; // Being dragged?

    Vector2 CurrentPos; // Current position
    Vector2 PosBeforeDrag; // Position before dragging left 
    Vector2 PrevPos; // Position last frame

    bool LeftDown = false; // Currently down?
    bool PrevLeftDown = false; // Down last frame?
    bool LeftClicked = false; // Just clicked?

    void Update()
    {
        // Get current position
        SDL_GetMouseState(&CurrentPos.x, &CurrentPos.y); 

        // Get mouse state
        State = SDL_GetMouseState(nullptr, nullptr);
        LeftDown = State & SDL_BUTTON_LMASK;

        // If left down and not before
        if(LeftDown && !PrevLeftDown) {
            LeftClicked = true; // Clicked this frame
        } 
        // If being dragged and var not updated
        else if (LeftDown && PrevLeftDown && !LeftDragging) {
            LeftDragging = true; // Set dragging
            PosBeforeDrag = CurrentPos; // Set position before drag
            LeftClicked = false; // Being dragged now
        } 
        // If not LeftClicked but was before
        else if(!LeftDown && PrevLeftDown) {
            LeftDragging = false; // Stop dragging
            LeftClicked = false; // Update clicked
        }
        
        // Set prev vars
        PrevLeftDown = LeftDown;
        PrevPos = CurrentPos;
    }

    // Get the amount moved since last frame
    Vector2 GetDeltaSinceLastFrame()
    {
        return Vector2{
            CurrentPos.x - PrevPos.x,
            CurrentPos.y - PrevPos.y
        };
    }
};

int main()
{

    
    // SDL -------------------------
    
    // Init
    SDL_Init(SDL_INIT_VIDEO);
    
    // Window
    SDL_Window* Window = SDL_CreateWindow(
        "Blokk Editor",
        1280,
        720,
        0
    );
    
    // Renderer
    SDL_Renderer* Renderer = SDL_CreateRenderer(
        Window,
        nullptr
    );
    
    // Viewport
    SDL_Texture* ViewportTexture = SDL_CreateTexture(
        Renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_TARGET,
        1280,
        720
    );
    SDL_Texture* CurrentAssetTexture = nullptr; // Displayed asset
    Camera ViewportCamera{ // Camera
        0, // x
        0, // y 
        1.0f // zoom
    }; 
    
    // Mouse -------
    MouseData Mouse;
    
    // Vars ----------------------------

    // Project selection hierarchy
    std::filesystem::path ProjectsPath = GetDocumentsPath() / "Blokk" / "Projects";
    Hierarchy ProjectSelectionHierarchy(ProjectsPath, "Projects", Renderer);

    // Editor hierarchies (made inside loop)
    std::unique_ptr<Hierarchy> EditorFileHierarchy; // Files
    std::unique_ptr<AssetHierarchy> EditorAssetHierarchy; // Assets

    // Current scene state
    Scenes CurrentScene = Scenes::MainAnimation;

    // Starting animation
    BlokkLogo::Animation Logo;
    Logo.Reset();
    
    // Dear ImGui -------------------------

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& IO = ImGui::GetIO();


    IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(Window, Renderer);
    ImGui_ImplSDLRenderer3_Init(Renderer);

    // Main loop ---------------------------------------------------------------------------

    bool Running = true;
    while (Running)
    {
        // SDL events ----------
        SDL_Event Event;
        while (SDL_PollEvent(&Event))
        {
            // Give SDL events to ImGui
            ImGui_ImplSDL3_ProcessEvent(&Event);

            if (Event.type == SDL_EVENT_QUIT)
            {
                Running = false;
            }
        }

        // Update mouse -------
        Mouse.Update();

        // Start ImGui frame -------------------------

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport();

        // UI -------------------------

        ImGui::Begin("Blokk Editor");

        if(CurrentScene == Scenes::ProjectSelection)
        {
            ProjectSelectionHierarchy.Render();

            if(ProjectSelectionHierarchy.OpenFile)
            {
                // Update
                ProjectSelectionHierarchy.OpenFile = false;

                // Switch scenes
                CurrentScene = Scenes::Editor;

                // Create file hierarchy
                std::filesystem::path ProjectPath = ProjectSelectionHierarchy.SelectedPath / "Source" ;
                EditorFileHierarchy = std::make_unique<Hierarchy>(ProjectPath, "Files");

                // Create asset hierarchy
                EditorAssetHierarchy = std::make_unique<AssetHierarchy>(ProjectPath, "Assets");
            }
        }

        else if(CurrentScene == Scenes::MainAnimation)
        {
            if(Logo.IsFinished())
            {
                CurrentScene = Scenes::ProjectSelection;
            }

            Logo.Update();
            Logo.Render(Renderer);
        }

        else if(CurrentScene == Scenes::Editor)
        {
            // Viewport --------------------------------
            SDL_SetRenderTarget(Renderer, ViewportTexture);

            SDL_SetRenderDrawColor(
                Renderer,
                40, 40, 40, 255
            );

            SDL_RenderClear(Renderer);

            // Render ------------------------------------------

            // Asset hierarchy
            if(EditorAssetHierarchy->OpenFile)
            {
                // Close files
                EditorFileHierarchy->OpenFile = false;
                EditorAssetHierarchy->OpenFile = false;

                // If older asset present
                if(CurrentAssetTexture != nullptr)
                {
                    // Clean up, not needed
                    SDL_DestroyTexture(CurrentAssetTexture);
                }

                // Render image onto viewport -------------------------------------------
                SDL_Surface* Surface = IMG_Load(EditorAssetHierarchy->SelectedPath.string().c_str()); // Load
                CurrentAssetTexture = SDL_CreateTextureFromSurface(Renderer, Surface); // Turn to texture
                SDL_DestroySurface(Surface); // Destroy temp surface
            }

            // If present render asset ---------------------
            if(CurrentAssetTexture != nullptr)
            {
                // Render ------------------------------
                SDL_FRect Destination{ // Destination
                    (100.0f - ViewportCamera.x) * ViewportCamera.Zoom, // x
                    (100.0f - ViewportCamera.y) * ViewportCamera.Zoom, // y
                    256.0f * ViewportCamera.Zoom, // width
                    256.0f * ViewportCamera.Zoom // height
                };

                // Render at destination
                SDL_RenderTexture(Renderer, CurrentAssetTexture, nullptr, &Destination); 
            }

            // IMGUI Rendering ------------------------------------

            SDL_SetRenderTarget(Renderer, nullptr);

            // Viewport --------------------------
            ImGui::Begin("Viewport");

            // If viewport being hovered over
            bool ViewportHovered = ImGui::IsWindowHovered();
            if (ViewportHovered)
            {
                // Camera movement -------------------------------
                Vector2 MouseDelta = Mouse.GetDeltaSinceLastFrame();

                // If being dragged
                if(Mouse.LeftDragging) {
                    ViewportCamera.x -= MouseDelta.x; // x
                    ViewportCamera.y -= MouseDelta.y; // y
                }

                // Camera zoom --------------------------------
                ViewportCamera.Zoom += ImGui::GetIO().MouseWheel * 0.1f;

                // Clamp
                if (ViewportCamera.Zoom < 0.1f) ViewportCamera.Zoom = 0.1f; // Too small
                if (ViewportCamera.Zoom > 10.0f) ViewportCamera.Zoom = 10.0f; // Too big
            }

            ImGui::Image(
                (ImTextureID)ViewportTexture,
                ImGui::GetContentRegionAvail()
            );

            ImGui::End(); // Viewport

            // Hierarchies ----------------------
            EditorFileHierarchy->Render();
            EditorAssetHierarchy->Render();
        }

        ImGui::End();


        // Render -------------------------

        ImGui::Render();

        SDL_SetRenderDrawColor(
            Renderer,
            30,
            30,
            30,
            255
        );

        SDL_RenderClear(Renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData(
            ImGui::GetDrawData(),
            Renderer
        );

        SDL_RenderPresent(Renderer);
    }


    // Cleanup -------------------------

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext();

    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);

    SDL_Quit();

    return 0;
}