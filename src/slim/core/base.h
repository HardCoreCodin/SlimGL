#pragma once

#include <cmath>

#if defined(__clang__)
    #define COMPILER_CLANG 1
    #define COMPILER_CLANG_OR_GCC 1
#elif defined(__GNUC__) || defined(__GNUG__)
    #define COMPILER_GCC 1
    #define COMPILER_CLANG_OR_GCC 1
    #include <new>
#elif defined(_MSC_VER)
    #define COMPILER_MSVC 1
#endif

#ifdef __CUDACC__
    #ifndef NDEBUG
        #include <stdio.H>
        #include <stdlib.H>
        inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true) {
            if (code != cudaSuccess) {
                fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code) , file, line);
                if (abort) exit(code);
            }
        }
        #define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
        #ifndef XPU
            #define XPU __device__ __host__
        #endif
        #ifndef INLINE_XPU
            #define INLINE_XPU __device__ __host__
        #endif
        #ifndef INLINE
            #define INLINE
        #endif
    #else
        #ifndef XPU
            #define XPU __device__ __host__
        #endif
        #ifndef INLINE_XPU
            #define INLINE_XPU __device__ __host__ __forceinline__
        #endif
        #ifndef INLINE
            #define INLINE __forceinline__
        #endif
        #define gpuErrchk(ans) (ans);
    #endif

    #define checkErrors() gpuErrchk(cudaPeekAtLastError())
    #define uploadConstant(cpu_ptr, constant)      gpuErrchk(cudaMemcpyToSymbol(constant, cpu_ptr, sizeof(constant)))
    #define uploadNto(cpu_ptr, gpu_ptr, N, offset) gpuErrchk(cudaMemcpy(&((gpu_ptr)[(offset)]), (cpu_ptr), sizeof((cpu_ptr)[0]) * (N), cudaMemcpyHostToDevice))
    #define uploadN(  cpu_ptr, gpu_ptr, N        ) gpuErrchk(cudaMemcpy(&((gpu_ptr)[0])       , (cpu_ptr), sizeof((cpu_ptr)[0]) * (N), cudaMemcpyHostToDevice))
    #define downloadN(gpu_ptr, cpu_ptr, N)         gpuErrchk(cudaMemcpy((cpu_ptr), &((gpu_ptr)[0])       , sizeof((cpu_ptr)[0]) * (N), cudaMemcpyDeviceToHost))
    #define downloadNto(gpu_ptr,cpu_ptr,N, offset) gpuErrchk(cudaMemcpy((cpu_ptr), &((gpu_ptr)[(offset)]), sizeof((cpu_ptr)[0]) * (N), cudaMemcpyDeviceToHost))
    #define setDataOnGPU(gpu_ptr, data, N)         gpuErrchk(cudaMemset((gpu_ptr), (data),                 sizeof((gpu_ptr)[0]) * (N)))
#else
    #ifndef XPU
        #define XPU
    #endif

    #ifndef INLINE
        #if (defined(SLIMMER) || !defined(NDEBUG))
            #define INLINE
        #elif defined(COMPILER_MSVC)
            #define INLINE inline __forceinline
        #elif defined(COMPILER_CLANG_OR_GCC)
            #define INLINE inline __attribute__((always_inline))
        #else
            #define INLINE inline
        #endif
    #endif
    #ifndef INLINE_XPU
        #define INLINE_XPU INLINE
    #endif
#endif

#if defined(COMPILER_CLANG)
    #define likely(x)   __builtin_expect(x, true)
    #define unlikely(x) __builtin_expect_with_probability(x, false, 0.95)
#else
    #define likely(x)   x
    #define unlikely(x) x
#endif

#ifdef COMPILER_CLANG
    #define ENABLE_FP_CONTRACT \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wunknown-pragmas\"") \
        _Pragma("STDC FP_CONTRACT ON") \
        _Pragma("clang diagnostic pop")
#else
    #define ENABLE_FP_CONTRACT
#endif

#ifdef FP_FAST_FMAF
    #define fast_mul_add(a, b, c) fmaf(a, b, c)
#else
    ENABLE_FP_CONTRACT
    #define fast_mul_add(a, b, c) ((a) * (b) + (c))
#endif

#ifndef signbit
    #define signbit std::signbit
#endif

//#define SLIM_EXPORT
#ifdef SLIM_EXPORT
    #if defined(COMPILER_CLANG)
        #define SLIM_API __declspec(dllexport)
    #else
        #define SLIM_API __attribute__((visibility("default")))
    #endif
#else
    #if defined(COMPILER_CLANG)
        #define SLIM_API __declspec(dllimport)
    #else
        #define SLIM_API
    #endif
#endif

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned long int  u32;
typedef unsigned long long u64;
typedef signed   short     i16;
typedef signed   long int  i32;

typedef float  f32;
typedef double f64;

#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))

#ifndef CANVAS_COUNT
#define CANVAS_COUNT 2
#endif

#define FONT_WIDTH 9
#define FONT_HEIGHT 12

#define EPS 0.0001f
#define HALF_SQRT2 0.70710678118f
#define SQRT2 1.41421356237f
#define SQRT3 1.73205080757f

#define pi (3.14159265f)
#define TAU (2.0f*pi)
#define UNIT_SPHERE_AREA_OVER_SIX ((4.0f*pi)/6.0f)
#define ONE_OVER_PI (1.0f/pi)
#define ONE_OVER_TWO_PI (1.0f/TAU)

#define TET_MAX (0.577350259f)
#define TET_MIN (0.288675159f)

#define COLOR_COMPONENT_TO_FLOAT 0.00392156862f
#define FLOAT_TO_COLOR_COMPONENT 255.0f
#define DEG_TO_RAD 0.0174533f

#define MAX_COLOR_VALUE 0xFF

#define MATERIAL_IS_REFLECTIVE ((u8)1)
#define MATERIAL_IS_REFRACTIVE ((u8)2)
#define MATERIAL_IS_EMISSIVE ((u8)4)
#define MATERIAL_HAS_ALBEDO_MAP ((u8)16)
#define MATERIAL_HAS_NORMAL_MAP ((u8)32)

#define GEOMETRY_IS_VISIBLE ((u8)1)
#define GEOMETRY_IS_SHADOWING ((u8)2)
#define GEOMETRY_IS_TRANSPARENT ((u8)4)

#define TRACE_OFFSET 0.0001f

#define MAX_HIT_DEPTH 4
#define MAX_DISTANCE INFINITY
#define MAX_TRIANGLES_PER_MESH_BVH_NODE 2
#define MAX_OBJS_PER_SCENE_BVH_NODE 2
#define MAX_PRIMITIVES_PER_LEAF ( \
            MAX_TRIANGLES_PER_MESH_BVH_NODE > MAX_OBJS_PER_SCENE_BVH_NODE ? \
            MAX_TRIANGLES_PER_MESH_BVH_NODE : MAX_OBJS_PER_SCENE_BVH_NODE   \
)

#define IOR_AIR 1.0003f
#define IOR_GLASS 1.52f

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

#define MEMORY_SIZE Gigabytes(1)
#define MEMORY_BASE Terabytes(2)

