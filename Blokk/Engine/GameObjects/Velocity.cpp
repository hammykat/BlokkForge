#include "GameObject.hpp"
#include "EngineClassData.hpp"

#include <cmath>

#define Vel_ZERO vector2<float>{0, 0}

namespace Blokk {

// Set
void GameObject::SetVelocity(float xvel, float YVel) 
{
    // If being set to static
    if(xvel == 0 && YVel == 0) {

        // If Dynamic
        if(!IsStatic) 
        {
            // Set to static
            SetToStatic();
        }
    } 
    else // If being set to dynamic 
    {
        // If static
        if(IsStatic) 
        {
            // Set to dynamic
            SetToDynamic(vector2<float>{xvel, YVel});
        }
        else // if dynamic
        {
            // Set X and Y
            Engine->XVelocities[EngineIdx] = xvel;
            Engine->YVelocities[EngineIdx] = YVel;
        }
    }
}

void GameObject::SetVelocity(vector2<float> Velocity) 
{
    SetVelocity(Velocity.x, Velocity.y);
}

void GameObject::SetVelocityX(float XVel) 
{
    // If being set to static
    if(XVel == 0) 
    {
        // If static
        if(IsStatic) 
        {
            // Do nothing
            return;
        } 
        else // If dynamic
        {
            // If being set to static
            if(GetVelocityY() == 0) 
            {
                // Set to static
                SetToStatic();
            }
            else // If staying dynamic
            {
                // Set X
                Engine->XVelocities[EngineIdx] = XVel;
            }
        }
    } 
    else // If being set to dynamic 
    {
        // If static
        if(IsStatic) 
        {
            // Set to dynamic with vel
            SetToDynamic(vector2<float>(XVel, 0));
        }
        else // If dynamic
        {
            // Set X
            Engine->XVelocities[EngineIdx] = XVel;
        }
    }
}

void GameObject::SetVelocityY(float YVel) 
{
    // If being set to static
    if(YVel == 0) 
    {
        // If static
        if(IsStatic) 
        {
            // Do nothing
            return;
        } 
        else // If dynamic
        {
            // If being set to static
            if(GetVelocityX() == 0) 
            {
                // Set to static
                SetToStatic();
            }
            else // If staying dynamic
            {
                // Set Y
                Engine->YVelocities[EngineIdx] = YVel;
            }
        }
    } 
    else // If being set to dynamic 
    {
        // If static
        if(IsStatic) 
        {
            // Set to dynamic with vel
            SetToDynamic(vector2<float>(0, YVel));
        }
        else // If dynamic
        {
            // Set Y
            Engine->YVelocities[EngineIdx] = YVel;
        }
    }
}

// Get
vector2<float> GameObject::GetVelocity() 
{
    // If the object is static
    if(IsStatic)
    {
        // Give the static velocity (Always 0)
        return Vel_ZERO;
    }
    else // If the object is dynamic
    {
        // Get the velocity from the manager
        return {
            Engine->XVelocities[EngineIdx], 
            Engine->YVelocities[EngineIdx]
        };
    }
}

float GameObject::GetVelocityX() 
{
    // If the object is static
    if(IsStatic)
    {
        return 0;
    }
    else // If the object is dynamic
    {
        // Get the vel from the manager's data
        return Engine->XVelocities[EngineIdx];
    }
}

float GameObject::GetVelocityY() 
{
    // If the object is static
    if(IsStatic)
    {
        return 0;
    }
    else // If the object is dynamic
    {
        // Get the vel from the manager's data
        return Engine->YVelocities[EngineIdx];
    }
}

// Get the speed
float GameObject::GetSpeed() 
{
    // If static return 0, not moving
    if(IsStatic) return 0;

    float XVel = Engine->XVelocities[EngineIdx];
    float YVel = Engine->YVelocities[EngineIdx];

    return sqrt(XVel * XVel + YVel * YVel);
}

float GameObject::GetSpeedSquared() 
{
    // If static return 0, not moving
    if(IsStatic) return 0;

    float XVel = Engine->XVelocities[EngineIdx];
    float YVel = Engine->YVelocities[EngineIdx];

    return XVel * XVel + YVel * YVel;
}

// If not moving, vel != 0
bool GameObject::IsMoving() 
{
    return !IsStatic;
}

// Stop, set to 0
void GameObject::StopVelocity() 
{
    // Remove its idx if present
    if(IsStatic)
    {
        return;
    }
    else // If dynamic
    {
        // Set to static
        SetToStatic();
    }
}

// Reverse
void GameObject::ReverseVelocity() 
{
    // If static do nothing
    if(IsStatic) return;

    // Reverse X and Y
    Engine->XVelocities[EngineIdx] *= -1;
    Engine->YVelocities[EngineIdx] *= -1;
}

void GameObject::ReverseVelocityX() 
{
    // If static do nothing
    if(IsStatic) return;

    // Reverse X
    Engine->XVelocities[EngineIdx] *= -1;
}

void GameObject::ReverseVelocityY() 
{
    // If static do nothing
    if(IsStatic) return;

    // Reverse Y
    Engine->YVelocities[EngineIdx] *= -1;
}

// Multiply ------------------------------------------------

void GameObject::MultiplyVelocity(float FactorX, float FactorY)
{
    // Return if 0
    if(IsStatic)
        return;

    // Find new X and Y
    float NewX = GetVelocityX() * FactorX;
    float NewY = GetVelocityY() * FactorY;

    if(NewX == 0 && NewY == 0)
    {
        SetToStatic();
        return;
    }

    // Update data
    Engine->XVelocities[EngineIdx] *= FactorX;
    Engine->YVelocities[EngineIdx] *= FactorY;
}

void GameObject::MultiplyVelocity(vector2<float> Factor) 
{
    MultiplyVelocity(Factor.x, Factor.y);
}

void GameObject::MultiplyVelocity(float Factor) 
{
    MultiplyVelocity(Factor, Factor);
}

// Individual
void GameObject::MultiplyXVelocity(float Factor) 
{
    MultiplyVelocity(Factor, 1);
}

void GameObject::MultiplyYVelocity(float Factor) 
{
    MultiplyVelocity(1, Factor);
}

// Add --------------------------------------------------

void GameObject::AddVelocity(float XVel, float YVel) 
{
    // Don't do anything if not changing
    if(XVel == 0 && YVel == 0) return;

    // If the object is static
    if(IsStatic)
    {
        // Set to dynamic
        SetToDynamic(vector2<float>{XVel, YVel});
    }
    else // If the object is dynamic
    {
        // Add
        Engine->XVelocities[EngineIdx] += XVel;
        Engine->YVelocities[EngineIdx] += YVel;
    }
}

void GameObject::AddVelocity(vector2<float> Vel) 
{
    AddVelocity(Vel.x, Vel.y);
}

// Divide -----------------------------------------------------

void GameObject::DivideVelocity(float XVel, float YVel) 
{
    // Avoid bugs with dividing by 0
    if(XVel == 0 || YVel == 0 || IsStatic) return;

    // Divide
    Engine->XVelocities[EngineIdx] /= XVel;
    Engine->YVelocities[EngineIdx] /= YVel;
}

void GameObject::DivideVelocity(vector2<float> Vel) 
{
    DivideVelocity(Vel.x, Vel.y);
}

void GameObject::DivideVelocityX(float Vel) 
{
    DivideVelocity(Vel, 1);
}

void GameObject::DivideVelocityY(float Vel) 
{
    DivideVelocity(1, Vel);
}

}