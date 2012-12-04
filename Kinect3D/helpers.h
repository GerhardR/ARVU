#ifndef HELPERS_H
#define HELPERS_H

#include <functional>
#include <vector>

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;

// transform object to shift data right
template<class Type>
struct shift_right : public std::unary_function <Type, Type> {
    int bits;

    shift_right( int b ) : bits(b) {}
    
    Type operator()( const Type & arg ) const {
        return arg >> bits;
    }
};

inline uint32_t flipColors( const uint32_t bgra ){
    return ((bgra & 0x00ff0000) >> 16 ) | (bgra & 0x0000ff00) | ((bgra & 0x000000ff) << 16);
}

struct Point {
    Point( float ax, float ay, float az, uint32_t ac ) : x(ax), y(ay), z(az), color(ac) 
        {} 
    float x, y, z;
    uint32_t color;
};

void render_points_colored( const std::vector<Point> & points );
void render_points( const std::vector<Point> & points );
void render_skeleton_points( const float * skeleton);
void render_skeleton( const float * skeleton);

#endif
