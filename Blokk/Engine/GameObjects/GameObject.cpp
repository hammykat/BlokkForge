#include "GameObject.hpp"
#include "EngineClassData.hpp"

namespace Blokk
{

GameObject::GameObject(ObjectCreationParams CP)
    :
    IsStatic(CP.velocity == 0),
    EngineIdx(0),
    IsVisible(true)
{
    Engine->ProcessAddCommand<GameObject>(
        EngineObjectCreationParams<GameObject>{
            CP.visible, // is visible?
            CP.velocity, // velocity
            CP.position, // position
            *this // instance
        }
    );
}

GameObject::GameObject(
    vector2<float> velocity,
    vector2<float> position,
    GameObject* object,
    bool visible
)
    :
    IsStatic(velocity.x == 0 && velocity.y == 0),
    EngineIdx(0),
    IsVisible(true)
{
    Engine->ProcessAddCommand<GameObject>(
        visible,
        velocity,
        position,
        object
    );
}

GameObject::~GameObject()
{
    if (IsStatic) {
        Engine->DestroyStaticObject(EngineIdx);
    }
    else {
        Engine->DestroyDynamicObject(EngineIdx);
    }
}

/* Set the object to static
Assumes the object was dynamic at first */
void GameObject::SetToStatic()
{
    IsStatic = true;

    // Remove velocity from engine data --------------------------------------------
    Engine->SwapDynamicObjects(EngineIdx, Engine->DynamicObjectCount - 1); // Swap with last dynamic
    Engine->SwapStaticObjects(EngineIdx, Engine->ObjectCount - 1); // Swap with last static
    // Remove velocity
    Engine->XVelocities.pop_back(); 
    Engine->YVelocities.pop_back(); 

    // Update engine vars
    Engine->StaticObjectCount++;
    Engine->DynamicObjectCount--;
}

// Set the object to dynamic
// Assumes the object was static at first
void GameObject::SetToDynamic(vector2<float> Vel)
{
    IsStatic = false;

    // Swap with first static object
    Engine->SwapStaticObjects(EngineIdx, Engine->DynamicObjectCount);

    // Add velocity to engine data
    Engine->XVelocities.push_back(Vel.x);
    Engine->YVelocities.push_back(Vel.y);

    // Update engine vars
    Engine->DynamicObjectCount++;
    Engine->StaticObjectCount--;
}

}