#define MAX_WIDTH 3840
#define MAX_HEIGHT 2160
#define MAX_WINDOW_SIZE (MAX_WIDTH * MAX_HEIGHT)

#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080

#define WINDOW_CONTENT_PIXEL_SIZE 4
#define WINDOW_CONTENT_SIZE (MAX_WINDOW_SIZE * WINDOW_CONTENT_PIXEL_SIZE)

#define BOX__ALL_SIDES (BoxSide_Top | BoxSide_Bottom | BoxSide_Left | BoxSide_Right | BoxSide_Front | BoxSide_Back)
#define BOX__VERTEX_COUNT 8
#define BOX__EDGE_COUNT 12
#define GRID__MAX_SEGMENTS 101

#define TET__VERTEX_COUNT 4
#define TET__EDGE_COUNT 6
#define QUAD__VERTEX_COUNT 4
#define QUAD__EDGE_COUNT 4

#define CURVE_STEPS 360

#define CUBE_QUAD_UV_COUNT 4
#define CUBE_FLAT_UV_COUNT 14
#define CUBE_NORMAL_COUNT 6
#define CUBE_TANGENT_COUNT 36
#define CUBE_VERTEX_COUNT 8
#define CUBE_TRIANGLE_COUNT 12
#define CUBE_TRIANGLE_EDGE_COUNT 18
#define CUBE_QUAD_EDGE_COUNT 24
#define CUBE_BBOX_EDGE_COUNT 12

#define CAMERA_DEFAULT__FOCAL_LENGTH 2.0f
#define CAMERA_DEFAULT__TARGET_DISTANCE 10

#define NAVIGATION_DEFAULT__MAX_VELOCITY 5
#define NAVIGATION_DEFAULT__ACCELERATION 10
#define NAVIGATION_SPEED_DEFAULT__TURN   1
#define NAVIGATION_SPEED_DEFAULT__ORIENT 0.002f
#define NAVIGATION_SPEED_DEFAULT__ORBIT  0.002f
#define NAVIGATION_SPEED_DEFAULT__ZOOM   0.003f
#define NAVIGATION_SPEED_DEFAULT__DOLLY  1
#define NAVIGATION_SPEED_DEFAULT__PAN    0.03f

#define VIEWPORT_DEFAULT__NEAR_CLIPPING_PLANE_DISTANCE 0.001f
#define VIEWPORT_DEFAULT__FAR_CLIPPING_PLANE_DISTANCE 1000.0f

#define fractionOf(x) ((x) - floorf(x))
#define oneMinusFractionOf(x) (1 - fractionOf(x))

INLINE_XPU f32 clampedValue(f32 value, f32 from, f32 to) {
    f32 mn = value < to ? value : to;
    return mn > from ? mn : from;
}

INLINE_XPU i32 clampedValue(i32 value, i32 from, i32 to) {
    i32 mn = value < to ? value : to;
    return mn > from ? mn : from;
}

INLINE_XPU u32 clampedValue(u32 value, u32 from, u32 to) {
    u32 mn = value < to ? value : to;
    return mn > from ? mn : from;
}

INLINE_XPU unsigned int clampedValue(unsigned int value, unsigned int from, unsigned int to) {
    unsigned int mn = value < to ? value : to;
    return mn > from ? mn : from;
}

INLINE_XPU f32 clampedValue(f32 value, f32 to) {
    return value < to ? value : to;
}

INLINE_XPU i32 clampedValue(i32 value, i32 to) {
    return value < to ? value : to;
}

INLINE_XPU f32 clampedValue(f32 value) {
    f32 mn = value < 1.0f ? value : 1.0f;
    return mn > 0.0f ? mn : 0.0f;
}

INLINE_XPU i32 clampedValue(i32 value) {
    i32 mn = value < 1 ? value : 1;
    return mn > 0 ? mn : 0;
}

INLINE_XPU f32 smoothStep(f32 from, f32 to, f32 t) {
    if (t <= from) return 0;
    if (t >= to) return 1;
    t = (t - from) / (to - from);
    return t * t * (3.0f - 2.0f * t);
}

INLINE_XPU f32 smoothStep(f32 t) {
    return smoothStep(0.0f, 1.0f, t);
}

INLINE_XPU f32 approach(f32 src, f32 trg, f32 diff) {
    f32 out;

    out = src + diff; if (trg > out) return out;
    out = src - diff; if (trg < out) return out;

    return trg;
}

template <typename T>
INLINE_XPU void swap(T *a, T *b) {
    T t = *a;
    *a = *b;
    *b = t;
}

INLINE_XPU bool isOnCheckerboard(f32 u, f32 v, u8 steps = 4) {
    return (u8)(v * (f32)steps) % 2 == (u8)(u * (f32)steps) % 2;
}

template <typename T>
struct RangeOf {
    T first, last;

    INLINE_XPU RangeOf() : RangeOf{0, 0} {}
    INLINE_XPU RangeOf(T first, T last) : first{first}, last{last} {}
    INLINE_XPU RangeOf(const RangeOf<T> &other) : RangeOf{other.first, other.last} {}

    INLINE_XPU bool contains(i32 v) const { return (first <= v) && (v <= last); }
    INLINE_XPU bool bounds(i32 v) const { return (first < v) && (v < last); }
    INLINE_XPU bool operator!() const { return last < first; }
    INLINE_XPU bool operator[](T v) const { return contains(v); }
    INLINE_XPU bool operator()(T v) const { return bounds(v); }
    INLINE_XPU void operator+=(T offset) {first += offset; last += offset;}
    INLINE_XPU void operator-=(T offset) {first -= offset; last -= offset;}
    INLINE_XPU void operator*=(T factor) {first *= factor; last *= factor;}
    INLINE_XPU void operator/=(T factor) {factor = 1 / factor; first *= factor; last *= factor;}
    INLINE_XPU void operator-=(const RangeOf<T> &rhs) { sub(rhs.first, rhs.last); }
    INLINE_XPU RangeOf<T> operator-(const RangeOf<T> &rhs) const {
        RangeOf<T> result{first, last};
        result.sub(rhs.first, rhs.last);
        return result;
    }
    INLINE_XPU void sub(T sub_first, T sub_last) {
        if (sub_last < sub_first) {
            T tmp = sub_last;
            sub_last = sub_first;
            sub_first = tmp;
        }
        if (last < sub_first || sub_last < first) {
            first = -1;
            last = -2;
        } else {
            first = first < sub_first ? sub_first : first;
            last = sub_last < last ? sub_last : last;
        }
    }
};
typedef RangeOf<f32> Range;
typedef RangeOf<i32> RangeI;

template <typename T>
struct RectOf {
    union {
        struct {
            T left;
            T right;
            T top;
            T bottom;
        };
        struct {
            RangeOf<T> x_range, y_range;
        };
    };

    INLINE_XPU RectOf(const RectOf<T> &other) : RectOf{other.x_range, other.y_range} {}
    INLINE_XPU RectOf(const RangeOf<T> &x_range, const RangeOf<T> &y_range) : x_range{x_range}, y_range{y_range} {}
    INLINE_XPU RectOf(T left = 0, T right = 0, T top = 0, T bottom = 0) : left{left}, right{right}, top{top}, bottom{bottom} {
        if (right < left) swap(&right, &left);
        if (bottom < top) swap(&top, &bottom);
    }

