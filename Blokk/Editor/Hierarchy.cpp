#include <filesystem>
#include <vector>
#include <string>
#include <memory>
#include <utility>

#include "imgui.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_dialog.h>

struct AssetPreview
{
    std::filesystem::path Path;
    std::string Name;

    SDL_Texture* Texture = nullptr;

    float Width = 0.0f;
    float Height = 0.0f;
}; 

struct HierarchyIcons
{
    SDL_Texture* Folder = nullptr;
    SDL_Texture* FolderOpen = nullptr;
    SDL_Texture* FolderPlus = nullptr;
    SDL_Texture* File = nullptr;
    SDL_Texture* FilePicture = nullptr;
   
    HierarchyIcons(SDL_Renderer* Renderer)
    {
        Folder = IMG_LoadTexture(Renderer, "Icons/folder.png");
        FolderPlus = IMG_LoadTexture(Renderer, "Icons/folder-plus.png");
        FolderOpen = IMG_LoadTexture(Renderer, "Icons/folder-open.png");
        File = IMG_LoadTexture(Renderer, "Icons/file-empty.png");
        FilePicture = IMG_LoadTexture(Renderer, "Icons/file-picture.png");
    }

    ~HierarchyIcons()
    {
        if (Folder != nullptr)
        {
            SDL_DestroyTexture(Folder);
        }

        if (FolderOpen != nullptr)
        {
            SDL_DestroyTexture(FolderOpen);
        }

        if (FolderPlus != nullptr)
        {
            SDL_DestroyTexture(FolderPlus);
        }

        if (File != nullptr)
        {
            SDL_DestroyTexture(File);
        }

        if (FilePicture != nullptr)
        {
            SDL_DestroyTexture(FilePicture);
        }
    }
};

struct Folder
{
    std::string Name;
    std::filesystem::path Path;

    std::vector<std::unique_ptr<Folder>> ChildFolders;
    std::vector<std::string> FileNames;

    void SetPath(const std::filesystem::path& _Path)
    {
        Path = _Path;
        Name = _Path.filename().string();
    }
};

struct AssetFolder
{
    std::string Name;
    std::filesystem::path Path;

    std::vector<std::unique_ptr<AssetFolder>> ChildFolders;
    std::vector<AssetPreview> Assets;

    void SetPath(const std::filesystem::path& _Path)
    {
        Path = _Path;
        Name = _Path.filename().string();
    }
};


struct Hierarchy
{
    HierarchyIcons Icons;

    Folder RootFolder;
    std::filesystem::path SelectedPath;

    std::string WindowName;
    bool OpenFile;

    Hierarchy(
        const std::filesystem::path& Path,
        std::string _WindowName,
        SDL_Renderer* Renderer
    ) :
        Icons(Renderer),
        WindowName(_WindowName),
        OpenFile(false)
    {
        LoadDirectory(RootFolder, Path);
    }


    void Render()
    {
        ImGui::Begin(WindowName.c_str());

        DrawFolder(RootFolder);

        ImGui::End();
    }


    void DrawFolder(Folder& TargetFolder)
    {
        // If opened
        ImGuiTreeNodeFlags Flags =
            ImGuiTreeNodeFlags_OpenOnArrow;

        bool Open = ImGui::TreeNodeEx(
            &TargetFolder,
            Flags,
            ""
        );

        if (ImGui::IsItemClicked())
        {
            SelectedPath = TargetFolder.Path;
        }

        ImGui::SameLine();

        SDL_Texture* FolderTexture =
            Open ? Icons.FolderOpen : Icons.Folder;

        if (FolderTexture != nullptr)
        {
            ImGui::Image(
                (ImTextureID)FolderTexture,
                ImVec2(16.0f, 16.0f)
            );

            ImGui::SameLine();
        }

        ImGui::TextUnformatted(
            TargetFolder.Name.c_str()
        );

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* Payload =
                ImGui::AcceptDragDropPayload("REGULAR_FILE"))
            {
                const char* SourcePath =
                    static_cast<const char*>(Payload->Data);

                std::filesystem::path Source(SourcePath);

                MoveFile(
                    Source,
                    TargetFolder
                );
            }

