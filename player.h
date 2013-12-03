#ifndef PLAYER_CLASS
#define PLAYER_CLASS

#include <string>
#include "vector.h"

class player {
public:
    string Name;
    int Health;
    Vector3 Position;
    Vector4 Rotation;
    string Animation;
    string Compass;
    float Velocity;
    string Ip;
    string Status;
};

#endif
