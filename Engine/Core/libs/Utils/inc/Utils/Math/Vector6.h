//
// Created by Monika on 08.07.2022.
//

#ifndef SRENGINE_VECTOR6_H
#define SRENGINE_VECTOR6_H

#include <Utils/Math/Mathematics.h>

namespace SR_MATH_NS {
    template<typename T> struct SR_DLL_EXPORT Vector6 {
    public:
        union {
            struct {
                T x;
                T y;
                T z;
                T w;
                T v;
                T t;
            };

            T coord[6] = { 0 };
        };
    public:
        SR_FAST_CONSTRUCTOR Vector6() {
            x = y = z = w = v = t = 0;
        }

        SR_FAST_CONSTRUCTOR Vector6(T scalar) {
            x = y = z = w = v = t = scalar;
        }

        template<typename U> SR_FAST_CONSTRUCTOR explicit Vector6(const Vector3<U>& vec) {
            x = static_cast<T>(vec.x);
            y = static_cast<T>(vec.y);
            z = static_cast<T>(vec.z);
            w = static_cast<T>(vec.w);
            v = static_cast<T>(vec.v);
            t = static_cast<T>(vec.t);
        }

        SR_FAST_CONSTRUCTOR Vector6(T _x, T _y, T _z, T _w, T _v, T _t) {
            x = _x;
            y = _y;
            z = _z;
            w = _w;
            v = _v;
            t = _t;
        }
    };

    typedef Vector6<Unit>     FVector6;
    typedef Vector6<int32_t>  IVector6;
    typedef Vector6<uint32_t> UVector6;
}

#endif //SRENGINE_VECTOR6_H