            ImGui::EndDragDropTarget();
        }

        if (Open)
        {
            // Files
            for (const auto& File : TargetFolder.FileNames)
            {
                std::filesystem::path FilePath =
                    TargetFolder.Path / File;

                bool Selected = (SelectedPath == FilePath);

                if (Icons.File != nullptr)
                {
                    ImGui::Image(
                        (ImTextureID)Icons.File,
                        ImVec2(16.0f, 16.0f)
                    );

                    ImGui::SameLine();
                }

                if (ImGui::Selectable(
                    File.c_str(),
                    Selected
                ))
                {
                    SelectedPath = FilePath;
                }

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload(
                        "REGULAR_FILE",
                        FilePath.string().c_str(),
                        FilePath.string().size() + 1
                    );

                    if (Icons.File != nullptr)
                    {
                        ImGui::Image(
                            (ImTextureID)Icons.File,
                            ImVec2(16.0f, 16.0f)
                        );

                        ImGui::SameLine();
                    }

                    ImGui::Text("%s", File.c_str());

                    ImGui::EndDragDropSource();
                }

                if (
                    ImGui::IsItemHovered() &&
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
                )
                {
                    OpenFile = true;
                }
            }

            // Child folders
            for (const auto& Child : TargetFolder.ChildFolders)
            {
                DrawFolder(*Child);
            }

            ImGui::TreePop();
        }
    }


    void LoadDirectory(
        Folder& Parent,
        const std::filesystem::path& Path
    )
    {
        Parent.SetPath(Path);

        for (const auto& Entry :
             std::filesystem::directory_iterator(Path))
        {
            if (Entry.is_directory())
            {
                auto Child = std::make_unique<Folder>();

                LoadDirectory(
                    *Child,
                    Entry.path()
                );

                Parent.ChildFolders.push_back(
                    std::move(Child)
                );
            }
            else if (Entry.is_regular_file())
            {
                Parent.FileNames.push_back(
                    Entry.path().filename().string()
                );
            }
        }
    }

    void MoveFile(
        const std::filesystem::path& Source,
        Folder& DestinationFolder
    )
    {
        std::filesystem::path Destination =
            DestinationFolder.Path / Source.filename();

        if (Source.parent_path() == DestinationFolder.Path)
        {
            return;
        }

        std::filesystem::rename(
            Source,
            Destination
        );

        Folder* SourceFolder =
            FindFolder(RootFolder, Source.parent_path());

        if (SourceFolder == nullptr)
        {
            return;
        }

        for (auto It = SourceFolder->FileNames.begin();
             It != SourceFolder->FileNames.end();
             ++It)
        {
            if (SourceFolder->Path / *It == Source)
            {
                std::string FileName = std::move(*It);

                DestinationFolder.FileNames.push_back(
                    std::move(FileName)
                );

                SourceFolder->FileNames.erase(It);

                break;
            }
        }

        if (SelectedPath == Source)
        {
            SelectedPath = Destination;
        }
    }

    Folder* FindFolder(
        Folder& Folder,
        const std::filesystem::path& Path
    )
    {
        if (Folder.Path == Path)
        {
            return &Folder;
        }

        for (const auto& Child : Folder.ChildFolders)
        {
            Folder* FoundFolder = FindFolder(*Child, Path);

            if (FoundFolder != nullptr)
            {
                return FoundFolder;
            }
        }

        return nullptr;
    }
};

struct AssetHierarchy
{
    HierarchyIcons Icons;

    SDL_DialogFileFilter SupportedFileTypes = { "Images", "bmp;gif;ico;cur;jpg;jpeg;jxl;lbm;pcx;png;pnm;ppm;pgm;pbm;qoi;svg;tga;tif;tiff;webp;xcf;xpm;xv;avif" };

    AssetFolder RootFolder;
    std::filesystem::path SelectedPath;

    AssetFolder* SelectedFolder = nullptr;

    std::string WindowName;
    bool OpenFile;

    SDL_Renderer* Renderer;
    SDL_Window* Window;

    bool ImportAsset;

    AssetHierarchy(
        const std::filesystem::path& Path,
        std::string _WindowName,
        SDL_Renderer* _Renderer,
        SDL_Window* _Window
    ) :
        Icons(_Renderer),
        WindowName(_WindowName),
        Window(_Window),
        OpenFile(false),
        Renderer(_Renderer),
        ImportAsset(false)
    {
        LoadDirectory(RootFolder, Path);
    }

    ~AssetHierarchy()
    {
        DestroyTextures(RootFolder);
    }


