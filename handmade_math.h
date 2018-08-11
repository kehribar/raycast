// ----------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------
// Heavily copied & pasted from
//   github.com/rsvargas/handmade_hero/blob/master/code/handmade_math.h
// ----------------------------------------------------------------------------
// Original source is: https://hero.handmade.network/episode/ray/
// ----------------------------------------------------------------------------
#ifndef HANDMADE_MATH
#define HANDMADE_MATH

// ----------------------------------------------------------------------------
#include <stdint.h>
#include <math.h>

// ----------------------------------------------------------------------------
typedef union{
  struct
  {
    float x;
    float y;
    float z;
  };
  struct
  {
    float r;
    float g;
    float b;
  };
  float mem[3];
}v3;

// ----------------------------------------------------------------------------
v3 V3(float x, float y, float z)
{
  v3 rv = {{x, y, z}};
  return rv;
}

// ----------------------------------------------------------------------------
float Clamp(float Min, float Value, float Max)
{
  float Result = Value;

  if(Result < Min)
  {
    Result = Min;
  }
  else if(Result > Max)
  {
    Result = Max;
  }

  return Result;
}

// ----------------------------------------------------------------------------
float Clamp01(float Value)
{
  float Result = Clamp(0.0f, Value, 1.0f);
  return Result;
}

// ----------------------------------------------------------------------------
v3 operator*(float A, v3 B)
{
  v3 Result;

  Result.x = A * B.x;
  Result.y = A * B.y;
  Result.z = A * B.z;

  return Result;
}

// ----------------------------------------------------------------------------
v3 operator*(v3 B, float A)
{
  v3 Result = A * B;

  return Result;
}

// ----------------------------------------------------------------------------
v3 &operator*=(v3 &A, float B)
{
  A = B * A;

  return A;
}

// ----------------------------------------------------------------------------
v3 operator-(v3 A)
{
  v3 Result;

  Result.x = -A.x;
  Result.y = -A.y;
  Result.z = -A.z;

  return Result;
}

// ----------------------------------------------------------------------------
v3 operator+(v3 A, v3 B)
{
  v3 Result;

  Result.x = A.x + B.x;
  Result.y = A.y + B.y;
  Result.z = A.z + B.z;

  return Result;
}

// ----------------------------------------------------------------------------
v3 &operator+=(v3 &A, v3 B)
{
  A = A + B;

  return A;
}

// ----------------------------------------------------------------------------
v3 operator-(v3 A, v3 B)
{
  v3 Result;

  Result.x = A.x - B.x;
  Result.y = A.y - B.y;
  Result.z = A.z - B.z;

  return Result;
}

// ----------------------------------------------------------------------------
v3 Hadamard(v3 A, v3 B)
{
  v3 Result = {{A.x * B.x, A.y * B.y, A.z * B.z}};

  return Result;
}

// ----------------------------------------------------------------------------
float Inner(v3 A, v3 B)
{
  float Result = A.x * B.x + A.y * B.y + A.z * B.z;
  return Result;
}

// ----------------------------------------------------------------------------
float LengthSq(v3 A)
{
  // Length is the square root of the inner product!
  float Result = Inner(A, A);
  return Result;
}

// ----------------------------------------------------------------------------
float Length(v3 A)
{
  float Result = sqrtf(LengthSq(A));
  return Result;
}

// ----------------------------------------------------------------------------
v3 Normalize(v3 A)
{
  v3 Result = A * (1.0f / Length(A));
  return Result;
}

// ----------------------------------------------------------------------------
v3 Cross(v3 A, v3 B)
{
  v3 Result;

  Result.x = (A.y * B.z) - (A.z * B.y);
  Result.y = (A.z * B.x) - (A.x * B.z);
  Result.z = (A.x * B.y) - (A.y * B.x);

  return Result;
}

// ----------------------------------------------------------------------------
v3 Clamp01(v3 value)
{
  v3 Result;
  Result.x = Clamp01(value.x);
  Result.y = Clamp01(value.y);
  Result.z = Clamp01(value.z);

  return Result;
}

// ----------------------------------------------------------------------------
v3 Lerp(v3 A, float t, v3 B)
{
  v3 Result = (1.0f - t) * A + t * B;
  return Result;
}

// ----------------------------------------------------------------------------
#endif
