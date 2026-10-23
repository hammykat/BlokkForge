#include "GameObject.hpp" 
#include "EngineClassData.hpp"
 
namespace Blokk { 
 
// Set 
void GameObject::SetPosition(float TargetX, float TargetY)  
{ 
    // Update X and Y 
    Engine->XPositions[EngineIdx] = TargetX;
    Engine->YPositions[EngineIdx] = TargetY;
} 

void GameObject::SetPosition(vector2<float> TargetPos) 
{ 
    SetPosition(TargetPos.x, TargetPos.y); 
} 
 
void GameObject::SetXPosition(float Target)  
{ 
    Engine->XPositions[EngineIdx] = Target;
} 

void GameObject::SetYPosition(float Target)  
{ 
    Engine->YPositions[EngineIdx] = Target;
} 
 
// Get 
vector2<float> GameObject::GetPosition()  
{ 
    return vector2<float>( 
        Engine->XPositions[EngineIdx], 
        Engine->YPositions[EngineIdx] 
    ); 
} 
 
// Change 
void GameObject::ChangePosition(float ChangeX, float ChangeY)  
{ 
    Engine->XPositions[EngineIdx] += ChangeX;
    Engine->YPositions[EngineIdx] += ChangeY;
} 

void GameObject::ChangePosition(vector2<float> Change) 
{ 
    ChangePosition(Change.x, Change.y); 
} 
 
void GameObject::ChangeXPosition(float X)  
{ 
    Engine->XPositions[EngineIdx] += X;
} 

void GameObject::ChangeYPosition(float Y)  
{ 
    Engine->YPositions[EngineIdx] += Y;
} 
 
}