    void Render()
    {
        ImGui::Begin(WindowName.c_str());

        // Import buton
        if (Icons.FilePicture != nullptr)
        {
            if (ImGui::ImageButton(
                "##Import",
                (ImTextureID)Icons.FilePicture,
                ImVec2(18.0f, 18.0f)
            ))
            {
                // Let SDL handle file picker
                SDL_ShowOpenFileDialog(
                    AssetHierarchy::Callback,
                    this,
                    Window,
                    &SupportedFileTypes,
                    1,
                    nullptr,
                    true
                );
            }
        }
        else if(ImGui::Button("+"))
        {
            // Let SDL handle file picker
            SDL_ShowOpenFileDialog(
                AssetHierarchy::Callback,
                this,
                Window,
                &SupportedFileTypes,
                1,
                nullptr,
                true
            );
        }

        ImGui::Separator();

        DrawFolder(RootFolder);

        ImGui::End();
    }

    void DrawFolder(AssetFolder& TargetFolder)
    {
        // Flags
        ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (SelectedFolder == &TargetFolder) {
            Flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool Open = ImGui::TreeNodeEx(
            &TargetFolder,
            Flags,
            ""
        );

        // Select folder
        if (ImGui::IsItemClicked())
        {
            SelectedFolder = &TargetFolder;
        }

        ImGui::SameLine();

        SDL_Texture* FolderTexture =
            Open ? Icons.FolderOpen : Icons.Folder;

        if (FolderTexture != nullptr)
        {
            ImGui::Image(
                (ImTextureID)FolderTexture,
                ImVec2(16.0f, 16.0f)
            );

            ImGui::SameLine();
        }

        ImGui::TextUnformatted(
            TargetFolder.Name.c_str()
        );

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* Payload =
                ImGui::AcceptDragDropPayload("ASSET_FILE"))
            {
                const char* SourcePath =
                    static_cast<const char*>(Payload->Data);

                std::filesystem::path Source(SourcePath);

                MoveFile(
                    Source,
                    TargetFolder
                );
            }

            ImGui::EndDragDropTarget();
        }

        // If opened
        if (Open)
        {
            for (AssetPreview& Asset : TargetFolder.Assets)
            {
                std::filesystem::path FilePath = Asset.Path;

                bool Selected = (SelectedPath == FilePath);

                ImGui::Image(
                    (ImTextureID)Asset.Texture,
                    ImVec2(Asset.Width, Asset.Height)
                );

                ImGui::SameLine();

                if (ImGui::Selectable(
                    Asset.Name.c_str(),
                    Selected
                ))
                {
                    SelectedPath = FilePath;
                }

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload(
                        "ASSET_FILE",
                        FilePath.string().c_str(),
                        FilePath.string().size() + 1
                    );

                    ImGui::Text("%s", Asset.Name.c_str());

                    ImGui::EndDragDropSource();
                }

                if (
                    ImGui::IsItemHovered() &&
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
                )
                {
                    OpenFile = true;
                }
            }

            // Child folders
            for (const auto& Child : TargetFolder.ChildFolders)
            {
                DrawFolder(*Child);
            }

