#pragma once

#include <cmath>

using real_type = float;

struct Vector3 {
    real_type x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(real_type x, real_type y, real_type z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    Vector3 operator-(const Vector3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    Vector3 operator*(real_type t)       const { return {x*t, y*t, z*t}; }
    Vector3 operator-()                  const { return {-x, -y, -z}; }

    real_type  operator[](int i) const { return i==0?x:(i==1?y:z); }
    real_type& operator[](int i)       { if(i==0)return x; if(i==1)return y; return z; }

    real_type length() const { return std::sqrt(x*x + y*y + z*z); }
};

struct Point3 {
    real_type x, y, z;

    Point3() : x(0), y(0), z(0) {}
    Point3(real_type x, real_type y, real_type z) : x(x), y(y), z(z) {}

    Point3  operator+(const Vector3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    Vector3 operator-(const Point3& p)  const { return {x-p.x, y-p.y, z-p.z}; }

    real_type  operator[](int i) const { return i==0?x:(i==1?y:z); }
    real_type& operator[](int i)       { if(i==0)return x; if(i==1)return y; return z; }
};

inline Vector3 normalize(const Vector3& v) {
    real_type len = v.length();
    return {v.x/len, v.y/len, v.z/len};
}

inline real_type dot(const Vector3& a, const Vector3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline Vector3 cross(const Vector3& a, const Vector3& b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

// Reflexao do vetor d em torno da normal n (ambos no mesmo lado).
// r = d - 2*(d . n)*n   (formula classica da reflexao especular ideal)
inline Vector3 reflect(const Vector3& d, const Vector3& n) {
    return d - n * (2.0f * dot(d, n));
}