    INLINE_XPU bool contains(T x, T y) const { return x_range.contains(x) && y_range.contains(y); }
    INLINE_XPU bool bounds(T x, T y) const { return x_range.bounds(x) && y_range.bounds(y); }
    INLINE_XPU bool operator!() const { return !x_range || !y_range; }
    INLINE_XPU bool isOutsideOf(const RectOf<T> &other) {
        return (
                other.right < left || right < other.left ||
                other.bottom < top || bottom < other.top
        );
    }
    INLINE_XPU void operator+=(T offset) {x_range += offset; y_range += offset;}
    INLINE_XPU void operator-=(T offset) {x_range -= offset; y_range -= offset;}
    INLINE_XPU void operator*=(T factor) {x_range *= factor; y_range *= factor;}
    INLINE_XPU void operator/=(T factor) {x_range /= factor; y_range /= factor;}
    INLINE_XPU void operator-=(const RectOf<T> &rhs) { sub(rhs.x_range, rhs.y_range); }
    INLINE_XPU RectOf<T> operator-(const RectOf<T> &rhs) const {
        RectOf<T> result{x_range, y_range};
        result.sub(rhs.x_range, rhs.y_range);
        return result;
    }
    INLINE_XPU void sub(const RangeOf<T> &other_x_range, const RangeOf<T> &other_y_range) {
        x_range -= other_x_range;
        y_range -= other_y_range;
    }
};
typedef RectOf<f32> Rect;
typedef RectOf<i32> RectI;

struct Turn {
    bool right{false};
    bool left{false};
};

struct Move {
    bool right{false};
    bool left{false};
    bool up{false};
    bool down{false};
    bool forward{false};
    bool backward{false};
};

enum CurveType {
    CurveType_None = 0,

    CurveType_Helix,
    CurveType_Coil,
    CurveType_Sphere,

    CurveType_Count
};
struct Curve {
    CurveType type{CurveType_None};
    f32 revolution_count{1}, thickness{0.1f};
};

enum GeometryType {
    GeometryType_None = 0,

    GeometryType_Mesh,
    GeometryType_Grid,
    GeometryType_Box,
    GeometryType_Curve,
    GeometryType_Tet,
    GeometryType_Quad,
    GeometryType_Sphere,
    GeometryType_Light,

    GeometryType_Count
};

enum BoxSide {
    BoxSide_None   = 0,
    BoxSide_Left   = 1,
    BoxSide_Bottom = 2,
    BoxSide_Back   = 4,
    BoxSide_Right  = 8,
    BoxSide_Top    = 16,
    BoxSide_Front  = 32
};


INLINE_XPU void setUVByBoxSide(BoxSide side, f32 X, f32 Y, f32 Z, f32 *u, f32 *v) {
    switch (side) {
        case BoxSide_Top: {
            *u = X;
            *v = Z;
        } break;
        case BoxSide_Bottom: {
            *u = -X;
            *v = -Z;
        } break;
        case BoxSide_Left: {
            *u = -Z;
            *v = Y;
        } break;
        case BoxSide_Right: {
            *u = Z;
            *v = Y;
        } break;
        case BoxSide_Front: {
            *u = X;
            *v = Y;
        } break;
        default: {
            *u = -X;
            *v = Y;
        } break;
    }
    *u = fast_mul_add(*u, 0.5f, 0.5f);
    *v = fast_mul_add(*v, 0.5f, 0.5f);
}

INLINE_XPU void setUVBySphere(f32 X, f32 Y, f32 Z, f32 *u, f32 *v, BoxSide *side = nullptr) {
    f32 z_over_x = X ? (Z / X) : 2;
    f32 y_over_x = X ? (Y / X) : 2;
    if (z_over_x <=  1 &&
        z_over_x >= -1 &&
        y_over_x <=  1 &&
        y_over_x >= -1) { // Right or Left
        *u = z_over_x;
        *v = X > 0 ? y_over_x : -y_over_x;
        if (side) *side = X > 0 ? BoxSide_Right : BoxSide_Left;
    } else {
        f32 x_over_z = Z ? (X / Z) : 2;
        f32 y_over_z = Z ? (Y / Z) : 2;
        if (x_over_z <=  1 &&
            x_over_z >= -1 &&
            y_over_z <=  1 &&
            y_over_z >= -1) { // Front or Back:
            *u = -x_over_z;
            *v = Z > 0 ? y_over_z : -y_over_z;
            if (side) *side = Z > 0 ? BoxSide_Front : BoxSide_Back;
        } else { // Top or Bottom
            *u = X / (Y > 0 ? Y : -Y);
            *v = Z / Y;
            if (side) *side = Y > 0 ? BoxSide_Top : BoxSide_Bottom;
        }
    }
    *u = fast_mul_add(*u, 0.5f, 0.5f);
    *v = fast_mul_add(*v, 0.5f, 0.5f);
}

struct Sides {
    union {
        u8 mask;
        struct {
            unsigned int left:1;
            unsigned int bottom:1;
            unsigned int back:1;
            unsigned int right:1;
            unsigned int top:1;
            unsigned int front:1;
        };
    };

    INLINE_XPU Sides() : mask{0} {}
    INLINE_XPU Sides(u8 mask) : mask{mask} {}
    INLINE_XPU Sides(f32 x, f32 y, f32 z) :
        left{signbit(x)},
        bottom{signbit(y)},
        back{signbit(z)},
        right{!signbit(x)},
        top{!signbit(y)},
        front{!signbit(z)}
    {}

    INLINE_XPU void flip() { mask = ~mask; }
    INLINE_XPU Sides flipped() const {
        Sides sides = *this;
        sides.flip();
        return sides;
    }
};

enum RenderMode {
    RenderMode_Normals,
    RenderMode_NormalMap,
    RenderMode_Beauty,
    RenderMode_Depth,
    RenderMode_MipLevel,
    RenderMode_UVs
};

enum Axis {
    Axis_X = 1,
    Axis_Y = 2,
    Axis_Z = 4
};

struct OctantShifts {
    unsigned int x:2;
    unsigned int y:2;
    unsigned int z:2;

    INLINE_XPU OctantShifts() : x{0}, y{0}, z{0} {}

    INLINE_XPU OctantShifts(Sides sides) :
            x{(unsigned int)(3 * sides.left)},
            y{(unsigned int)(3 * sides.bottom)},
            z{(unsigned int)(3 * sides.back)}
    {}

    INLINE_XPU BoxSide getBoxSide(Axis axis) const {
        return (BoxSide)(
            (axis == Axis_X) * (Axis_X << x) |
            (axis == Axis_Y) * (Axis_Y << y) |
            (axis == Axis_Z) * (Axis_Z << z)
        );
    }
};

template <class T>
struct Orientation : T {
    INLINE_XPU Orientation(f32 x_radians = 0.0f, f32 y_radians = 0.0f, f32 z_radians = 0.0f) {
        setRotation(x_radians, y_radians, z_radians);
    }

