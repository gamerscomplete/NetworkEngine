#ifndef UTILITIES_H
#define UTILITIES_H
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include "vectors.h"
#include <iomanip>

class BadConversion : public std::runtime_error {
public:
    BadConversion(std::string const& s)
    : std::runtime_error(s)
    { }
};

inline std::string stringify(double x)
{
    std::ostringstream o;
    if(!(o <<x))
        throw BadConversion("stringify(double)");
    return o.str();
}

inline std::string stringify(int x)
{
    std::ostringstream o;
    if(!(o <<x))
        throw BadConversion("stringify(int)");
    return o.str();
}

inline std::string stringify(float x)
{
    std::ostringstream o;
//    o << std::fixed;
    if(!(o <<x))
        throw BadConversion("stringify(float)");
    return o.str();
}

inline std::string stringify(Vector3 x)
{
    std::ostringstream o;
    if(!(o << "(" << x.x << ", " << x.y << ", " << x.z << ")"))
        throw BadConversion("stringify(Vector3)");
    return o.str();
}

inline std::string stringify(Vector2 x)
{
    std::ostringstream o;
    if(!(o << "(" << x.x << ", " << x.y << ")"))
        throw BadConversion("stringify(Vector2)");
    return o.str();
}

inline float Clamp(float min, float max, float val) {
    if(max < min) {
        float tmp = min;
        min = max;
        max = tmp;
    }
            
    if(val < min)
        val = min;
    if(val > max)
        val = max;
    return val;
}

inline float Lerp(float v1, float v2, float amt) {
    float val = v1 + ((v2 - v1) * amt);
    val = Clamp(v1, v2, val);
            
    return val;
}

inline int Lerp(int v1, int v2, float amt) {
    float val = v1 + ((v2 - v1) * amt);
    val = Clamp(v1, v2, val);
            
    return (int)val;
}

#endif
