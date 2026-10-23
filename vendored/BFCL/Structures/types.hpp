#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <stdlib.h>
#include <stdbool.h>
#include <vector>
#include <string>

namespace Blokk {
    
struct Range {
    int Start, End;
    size_t GetSize() {
        return End - Start + 1;
    }
};

// The vector stores coordinates in a 2D plane. It can be used to store positions, velocities, and other 2D data.
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
        return vector2<T>{x* val, y* val};
    }
    [[nodiscard]] inline
        vector2<T> operator*(vector2<T> val) {
        return vector2<T>{x* val.x, y* val.y};
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
    Vector2 Center;
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

struct DynamicRegisterInfo {
    Vector2 Vel;
    size_t Idx;
};
}