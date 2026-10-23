
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <string>

#include <filesystem>
#include <vector>

struct HierarchyData
{
    std::vector<std::filesystem::path> Folders;
    std::vector<std::filesystem::path> Files;
};


// Get all the folder names inside a directory
std::vector<std::filesystem::path> GetFolders(
    const std::filesystem::path& Folder
)
{
    std::vector<std::filesystem::path> Results;

    for (const auto& Entry : std::filesystem::directory_iterator(Folder))
    {
        if (Entry.is_directory())
        {
            Results.push_back(Entry.path().filename());
        }
    }

    return Results;
}


// Get all the file names inside a directory
std::vector<std::filesystem::path> GetFiles(
    const std::filesystem::path& Folder
)
{
    std::vector<std::filesystem::path> Results;

    for (const auto& Entry : std::filesystem::directory_iterator(Folder))
    {
        if (Entry.is_regular_file())
        {
            Results.push_back(Entry.path().filename());
        }
    }

    return Results;
}


// Get all the files and folders inside a directory
HierarchyData GetFilesAndFolders(
    const std::filesystem::path& Folder
)
{
    HierarchyData Results;

    for (const auto& Entry : std::filesystem::directory_iterator(Folder))
    {
        // If file
        if (Entry.is_regular_file())
        {
            Results.Files.push_back(Entry.path().filename());
        }
        // If folder
        else if (Entry.is_directory())
        {
            Results.Folders.push_back(Entry.path().filename());
        }
    }

    return Results;
}


// Getting the path ------------------------

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#endif


std::filesystem::path GetDocumentsPath()
{
#ifdef _WIN32

    PWSTR Path = nullptr;

    HRESULT Result = SHGetKnownFolderPath(
        FOLDERID_Documents,
        0,
        nullptr,
        &Path
    );

    if (FAILED(Result))
    {
        throw std::runtime_error(
            "Failed to find Documents folder."
        );
    }

    std::filesystem::path Documents(Path);

    CoTaskMemFree(Path);

    return Documents;

#elif defined(__APPLE__) || defined(__linux__)

    const char* Home = std::getenv("HOME");

    if (Home == nullptr)
    {
        throw std::runtime_error(
            "Failed to find home directory."
        );
    }

    return std::filesystem::path(Home) / "Documents";

#else

    #error "Unsupported operating system"

#endif
}