    INLINE_XPU void rotate(f32 x_radians, f32 y_radians, f32 z_radians) {
        setRotation(x + x_radians, y + y_radians, x + z_radians);
    }

    INLINE_XPU void rotate(f32 x_radians, f32 y_radians) {
        setRotation(x + x_radians, y + y_radians);
    }

    INLINE_XPU void setRotation(f32 x_radians, f32 y_radians, f32 z_radians) {
        x = x_radians;
        y = y_radians;
        z = z_radians;
        _update();
    }

    INLINE_XPU void setRotation(f32 x_radians, f32 y_radians) {
        x = x_radians;
        y = y_radians;
        _update();
    }

    INLINE_XPU void rotateAroundX(f32 radians) {
        setRotationAroundX(x + radians);
    }

    INLINE_XPU void rotateAroundY(f32 radians) {
        setRotationAroundY(y + radians);
    }

    INLINE_XPU void rotateAroundZ(f32 radians) {
        setRotationAroundZ(z + radians);
    }

    INLINE_XPU void setRotationAroundX(f32 radians) {
        x = radians;
        _update();
    }

    INLINE_XPU void setRotationAroundY(f32 radians) {
        y = radians;
        _update();
    }

    INLINE_XPU void setRotationAroundZ(f32 radians) {
        z = radians;
        _update();
    }

    INLINE_XPU void reset() {
        this->setToIdentity();
        x = y = z = 0.0f;
    }

protected:
    f32 x = 0.0f, y = 0.0f, z = 0.0f;

    INLINE_XPU void _update() {
        this->setToIdentity();
        if (z != 0.0f) *this *= T::RotationAroundZ(z);
        if (x != 0.0f) *this *= T::RotationAroundX(x);
        if (y != 0.0f) *this *= T::RotationAroundY(y);
    }
};


struct ProjectionParams {
    float near_distance;
    float far_distance;
    union {
        float width;
        float focal_length;
    };
    union {
        float height;
        float height_over_width;
    };
    bool is_perspective;
    bool to_cube;

    static ProjectionParams makePerspective(
        float focal_length = 2.0f, 
        float height_over_width = 1.0f,
        float near_distance = 0.001f,
        float far_distance = 100.0f,
        bool to_cube = true) {
        return {near_distance, far_distance, focal_length, height_over_width, true, to_cube};
    }

    static ProjectionParams makeOrthographic(
        float width, 
        float height,
        float near_distance,
        float far_distance,
        bool to_cube = true) {
        return {near_distance, far_distance, width, height, false, to_cube};
    }
};

enum BRDFType {
    BRDF_Lambert,
    BRDF_Blinn,
    BRDF_Phong,
    BRDF_CookTorrance
};

enum ColorID {
    Black,
    White,
    Grey,

    Red,
    Green,
    Blue,

    Cyan,
    Magenta,
    Yellow,

    DarkRed,
    DarkGreen,
    DarkBlue,
    DarkGrey,

    BrightRed,
    BrightGreen,
    BrightBlue,
    BrightGrey,

    BrightCyan,
    BrightMagenta,
    BrightYellow,

    DarkCyan,
    DarkMagenta,
    DarkYellow,

    F0_Default,
    F0_Water,
    F0_Glass_Low,
    F0_Glass_High,
    F0_Plastic,
    F0_PlasticHight,
    F0_Ruby,

    F0_Diamond,
    F0_Iron,
    F0_Copper,
    F0_Gold,
    F0_Aluminium,
    F0_Silver
};

struct ByteColor {
    union {
        struct { u8 B, G, R, A; };
        u8 components[4];
        u32 value;
    };

    INLINE_XPU ByteColor(u8 R = 0, u8 G = 0, u8 B = 0, u8 A = 0) : B{B}, G{G}, R{R}, A{A} {}
    INLINE_XPU ByteColor(f32 r, f32 g, f32 b, f32 a = 0.0f) :
        B{(u8)(b * FLOAT_TO_COLOR_COMPONENT)},
        G{(u8)(g * FLOAT_TO_COLOR_COMPONENT)},
        R{(u8)(r * FLOAT_TO_COLOR_COMPONENT)},
        A{(u8)(a * FLOAT_TO_COLOR_COMPONENT)} {}

    INLINE_XPU ByteColor(u32 value) :
        B{(u8)value},
        G{(u8)(value >> 8)},
        R{(u8)(value >> 16)},
        A{(u8)(value >> 24)} {}
};

// Filmic Tone-Mapping: https://www.slideshare.net/naughty_dog/lighting-shading-by-john-hable
// ====================
// A = Shoulder Strength (i.e: 0.22)
// B = Linear Strength   (i.e: 0.3)
// C = Linear Angle      (i.e: 0.1)
// D = Toe Strength      (i.e: 0.2)
// E = Toe Numerator     (i.e: 0.01)
// F = Toe Denumerator   (i.e: 0.3)
// LinearWhite = Linear White Point Value (i.e: 11.2)
//   Note: E/F = Toe Angle
//   Note: i.e numbers are NOT gamma corrected(!)
//
// f(x) = ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F
//
// FinalColor = f(LinearColor)/f(LinearWhite)
//
// i.e:
// x = max(0, LinearColor-0.004)
// GammaColor = (x*(6.2*x + 0.5))/(x*(6.2*x+1.7) + 0.06)
//
// A = 6.2
// B = 1.7
//
// C*B = 1/2
// D*F = 0.06
// D*E = 0
//
// C = 1/2*1/B = 1/2*1/1.7 = 1/(2*1.7) = 1/3.4 =
// D = 1
// F = 0.06
// E = 0

#define TONE_MAP__SHOULDER_STRENGTH 6.2f
#define TONE_MAP__TOE_STRENGTH 1
#define TONE_MAP__TOE_NUMERATOR 0
#define TONE_MAP__TOE_DENOMINATOR 1
#define TONE_MAP__TOE_ANGLE (TONE_MAP__TOE_NUMERATOR / TONE_MAP__TOE_DENOMINATOR)
#define TONE_MAP__LINEAR_ANGLE (1.0f/3.4f)
#define TONE_MAP__LINEAR_WHITE 1
#define TONE_MAP__LINEAR_STRENGTH 1
// LinearWhite = 1:
// f(LinearWhite) = f(1)
// f(LinearWhite) = (A + C*B + D*E)/(A + B + D*F) - E/F
#define TONE_MAPPED__LINEAR_WHITE ( \
    (                               \
        TONE_MAP__SHOULDER_STRENGTH + \
        TONE_MAP__LINEAR_ANGLE * TONE_MAP__LINEAR_STRENGTH + \
        TONE_MAP__TOE_STRENGTH * TONE_MAP__TOE_NUMERATOR \
    ) / (                           \
        TONE_MAP__SHOULDER_STRENGTH + TONE_MAP__LINEAR_STRENGTH + \
        TONE_MAP__TOE_STRENGTH * TONE_MAP__TOE_DENOMINATOR  \
    ) - TONE_MAP__TOE_ANGLE \
)

