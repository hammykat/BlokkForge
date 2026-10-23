#include <cstdint>

// Camera functions
#ifdef Blokk_CamEnabled

class CameraController
{

public:

    int32_t GetXPosition() const { 
        return x; 
    }

    int32_t GetYPosition() const { 
        return y; 
    }

    void SetPosition(int32_t X, int32_t Y)
    {
        x = X;
        y = Y;
    }
    
    void SetPosition(Vector2 Pos)
    {
        x = Pos.x;
        y = Pos.y;
    }
    
    void SetXPosition(int32_t X) { 
        x = X; 
    }
    
    void SetYPosition(int32_t Y) { 
        y = Y; 
    }
    
    void ChangeXPosition(int32_t X) { 
        x += X; 
    }
    
    void ChangeYPosition(int32_t Y) { 
        y += Y; 
    }

    private:

    int32_t x;
    int32_t y;
};

#endif