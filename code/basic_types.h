#include <stdint.h>
#include <math.h>

#if BUILD_DEBUG
#define Assert(expression) if(!(expression)) { *(int*)0 = 0; }
#else
#define Assert(expression)
#endif

#define Kilobytes(value) ((value) * 1024ULL)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)
#define Terabytes(value) (Gigabytes(value) * 1024)

#define function static
#define global   static
#define local    static

typedef uint32_t bool32;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float  float32;
typedef double float64;

typedef char    char8;
typedef wchar_t char16;

union vec2
{
    struct
    {
        float32 x;
        float32 y;
    };
    struct
    {
        float32 width;
        float32 height;
    };

    const float32 operator[](int32 index) const
    {
        // TODO: Is current implementation slower than: return element[index];
        return ((float32*)this)[index];
    }
    float32& operator[](int32 index)
    {
        // TODO: Is current implementation slower than: return element[index];
        return ((float32*)this)[index];
    }
};

inline vec2 operator+(vec2 first_summand, vec2 second_summand)
{
    vec2 sum;

    sum.x = first_summand.x + second_summand.x;
    sum.y = first_summand.y + second_summand.y;

    return sum;
}

inline vec2& operator+=(vec2& first_summand, vec2 second_summand)
{
    vec2& sum = first_summand;

    sum.x = first_summand.x + second_summand.x;
    sum.y = first_summand.y + second_summand.y;

    return sum;
}

inline vec2 operator-(vec2 minuend, vec2 subtrahend)
{
    vec2 difference;

    difference.x = minuend.x - subtrahend.x;
    difference.y = minuend.y - subtrahend.y;

    return difference;
}

inline vec2& operator-=(vec2& minuend, vec2 subtrahend)
{
    vec2& difference = minuend;

    difference.x = minuend.x - subtrahend.x;
    difference.y = minuend.y - subtrahend.y;

    return difference;
}

inline vec2 operator-(vec2 vector)
{
    vec2 opposite_vector;

    opposite_vector.x = -vector.x;
    opposite_vector.y = -vector.y;

    return opposite_vector;
}

inline vec2 operator*(float32 scalar, vec2 vector)
{
    vec2 result;

    result.x = scalar * vector.x;
    result.y = scalar * vector.y;

    return result;
}

inline vec2 operator*(vec2 vector, float32 scalar)
{
    vec2 result;

    result.x = scalar * vector.x;
    result.y = scalar * vector.y;

    return result;
}

inline vec2& operator*=(vec2& vector, float32 scalar)
{
    vec2& result = vector;

    result.x = scalar * vector.x;
    result.y = scalar * vector.y;

    return result;
}

#define PI32 3.14159265359f

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

inline int32 round_float32_to_int32(float32 number)
{
    return (int32)(number + 0.5f);
}

inline float32 sin(float32 number)
{
    return sinf(number);
}

inline float32 cos(float32 number)
{
    return cosf(number);
}

inline float32 sqrt(float32 number)
{
    return (float32)sqrt((float64)number);
}

inline float32 length_square(vec2 vector)
{
    return vector.x * vector.x + vector.y * vector.y;
}

inline float32 length(vec2 vector)
{
    return sqrt(vector.x * vector.x + vector.y * vector.y);
}

inline vec2 normalize(vec2 vector)
{
    float32 length = ::length(vector);
    return vec2{vector.x / length, vector.y / length};
}

inline float32 dot_product(vec2 first_vector, vec2 second_vector)
{
    return first_vector.x * second_vector.x + first_vector.y * second_vector.y;
}