INLINE_XPU f32 toneMapped(f32 LinearColor) {
    f32 x = LinearColor - 0.004f;
    if (x < 0.0f) x = 0.0f;
    f32 x2 = x*x;
    f32 x2_times_sholder_strength = x2 * TONE_MAP__SHOULDER_STRENGTH;
    f32 x_times_linear_strength   =  x * TONE_MAP__LINEAR_STRENGTH;
    return (
                   (
                           (
                                   x2_times_sholder_strength + x*x_times_linear_strength + TONE_MAP__TOE_STRENGTH*TONE_MAP__TOE_NUMERATOR
                           ) / (
                                   x2_times_sholder_strength +   x_times_linear_strength + TONE_MAP__TOE_STRENGTH*TONE_MAP__TOE_DENOMINATOR
                           )
                   ) - TONE_MAP__TOE_ANGLE
           ) / (TONE_MAPPED__LINEAR_WHITE);
}


INLINE_XPU f32 gammaCorrected(f32 x) {
    return (x <= 0.0031308f ? (x * 12.92f) : (1.055f * powf(x, 1.0f/2.4f) - 0.055f));
}

INLINE_XPU f32 gammaCorrectedApproximately(f32 x) {
    return powf(x, 1.0f/2.2f);
}
INLINE_XPU f32 toneMappedBaked(f32 LinearColor) {
    // x = max(0, LinearColor-0.004)
    // GammaColor = (x*(6.2*x + 0.5))/(x*(6.2*x+1.7) + 0.06)
    // GammaColor = (x*x*6.2 + x*0.5)/(x*x*6.2 + x*1.7 + 0.06)

    f32 x = LinearColor - 0.004f;
    if (x < 0.0f) x = 0.0f;
    f32 x2_times_sholder_strength = x * x * 6.2f;
    return (x2_times_sholder_strength + x*0.5f)/(x2_times_sholder_strength + x*1.7f + 0.06f);
}

struct EdgeVertexIndices {
    u32 from, to;
};

union TriangleVertexIndices {
    u32 ids[3];
    struct {
        u32 v1, v2, v3;
    };
};

struct Color {
    union {
        struct { f32 red, green, blue; };
        struct { f32 r  , g    , b   ; };
        f32 components[3];
    };

    INLINE_XPU Color() : r{0.0f}, g{0.0f}, b{0.0f} {}
    INLINE_XPU Color(u8 R, u8 G, u8 B) : red{(f32)R * COLOR_COMPONENT_TO_FLOAT}, green{(f32)G * COLOR_COMPONENT_TO_FLOAT}, blue{(f32)B * COLOR_COMPONENT_TO_FLOAT} {}
    INLINE_XPU Color(ByteColor byte_color) : Color{byte_color.R, byte_color.G, byte_color.B} {}
    INLINE_XPU Color(u32 value) : Color{ByteColor{value}} {}
    INLINE_XPU Color(f32 value) : red{value}, green{value}, blue{value} {}
    INLINE_XPU Color(f32 red, f32 green, f32 blue) : red{red}, green{green}, blue{blue} {}
    INLINE_XPU Color(enum ColorID color_id) : Color{0.0f} {
        switch (color_id) {
            case Black: break;
            case White: *this = 1.0f; break;
            case Grey: *this = 0.5f; break;
            case DarkGrey: *this = 0.25f; break;
            case BrightGrey: *this = 0.75f; break;

            case Red: red = 1.0f; break;
            case Green: green = 1.0f; break;
            case Blue: blue = 1.0f; break;

            case DarkRed: red = 0.5f; break;
            case DarkGreen: green = 0.5f; break;
            case DarkBlue: blue = 0.5f; break;

            case DarkCyan: green = 0.5f; blue = 0.5f; break;
            case DarkMagenta: red = 0.5f; blue = 0.5f; break;
            case DarkYellow: red = 0.5f; green = 0.5f; break;

            case BrightRed: red = 1.0f; green = 0.5f; blue = 0.5f; break;
            case BrightGreen: red = 0.5f; green = 1.0f; blue = 0.5f; break;
            case BrightBlue: red = 0.5f; green = 0.5f; blue = 1.0f; break;

            case Cyan: blue = 1.0f; green = 1.0f; break;
            case Magenta: red = 1.0f; blue = 1.0f; break;
            case Yellow: red = 1.0f; green = 1.0f; break;

            case BrightCyan: green = 0.75f; blue = 0.75f; break;
            case BrightMagenta: red = 0.75f; blue = 0.75f; break;
            case BrightYellow: red = 0.75f; green = 0.75f; break;

            // Dialectrics (Fresnel):
            case F0_Default: *this = 0.04f; break;
            case F0_Water: *this = 0.02f; break;
            case F0_Plastic: *this = 0.03f; break;
            case F0_PlasticHight: *this = 0.05f; break;
            case F0_Glass_Low: *this = 0.03f; break;
            case F0_Glass_High: *this = 0.08f; break;
            case F0_Ruby: *this = 0.08f; break;
            case F0_Diamond: *this = 0.17f; break;

            // Conductors (Fresnel):
            case F0_Iron: red = 0.56f; green = 0.57f; blue = 0.58f; break;
            case F0_Copper: red = 0.95f; green = 0.64f; blue = 0.54f; break;
            case F0_Gold: red = 1.0f; green = 0.71f; blue = 0.29f; break;
            case F0_Aluminium: red = 0.91f; green = 0.92f; blue = 0.92f; break;
            case F0_Silver: red = 0.95f; green = 0.93f; blue = 0.88f; break;
        }
    }

    INLINE_XPU Color& operator = (f32 value) {
        r = g = b = value;
        return *this;
    }

    INLINE_XPU void applyToneMapping() {
        r = toneMappedBaked(r);
        g = toneMappedBaked(g);
        b = toneMappedBaked(b);
    }

    INLINE_XPU void applyGamma(f32 gamma = 2.2f) {
        r = powf(r, gamma);
        g = powf(g, gamma);
        b = powf(b, gamma);
    }

    INLINE_XPU Color gammaCorrected(f32 gamma = 2.2f) const {
        Color color{*this};
        color.applyGamma(gamma);
        return color;
    }

    INLINE_XPU ByteColor toByteColor(f32 opacity = 1.0f) const {
        return ByteColor{r, g, b, opacity};
    }

    INLINE_XPU Color clamped() const {
        return {
                clampedValue(r),
                clampedValue(g),
                clampedValue(b)
        };
    }

    INLINE_XPU  void setByHex(i32 hex) {
        r = (float)((0xFF0000 & hex) >> 16) * COLOR_COMPONENT_TO_FLOAT;
        g = (float)((0x00FF00 & hex) >>  8) * COLOR_COMPONENT_TO_FLOAT;
        b = (float)( 0x0000FF & hex)        * COLOR_COMPONENT_TO_FLOAT;
    }

    INLINE_XPU Color& operator = (ColorID color_id) {
        *this  = Color(color_id);
        return *this;
    }

    INLINE_XPU Color& operator = (ByteColor byte_color) {
        *this  = Color(byte_color);
        return *this;
    }

    INLINE_XPU Color& operator = (u32 value) {
        *this  = Color(value);
        return *this;
    }

    INLINE_XPU Color operator + (const Color &rhs) const {
        return {
                r + rhs.r,
                g + rhs.g,
                b + rhs.b
        };
    }