            ImGui::TreePop();
        }
    }

    void LoadDirectory(
        AssetFolder& Parent,
        const std::filesystem::path& Path
    )
    {
        Parent.SetPath(Path);

        for (const auto& Entry :
            std::filesystem::directory_iterator(Path))
        {
            if (Entry.is_directory())
            {
                auto Child = std::make_unique<AssetFolder>();

                LoadDirectory(
                    *Child,
                    Entry.path()
                );

                Parent.ChildFolders.push_back(
                    std::move(Child)
                );
            }
            else if (Entry.is_regular_file())
            {
                // Load original image
                SDL_Surface* Original =
                    IMG_Load(Entry.path().string().c_str());

                if (Original == nullptr)
                {
                    continue;
                }

                // Create asset
                AssetPreview Asset;

                Asset.Path = Entry.path();
                Asset.Name = Entry.path().filename().string();

                // Sizing
                float TargetTextureHeight =
                    ImGui::GetTextLineHeight() * 1.5f;

                float TextureWidth =
                    static_cast<float>(Original->w);

                float TextureHeight =
                    static_cast<float>(Original->h);

                float Scale =
                    TargetTextureHeight / TextureHeight;

                TextureWidth *= Scale;

                SDL_Surface* PreviewSurface =
                    SDL_CreateSurface(
                        static_cast<int>(TextureWidth),
                        static_cast<int>(TargetTextureHeight),
                        SDL_PIXELFORMAT_RGBA32
                    );

                SDL_BlitSurfaceScaled(
                    Original,
                    nullptr,
                    PreviewSurface,
                    nullptr,
                    SDL_SCALEMODE_LINEAR
                );

                Asset.Texture =
                    SDL_CreateTextureFromSurface(
                        Renderer,
                        PreviewSurface
                    );

                Asset.Width = TextureWidth;
                Asset.Height = TargetTextureHeight;

                Parent.Assets.push_back(
                    std::move(Asset)
                );

                // Clean up CPU surfaces
                SDL_DestroySurface(Original);
                SDL_DestroySurface(PreviewSurface);
            }
        }
    }

    static void SDLCALL Callback(
        void* UserData,
        const char* const* FileList,
        int Filter
    ) {
        AssetHierarchy* This = static_cast<AssetHierarchy*>(UserData);

        if (FileList == nullptr)
        {
            // Error
            return;
        }

        if (*FileList == nullptr)
        {
            // User canceled
            return;
        }

        // Loop through the files
        for (const char* const* File = FileList; *File != nullptr; ++File)
        {
            std::filesystem::path Source = *File;

            // Copy file into the root asset folder
            This->AddFile(Source);
        }
    }

    void AddFile(const std::filesystem::path& Path)
    {
        if (SelectedFolder == nullptr)
        {
            SelectedFolder = &RootFolder;
        }

        std::filesystem::path Destination =
            SelectedFolder->Path / Path.filename();

        std::filesystem::copy_file(
            Path,
            Destination,
            std::filesystem::copy_options::overwrite_existing
        );

        SDL_Surface* Original =
            IMG_Load(Destination.string().c_str());

        if (Original == nullptr)
        {
            return;
        }

        AssetPreview Asset;

        Asset.Path = Destination;
        Asset.Name = Destination.filename().string();

        // Sizing
        float TargetTextureHeight = ImGui::GetTextLineHeight() * 1.5f;
        float TextureWidth = static_cast<float>(Original->w);
        float TextureHeight = static_cast<float>(Original->h);
        float Scale = TargetTextureHeight / TextureHeight;
        TextureWidth *= Scale;

        SDL_Surface* PreviewSurface =
            SDL_CreateSurface(
                static_cast<int>(TextureWidth),
                static_cast<int>(TargetTextureHeight),
                SDL_PIXELFORMAT_RGBA32
            );

        SDL_BlitSurfaceScaled(
            Original,
            nullptr,
            PreviewSurface,
            nullptr,
            SDL_SCALEMODE_LINEAR
        );

        Asset.Texture =
            SDL_CreateTextureFromSurface(
                Renderer,
                PreviewSurface
            );

        Asset.Width = TextureWidth;
        Asset.Height = TargetTextureHeight;

        SelectedFolder->Assets.push_back(
            std::move(Asset)
        );

        // Clean up
        SDL_DestroySurface(Original);
        SDL_DestroySurface(PreviewSurface);
    }

    void MoveFile(
        const std::filesystem::path& Source,
        AssetFolder& DestinationFolder
    )
    {
        std::filesystem::path Destination =
            DestinationFolder.Path / Source.filename();

        std::filesystem::rename(
            Source,
            Destination
        );

        AssetFolder* SourceFolder =
            FindFolder(RootFolder, Source.parent_path());

        if (SourceFolder == nullptr)
        {
            return;
        }

        for (auto It = SourceFolder->Assets.begin();
            It != SourceFolder->Assets.end();
            ++It)
        {
            if (It->Path == Source)
            {
                AssetPreview Asset =
                    std::move(*It);

                Asset.Path = Destination;

                DestinationFolder.Assets.push_back(
                    std::move(Asset)
                );

                SourceFolder->Assets.erase(It);

                break;
            }
        }
    }

    AssetFolder* FindFolder(
        AssetFolder& Folder,
        const std::filesystem::path& Path
    ) {
        if (Folder.Path == Path)
        {
            return &Folder;
        }

        for (const auto& Child : Folder.ChildFolders)
        {
            AssetFolder* Result =
                FindFolder(*Child, Path);

            if (Result != nullptr)
            {
                return Result;
            }
        }

        return nullptr;
    }

    void DestroyTextures(AssetFolder& Folder)
    {
        for (AssetPreview& Asset : Folder.Assets)
        {
            if (Asset.Texture != nullptr)
            {
                SDL_DestroyTexture(Asset.Texture);
                Asset.Texture = nullptr;
            }
        }

        for (const auto& Child : Folder.ChildFolders)
        {
            DestroyTextures(*Child);
        }
    }
};
