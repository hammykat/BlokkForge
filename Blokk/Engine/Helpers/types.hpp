#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <stdlib.h>
#include <stdbool.h>
#include <vector>
#include <string>
#include <concepts>

namespace Blokk {
   
template<typename T>
struct range {
    T Start, End;
    T GetSize() const {
        return End - Start + 1;
    }
};

struct IndexRange : range<uint32_t> {};

template <typename T>
struct vector2 {
    T x, y;

    // operator +
    vector2<T> operator+(T val) {
        return vector2<T>{x + val, y + val};
    }
    [[nodiscard]] inline
    vector2<T> operator+(vector2<T> val) {
        return vector2<T>{x + val.x, y + val.y};
    }
    
    // operator -
    [[nodiscard]] inline
    vector2<T> operator-(T val) {
        return vector2<T>{x - val, y - val};
    }
    [[nodiscard]] inline
    vector2<T> operator-(vector2<T> val) {
        return vector2<T>{x - val.x, y - val.y};
    }

    // operator *
    [[nodiscard]] inline
    vector2<T> operator*(T val) {
        return vector2<T>{x * val, y * val};
    }
    [[nodiscard]] inline
    vector2<T> operator*(vector2<T> val) {
        return vector2<T>{x * val.x, y * val.y};
    }

    // operator /
    [[nodiscard]] inline
    vector2<T> operator/(T val) {
        return vector2<T>{x / val, y / val};
    }
    [[nodiscard]] inline
    vector2<T> operator/(vector2<T> val) {
        return vector2<T>{x / val.x, y / val.y};
    }

    // operator ==
    [[nodiscard]] inline
    bool operator==(T val) {
        return x == val && y == val;
    }
    [[nodiscard]] inline
    bool operator==(vector2<T> val) {
        return x == val.x && y == val.y;
    }

    // operator +=
    inline void operator+=(T val) {
        x += val; 
        y += val;
    }
    inline void operator+=(vector2<T> val) {
        x += val.x;
        y += val.y;
    }

    // operator -=
    inline void operator-=(vector2<T> val) {
        x -= val.x;
        y -= val.y;
    }
    inline void operator-=(T val) {
        x -= val;
        y -= val;
    }

    // operator *=
    inline void operator*=(T val) {
        x *= val;
        y += val;
    }
    inline void operator*=(vector2<T> val) {
        x *= val.x;
        y *= val.y;
    }

    // operator /=
    inline void operator/=(T val) {
        x /= val;
        y /= val;
    }
    inline void operator/=(vector2<T> val) {
        x /= val.x; 
        y /= val.y;
    }

};

struct Circle {
    vector2<float> Center;
    float Radius;
};

struct Rectangle {
    float x, y;
    float width, height;
};

struct Texture2D {
    SDL_Texture* Texture = nullptr;
    int32_t width = 0;
    int32_t height = 0;
};

struct Color {
    uint8_t r, g, b, a;
};

union CollisionHit {
    Rectangle RectHitBox;
    Circle CircleHitBox;
};

struct Animation {
    std::string Name;
    std::vector<Texture2D> Frames;
    size_t CurrentFrame;
};

class ObjectAnimations {
    std::vector<Animation> Animations;
    size_t CurrentAnimation;
};

class GameObject;

struct EngineObjectCreationParams {
    bool visible;
    vector2<float> velocity;
    vector2<float> position;
    GameObject& Object;
};

struct ObjectCreationParams {
    bool visible;
    vector2<float> velocity;
    vector2<float> position;
};

}