    INLINE_XPU Color operator + (f32 scalar) const {
        return {
                r + scalar,
                g + scalar,
                b + scalar
        };
    }

    INLINE_XPU Color& operator += (const Color &rhs) {
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        return *this;
    }

    INLINE_XPU Color& operator += (f32 scalar) {
        r += scalar;
        g += scalar;
        b += scalar;
        return *this;
    }

    INLINE_XPU Color operator - (const Color &rhs) const {
        return {
                r - rhs.r,
                g - rhs.g,
                b - rhs.b
        };
    }

    INLINE_XPU Color operator - (f32 scalar) const {
        return {
                r - scalar,
                g - scalar,
                b - scalar
        };
    }

    INLINE_XPU Color& operator -= (const Color &rhs) {
        r -= rhs.r;
        g -= rhs.g;
        b -= rhs.b;
        return *this;
    }

    INLINE_XPU Color& operator -= (f32 scalar) {
        r -= scalar;
        g -= scalar;
        b -= scalar;
        return *this;
    }

    INLINE_XPU Color operator * (const Color &rhs) const {
        return {
                r * rhs.r,
                g * rhs.g,
                b * rhs.b
        };
    }

    INLINE_XPU Color operator * (f32 scalar) const {
        return {
                r * scalar,
                g * scalar,
                b * scalar
        };
    }

    INLINE_XPU Color& operator *= (const Color &rhs) {
        r *= rhs.r;
        g *= rhs.g;
        b *= rhs.b;
        return *this;
    }

    INLINE_XPU Color& operator *= (f32 scalar) {
        r *= scalar;
        g *= scalar;
        b *= scalar;
        return *this;
    }

    INLINE_XPU Color operator / (const Color &rhs) const {
        return {
                r / rhs.r,
                g / rhs.g,
                b / rhs.b
        };
    }

    INLINE_XPU Color operator / (f32 scalar) const {
        scalar = 1.0f / scalar;
        return {
                r * scalar,
                g * scalar,
                b * scalar
        };
    }

    INLINE_XPU Color& operator /= (const Color &rhs) {
        r /= rhs.r;
        g /= rhs.g;
        b /= rhs.b;
        return *this;
    }

    INLINE_XPU Color& operator /= (f32 scalar) {
        scalar = 1.0f / scalar;
        r *= scalar;
        g *= scalar;
        b *= scalar;
        return *this;
    }

    INLINE_XPU Color lerpTo(const Color &to, f32 by) const {
        return (to - *this).scaleAdd(by, *this);
    }

    INLINE_XPU Color scaleAdd(f32 factor, const Color &to_be_added) const {
        return {
                fast_mul_add(r, factor, to_be_added.r),
                fast_mul_add(g, factor, to_be_added.g),
                fast_mul_add(b, factor, to_be_added.b)
        };
    }

    INLINE_XPU Color scaleAdd(f32 factor, f32 to_be_added) const {
        return {
                fast_mul_add(r, factor, to_be_added),
                fast_mul_add(g, factor, to_be_added),
                fast_mul_add(b, factor, to_be_added)
        };
    }

    INLINE_XPU Color mulAdd(const Color &factors, const Color &to_be_added) const {
        return {
                fast_mul_add(r, factors.r, to_be_added.r),
                fast_mul_add(g, factors.g, to_be_added.g),
                fast_mul_add(b, factors.b, to_be_added.b)
        };
    }
};

INLINE_XPU Color operator - (f32 lhs, const Color &rhs) {
    return {
            lhs - rhs.r,
            lhs - rhs.g,
            lhs - rhs.b
    };
}

INLINE_XPU Color operator + (f32 lhs, const Color &rhs) {
    return {
            lhs + rhs.r,
            lhs + rhs.g,
            lhs + rhs.b
    };
}

INLINE_XPU Color operator / (f32 lhs, const Color &rhs) {
    return {
            lhs / rhs.r,
            lhs / rhs.g,
            lhs / rhs.b
    };
}

INLINE_XPU Color operator * (f32 lhs, const Color &rhs) {
    return {
            lhs * rhs.r,
            lhs * rhs.g,
            lhs * rhs.b
    };
}

struct Pixel {
    Color color;
    f32 opacity;

    INLINE_XPU Pixel(Color color, f32 opacity = 1.0f) : color{color}, opacity{opacity} {}
    INLINE_XPU Pixel(f32 red = 0.0f, f32 green = 0.0f, f32 blue = 0.0f, f32 opacity = 0.0f) : color{red, green, blue}, opacity{opacity} {}
    INLINE_XPU Pixel(enum ColorID color_id, f32 opacity = 1.0f) : Pixel{Color(color_id), opacity} {}

    INLINE_XPU Pixel(ByteColor byte_color) : color{Color{byte_color}}, opacity{((f32)byte_color.A) * COLOR_COMPONENT_TO_FLOAT} {}
    INLINE_XPU Pixel(u32 value) : Pixel{ByteColor{value}} {}
    INLINE_XPU Pixel(u8 R, u8 G, u8 B, u8 A) : Pixel{ByteColor{R, G, B, A}} {}
    INLINE_XPU Pixel(f32 value, f32 opacity = 1.0f) : color{Color{value, value, value}}, opacity{opacity} {}

    INLINE_XPU Pixel lerpTo(const Pixel &to, f32 by) {
        return {
            color.lerpTo(to.color, by),
            opacity + (to.opacity - opacity) * by
        };
    }

    INLINE_XPU Pixel& operator = (f32 value) {
        color = value;
        opacity = 0.0f;
        return *this;
    }

    INLINE_XPU Pixel& operator = (ColorID color_id) {
        color = color_id;
        opacity = 0.0f;
        return *this;
    }

    INLINE_XPU Pixel& operator = (ByteColor byte_color) {
        color = byte_color;
        opacity = ((f32)byte_color.A) * COLOR_COMPONENT_TO_FLOAT;
        return *this;
    }

    INLINE_XPU Pixel& operator = (u32 value) {
        ByteColor byte_color{value};
        color = byte_color;
        opacity = ((f32)byte_color.A) * COLOR_COMPONENT_TO_FLOAT;;
        return *this;
    }

    INLINE_XPU Pixel operator * (f32 factor) const {
        return {
            color * factor,
            opacity * factor
        };
    }

    INLINE_XPU Pixel operator + (const Pixel &rhs) const {
        return {
            color + rhs.color,
            opacity + rhs.opacity
        };
    }

    INLINE_XPU Pixel& operator += (const Pixel &rhs) {
        color += rhs.color;
        opacity += rhs.opacity;
        return *this;
    }

    INLINE_XPU Pixel& operator *= (const Pixel &rhs) {
        color *= rhs.color;
        opacity *= rhs.opacity;
        return *this;
    }

    INLINE_XPU Pixel alphaBlendOver(const Pixel &background) const {
        return *this + background * (1.0f - opacity);
    }

