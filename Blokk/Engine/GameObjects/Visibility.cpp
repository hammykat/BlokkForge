#include "GameObject.hpp"
#include "EngineClassData.hpp"

#ifdef Blokk_Rendering_Enabled

namespace Blokk {

// Set to visible
void GameObject::Show() // Assumes that the object was invisible before
{
    // Update local var
    IsVisible = true;

    // Set to visible
    Engine->IsVisible[EngineIdx] = true;
}

// Set to invisible
void GameObject::Hide() // Assumes that the object was visible before
{
    // Update local var
    IsVisible = false;

    // Set to invisible
    Engine->IsVisible[EngineIdx] = false;
}

bool GameObject::IsCurrentlyVisible() {
    return IsVisible;
}

void GameObject::ToggleVisibility() 
{
    // If visible
    if(IsVisible)
    {
        // Set to invisible
        Hide();
    }
    else // If not visible
    {
        // Set to visible
        Show();
    }
}  
    
}

#endif
