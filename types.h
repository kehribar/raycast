// ----------------------------------------------------------------------------
// 
// See main.cpp ...
// 
// ----------------------------------------------------------------------------
#ifndef TYPES_H
#define TYPES_H

// ----------------------------------------------------------------------------
#include "handmade_math.h"

// ----------------------------------------------------------------------------
typedef struct{
  float Scatter; // 0 is pure diffuse (chalk), 1 is pure specular (mirror)
  v3 EmitColor;
  v3 RefColor;
}material;

// ----------------------------------------------------------------------------
typedef struct{
  v3 P;
  float r;
  uint32_t MatIndex;
}sphere;

// ----------------------------------------------------------------------------
typedef struct{
  v3 P0;
  v3 P1;
  uint32_t MatIndex;
}box; // AABB: 'Axis Aligned Boundary Box'

// ----------------------------------------------------------------------------
typedef struct{
  v3 N;
  float d;
  uint32_t MatIndex;  
}plane;

// ----------------------------------------------------------------------------
typedef struct{
  uint32_t MaterialCount;
  material* Materials;

  uint32_t PlaneCount;
  plane* Planes;

  uint32_t SphereCount;
  sphere* Spheres;

  uint32_t BoxCount;
  box* Boxes;
}world;

// ----------------------------------------------------------------------------
#endif