    INLINE_XPU u32 asContent() const {
        u8 R = (u8)(color.r > 1.0f ? MAX_COLOR_VALUE : (FLOAT_TO_COLOR_COMPONENT * sqrt(color.r)));
        u8 G = (u8)(color.g > 1.0f ? MAX_COLOR_VALUE : (FLOAT_TO_COLOR_COMPONENT * sqrt(color.g)));
        u8 B = (u8)(color.b > 1.0f ? MAX_COLOR_VALUE : (FLOAT_TO_COLOR_COMPONENT * sqrt(color.b)));
        return R << 16 | G << 8 | B;
    }
};

struct TiledGridDimensions {
    u32 width = 0;
    u32 height = 0;
    u32 size = 0;
    u32 stride = 0;

    u32 tile_width = 0;
    u32 tile_height = 0;

    XPU void updateDimensions(u32 Width, u32 Height, u32 Stride = 0) {
        stride = Stride == 0 ? Width : Stride;
        width = Width;
        height = Height;
        size = stride * height;
    }

    XPU void updateTileDimensions(u32 TileWidth, u32 TileHeight) {
        tile_width = TileWidth;
        tile_height = TileHeight;
    }
};

struct TiledGridInfo {
    u32 tile_width = 0;
    u32 tile_height = 0;
    u32 tile_size = 0;
    u32 rows = 0;
    u32 columns = 0;
    u32 right_halo = 0;
    u32 bottom_halo = 0;
    u32 right_column = 0;
    u32 bottom_row = 0;
    u32 right_column_tile_stride = 0;
    u32 right_column_tile_size = 0;
    u32 bottom_row_tile_height = 0;
    u32 bottom_row_tile_size = 0;
    u32 row_size = 0;

    u32 tile_x = 0;
    u32 tile_y = 0;
    u32 column = 0;
    u32 row = 0;
    u32 x = 0;
    u32 y = 0;

    INLINE_XPU TiledGridInfo(const TiledGridDimensions &dim) :
            tile_width{dim.tile_width},
            tile_height{dim.tile_height},
            tile_size{dim.tile_width * dim.tile_height},
            rows{   ((dim.height - 1) / dim.tile_height) + 1},
            columns{((dim.width  - 1) / dim.tile_width ) + 1},
            right_column{ dim.stride % dim.tile_width},
            right_halo{ dim.stride % dim.tile_width},
            bottom_halo{dim.height % dim.tile_height}
    {
        right_column = columns - 1;
        bottom_row = rows - 1;
        right_column_tile_stride = right_halo == 0 ? tile_width : right_halo;
        right_column_tile_size = right_column_tile_stride * tile_height;
        bottom_row_tile_height = bottom_halo == 0 ? tile_height : bottom_halo;
        bottom_row_tile_size = tile_width * bottom_row_tile_height;
        row_size = right_column * tile_size + right_column_tile_size;
    }

    INLINE_XPU void setCoords(u32 X, u32 Y) {
        x = X;
        y = Y;
        tile_x = x % tile_width;
        tile_y = y % tile_height;
        column = x / tile_width;
        row    = y / tile_height;
    }

    INLINE_XPU void updateGlobalCoords() {
        x = tile_x + column * tile_width;
        y = tile_y + row * tile_height;
    }

    INLINE_XPU u32 getOffset() const {
        u32 row_tile_stride = column == right_column ? right_column_tile_stride : tile_width;
        u32 row_tile_size = row == bottom_row ? bottom_row_tile_size : tile_size;
        return row * row_size + column * row_tile_size + row_tile_stride * tile_y + tile_x;
    }

    INLINE_XPU u32 getOffset(u32 X, u32 Y) {
        setCoords(X, Y);
        return getOffset();
    }
};

union ImageFlags {
    struct {
        unsigned int alpha:1;
        unsigned int linear:1;
        unsigned int tile:1;
        unsigned int channel:1;
        unsigned int mipmap:1;
        unsigned int flip:1;
        unsigned int wrap:1;
        unsigned int normal:1;
        unsigned int cubemap:1;
    };
    u32 flags = 0;
};

struct ImageInfo : TiledGridDimensions {
    u32 mip_count = 0;
    ImageFlags flags;
};


template <typename T>
struct Image : ImageInfo {
    T* content = nullptr;
    INLINE_XPU T* operator[] (int row) const { return content + row*(flags.channel ? (width * (flags.alpha ? 4 : 3)) : width); }
};

struct PixelImage : Image<Pixel> {};
struct FloatImage : Image<f32> {};
struct ByteColorImage : Image<ByteColor> {};
struct RawImage : Image<u8> {};

union CubeMapImages {
	struct {
		RawImage pos_x;
		RawImage neg_x;
		RawImage pos_y;
		RawImage neg_y;
		RawImage pos_z;
		RawImage neg_z;
	};
	RawImage array[6];

    CubeMapImages() {}
};

union CubeMapSet {
	struct {
		CubeMapImages skybox;
		CubeMapImages radiance;
		CubeMapImages irradiance;
	};
	CubeMapImages array[3];

    CubeMapSet() {}
};


#define PIXEL_SIZE (sizeof(Pixel))
#define CANVAS_PIXELS_SIZE (MAX_WINDOW_SIZE * PIXEL_SIZE * 4)
#define CANVAS_DEPTHS_SIZE (MAX_WINDOW_SIZE * sizeof(f32) * 4)
#define CANVAS_SIZE (CANVAS_PIXELS_SIZE + CANVAS_DEPTHS_SIZE)

struct Dimensions {
    u32 width_times_height;
    f32 width_over_height;
    f32 height_over_width;
    f32 f_width;
    f32 f_height;
    f32 h_width;
    f32 h_height;
    u16 width;
    u16 height;
    u16 stride;

    void update(u16 Width, u16 Height) {
        width = Width;
        height = Height;
        stride = Width;
        width_times_height = width * height;
        f_width  = (f32)width;
        f_height = (f32)height;
        h_width  = f_width  * 0.5f;
        h_height = f_height * 0.5f;
        width_over_height  = f_width  / f_height;
        height_over_width  = f_height / f_width;
    }
};

namespace controls {
    namespace key_map {
        u8 ctrl{0};
        u8 alt{0};
        u8 shift{0};
        u8 space{0};
        u8 tab{0};
        u8 escape{0};
        u8 left{0};
        u8 right{0};
        u8 up{0};
        u8 down{0};
    }

    namespace is_pressed {
        bool ctrl{false};
        bool alt{false};
        bool shift{false};
        bool space{false};
        bool tab{false};
        bool escape{false};
        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};
    }
}

namespace os {
    void* getMemory(u64 size, u64 base = 0);
    void freeMemory(void* memory);
    void setWindowTitle(char* str);
    void setWindowCapture(bool on);
    void setCursorVisibility(bool on);
    void closeFile(void *handle);
    void* openFileForReading(const char* file_path);
    void* openFileForWriting(const char* file_path);
    bool readFromFile(void *out, unsigned long, void *handle);
    bool writeToFile(void *out, unsigned long, void *handle);
    void print(const char *message, u8 color);
    void printError(const char *message, u8 color);
    long long int getFileSizeWithoutOpening(const char* path);
    long long int getFileSize(void *handle);
    void* readEntireFile(const char* file_path, u64 *out_size);
}

namespace timers {
    u64 getTicks();
    u64 ticks_per_second;
    f64 seconds_per_tick;
    f64 milliseconds_per_tick;
    f64 microseconds_per_tick;
    f64 nanoseconds_per_tick;

