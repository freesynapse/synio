#ifndef __TYPES_H
#define __TYPES_H


//
struct ivec2_t
{
    int x = 0;
    int y = 0;
    
    //
    ivec2_t() {}
    ivec2_t(int _i) : x(_i), y(_i) {}
    ivec2_t(int _x, int _y) : x(_x), y(_y) {}

    // operators
    ivec2_t &operator=(const ivec2_t &_v)   { x = _v.x; y =_v.y; return *this;      }
    bool operator==(const ivec2_t &_v)      { return (x == _v.x && y == _v.y);      }
    ivec2_t operator+(const ivec2_t &_v)    { return ivec2_t(x + _v.x, y + _v.y);   }
    void operator+=(const ivec2_t &_v)      { x += _v.x; y += _v.y;                 }
    ivec2_t operator-(const ivec2_t &_v)    { return ivec2_t(x - _v.x, y - _v.y);   }
    void operator-=(const ivec2_t &_v)      { x -= _v.x; y -= _v.y;                 }

};

//
struct irect_t
{
    ivec2_t v0; // top-left
    ivec2_t v1; // bottom-right

    //
    irect_t() {}
    irect_t(int _i) : v0(ivec2_t(_i)), v1(ivec2_t(_i)) {}
    irect_t(const ivec2_t &_v0, const ivec2_t &_v1) : v0(_v0), v1(_v1) {}

    int ncols() { return v1.x - v0.x; }
    int nrows() { return v1.y - v0.y; }

};


#endif // __TYPES_H