    struct Timer {
        f32 delta_time{0};

        u64 ticks_before{0};
        u64 ticks_after{0};
        u64 ticks_diff{0};
        u64 ticks_of_last_report{0};

        u64 accumulated_ticks{0};
        u64 accumulated_frame_count{0};

        u64 seconds{0};
        u64 milliseconds{0};
        u64 microseconds{0};
        u64 nanoseconds{0};

        f64 average_frames_per_tick{0.0};
        f64 average_ticks_per_frame{0.0};
        u16 average_frames_per_second{0};
        u16 average_milliseconds_per_frame{0};
        u16 average_microseconds_per_frame{0};
        u16 average_nanoseconds_per_frame{0};

        Timer() noexcept : ticks_before{getTicks()}, ticks_after{getTicks()}, ticks_of_last_report{getTicks()} {};

        INLINE void accumulate() {
            ticks_diff = ticks_after - ticks_before;
            accumulated_ticks += ticks_diff;
            accumulated_frame_count++;

            seconds = (u64) (seconds_per_tick * (f64) (ticks_diff));
            milliseconds = (u64) (milliseconds_per_tick * (f64) (ticks_diff));
            microseconds = (u64) (microseconds_per_tick * (f64) (ticks_diff));
            nanoseconds = (u64) (nanoseconds_per_tick * (f64) (ticks_diff));
        }

        INLINE void average() {
            average_frames_per_tick = (f64) accumulated_frame_count / (f64) accumulated_ticks;
            average_ticks_per_frame = (f64) accumulated_ticks / (f64) accumulated_frame_count;
            average_frames_per_second = (u16) (average_frames_per_tick * (f64) ticks_per_second);
            average_milliseconds_per_frame = (u16) (average_ticks_per_frame * milliseconds_per_tick);
            average_microseconds_per_frame = (u16) (average_ticks_per_frame * microseconds_per_tick);
            average_nanoseconds_per_frame = (u16) (average_ticks_per_frame * nanoseconds_per_tick);
            accumulated_ticks = accumulated_frame_count = 0;
        }

        INLINE void beginFrame() {
            ticks_before = getTicks();
            ticks_diff = ticks_before - ticks_after;
            delta_time = (f32) ((f64) ticks_diff * seconds_per_tick);
        }

        INLINE void endFrame() {
            ticks_after = getTicks();
            accumulate();
            if (accumulated_ticks >= (ticks_per_second / 8))
                average();
        }
    };
}

namespace mouse {
    struct Button {
        i32 down_pos_x, down_pos_y, up_pos_x, up_pos_y, double_click_pos_x, double_click_pos_y;

        bool is_pressed{false}, double_clicked{false};

        void down(i32 x, i32 y) {
            is_pressed = true;

            down_pos_x = x;
            down_pos_y = y;
        }

        void up(i32 x, i32 y) {
            is_pressed = false;

            up_pos_x = x;
            up_pos_y = y;
        }

        void doubleClick(i32 x, i32 y) {
            double_clicked = true;
            double_click_pos_x = x;
            double_click_pos_y = y;
        }
    };

    Button middle_button, right_button, left_button;

    i32 pos_x, pos_y, pos_raw_diff_x, pos_raw_diff_y, movement_x, movement_y;
    f32 wheel_scroll_amount{0};

    bool moved{false};
    bool is_captured{false};
    bool double_clicked{false};
    bool wheel_scrolled{false};

    void resetChanges() {
        moved = false;
        double_clicked = false;
        wheel_scrolled = false;
        wheel_scroll_amount = 0;
        pos_raw_diff_x = 0;
        pos_raw_diff_y = 0;
        right_button.double_clicked = false;
        left_button.double_clicked = false;
        middle_button.double_clicked = false;
    }

    void scroll(f32 amount) {
        wheel_scroll_amount += amount * 100;
        wheel_scrolled = true;
    }

    void setPosition(i32 x, i32 y) {
        pos_x = x;
        pos_y = y;
    }

    void move(i32 x, i32 y) {
        movement_x = x - pos_x;
        movement_y = y - pos_y;
        moved = true;
    }

    void moveRaw(i32 x, i32 y) {
        pos_raw_diff_x += x;
        pos_raw_diff_y += y;
        moved = true;
    }
}

namespace memory {
    u8 *canvas_memory{nullptr};
    u64 canvas_memory_capacity = CANVAS_SIZE * CANVAS_COUNT;

    typedef void* (*AllocateMemory)(u64 size);

    struct MonotonicAllocator {
        u8* address{nullptr};
        u64 capacity{0};
        u64 occupied{0};

        MonotonicAllocator() = default;

        explicit MonotonicAllocator(u64 Capacity, u64 starting = 0) {
            capacity = Capacity;
            address = (u8*)os::getMemory(Capacity, starting);
        }

        void* allocate(u64 size) {
            if (!address) return nullptr;
            occupied += size;
            if (occupied > capacity) return nullptr;

            void* current_address = address;
            address += size;
            return current_address;
        }

        void releaseMemory() {
            address -= occupied;
            os::freeMemory(address);
            capacity = occupied = 0;
            address = nullptr;
        }
    };
}

namespace window {
    u16 width{DEFAULT_WIDTH};
    u16 height{DEFAULT_HEIGHT};
    char* title{(char*)""};
    u32 *content{nullptr};
}

void writeHeader(const ImageInfo &info, void *file) {
    os::writeToFile((void*)&info,  sizeof(info),  file);
}
void readHeader(ImageInfo &info, void *file) {
    os::readFromFile(&info,  sizeof(info),  file);
}

template <typename T>
bool saveHeader(const T &value, char *file_path) {
    void *file = os::openFileForWriting(file_path);
    if (!file) return false;
    writeHeader(value, file);
    os::closeFile(file);
    return true;
}

template <typename T>
bool loadHeader(T &value, char *file_path) {
    void *file = os::openFileForReading(file_path);
    if (!file) return false;
    readHeader(value, file);
    os::closeFile(file);
    return true;
}

template <typename T>
bool saveContent(const T &value, char *file_path) {
    void *file = os::openFileForWriting(file_path);
    if (!file) return false;
    writeContent(value, file);
    os::closeFile(file);
    return true;
}

template <typename T>
bool loadContent(T &value, char *file_path) {
    void *file = os::openFileForReading(file_path);
    if (!file) return false;
    readContent(value, file);
    os::closeFile(file);
    return true;
}

template <typename T>
bool save(const T &value, char* file_path) {
    void *file = os::openFileForWriting(file_path);
    if (!file) return false;
    writeHeader(value, file);
    writeContent(value, file);
    os::closeFile(file);
    return true;
}

template <typename T>
bool load(T &value, char *file_path, memory::MonotonicAllocator *memory_allocator = nullptr) {
    void *file = os::openFileForReading(file_path);
    if (!file) return false;

    if (memory_allocator) {
        new(&value) T{};
        readHeader(value, file);
        if (!allocateMemory(value, memory_allocator)) return false;
    }
    readContent(value, file);
    os::closeFile(file);
    return true;
}