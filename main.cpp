// ----------------------------------------------------------------------------
// 
// Simple (?) raycaster written by watching 'Handmade Hero' Raycaster series
// and manually copying the code pieces from video by hand. Not an exact copy
// but the core algorithm is based on the video version.
// 
// More of an 'active learning' example.
// 
// Improvements (?)
// ----------------
// * 'RayIntersectBox' method added to original core code. It is probably
// not the most efficient way but it _seems_ like working.
// 
// ----------------------------------------------------------------------------
// Original source is: https://hero.handmade.network/episode/ray/
// ----------------------------------------------------------------------------
#define STB_IMAGE_WRITE_IMPLEMENTATION

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <stdbool.h>
#include "stb_image_write.h"
#include "handmade_math.h"
#include "types.h"

// ----------------------------------------------------------------------------
#define FILENAME "test.png"

// ----------------------------------------------------------------------------
#define IMAGE_SIZE_N 4
#define IMAGE_WIDTH (320 * IMAGE_SIZE_N)
#define IMAGE_HEIGHT (240 * IMAGE_SIZE_N)
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 3)
#define img_ind(col, row) ((3 * IMAGE_WIDTH * row) + (3 * col))

// ----------------------------------------------------------------------------
float RandomBilateral();
float RandomUnilateral();
int ImgWrite(float* img);
float ExactLinearTosRGB(float L);
void ImgPutColor(float* img, int x, int y, v3 rgb);

// ----------------------------------------------------------------------------
float RayIntersectPlane(
  v3 RayOrigin, v3 RayDirection, plane* Plane
)
{
  float Result = FLT_MAX;
  float Tolerance = 0.0001f;
  float Denom = Inner(Plane->N, RayDirection);

  if((Denom > Tolerance) || (Denom < -Tolerance))
  {
    Result = -Plane->d - Inner(Plane->N, RayOrigin);
    Result /= Denom;
  }

  return Result;
}

// ----------------------------------------------------------------------------
float RayIntersectSphere(
  v3 RayOrigin, v3 RayDirection, sphere* Sphere
)
{
  float Result = FLT_MAX;

  v3 SphereRelativeRayOrigin = RayOrigin - (Sphere->P);
  float a = Inner(RayDirection, RayDirection);
  float b = 2.0f * Inner(RayDirection, SphereRelativeRayOrigin);
  float c = Inner(SphereRelativeRayOrigin, SphereRelativeRayOrigin) - (Sphere->r * Sphere->r);

  float Denom = 2.0f * a;
  float Tolerance = 0.000001f;
  float RootTerm = sqrtf((b * b) - (4.0f * a * c));
   
  if(RootTerm > Tolerance)
  {
    float tp = (-b + RootTerm) / Denom; 
    float tn = (-b - RootTerm) / Denom; 

    Result = tp;
    if((tn > 0) && (tn < tp))
    {
      Result = tn;
    }
  }

  return Result;
}

// ----------------------------------------------------------------------------
float RayIntersectBox(
  v3 RayOrigin, v3 RayDirection, box* Box, v3* HitNormal 
)
{
  float Result = FLT_MAX;

  float xMin = Box->P0.x;
  float xMax = Box->P1.x;
  float yMin = Box->P0.y;
  float yMax = Box->P1.y;
  float zMin = Box->P0.z;
  float zMax = Box->P1.z;

  // Inverse distance calculation
  float t_xMin = (xMin - RayOrigin.x) / RayDirection.x;
  float t_xMax = (xMax - RayOrigin.x) / RayDirection.x;
  float t_yMin = (yMin - RayOrigin.y) / RayDirection.y;
  float t_yMax = (yMax - RayOrigin.y) / RayDirection.y;
  float t_zMin = (zMin - RayOrigin.z) / RayDirection.z;
  float t_zMax = (zMax - RayOrigin.z) / RayDirection.z;

  // Probable x,y,z hit locations
  float x;
  float y;
  float z;

  if(t_xMin > 0)
  {
    y = RayOrigin.y + (RayDirection.y * t_xMin);
    z = RayOrigin.z + (RayDirection.z * t_xMin);

    if((y <= yMax) && (y >= yMin) && (z <= zMax) && (z >= zMin))
    {
      if(t_xMin < Result)
      {
        Result = t_xMin;
        (*HitNormal) = {-1, 0, 0};
      }
    }
  }

  if(t_xMax > 0)
  {
    y = RayOrigin.y + (RayDirection.y * t_xMax);
    z = RayOrigin.z + (RayDirection.z * t_xMax);

    if((y <= yMax) && (y >= yMin) && (z <= zMax) && (z >= zMin))
    {
      if(t_xMax < Result)
      {
        Result = t_xMax;
        (*HitNormal) = {1, 0, 0};
      }
    }
  }

  if(t_yMin > 0)
  {
    x = RayOrigin.x + (RayDirection.x * t_yMin);
    z = RayOrigin.z + (RayDirection.z * t_yMin);

    if((x <= xMax) && (x >= xMin) && (z <= zMax) && (z >= zMin))  
    {
      if(t_yMin < Result)
      {
        Result = t_yMin;
        (*HitNormal) = {0, -1, 0};  
      }
    }
  }

  if(t_yMax > 0)
  {
    x = RayOrigin.x + (RayDirection.x * t_yMax);
    z = RayOrigin.z + (RayDirection.z * t_yMax);

    if((x <= xMax) && (x >= xMin) && (z <= zMax) && (z >= zMin))
    {
      if(t_yMax < Result)
      {
        Result = t_yMax;
        (*HitNormal) = {0, 1, 0};
      }
    }
  }

  if(t_zMin > 0)
  {
    y = RayOrigin.y + (RayDirection.y * t_zMin);
    x = RayOrigin.x + (RayDirection.x * t_zMin);

    if((x <= xMax) && (x >= xMin) && (y <= yMax) && (y >= yMin))
    {
      if(t_zMin < Result)
      {
        Result = t_zMin;
        (*HitNormal) = {0, 0, -1};
      }
    }
  }

  if(t_zMax > 0)
  {
    y = RayOrigin.y + (RayDirection.y * t_zMax);
    x = RayOrigin.x + (RayDirection.x * t_zMax);

    if((x <= xMax) && (x >= xMin) && (y <= yMax) && (y >= yMin))      
    {
      if(t_zMax < Result)
      {
        Result = t_zMax;
        (*HitNormal) = {0, 0, 1};
      }
    }
  }

  return Result;
}

// ----------------------------------------------------------------------------
v3 RayCast(world* World, v3 RayOrigin, v3 RayDirection)
{
  v3 Result = {}; 
  v3 Attenuation = V3(1, 1, 1);

  uint32_t BounceCountMax = 8;
  float MinHitDistance = 0.0001f;

  // We have maximum bounce limit
  for(uint32_t BounceCount = 0; BounceCount < BounceCountMax; BounceCount++)
  {
    float HitDistance = FLT_MAX;
    uint32_t HitMatIndex = 0;
    v3 NextOrigin;
    v3 NextNormal;

    // Check planes
    for(uint32_t i=0;i<(World->PlaneCount);i++)
    {
      plane Plane = World->Planes[i];

      float t = RayIntersectPlane(
        RayOrigin, RayDirection, &Plane
      );

      if((t > MinHitDistance) && (t < HitDistance))
      {
        HitDistance = t;
        HitMatIndex = Plane.MatIndex;

        NextOrigin = RayOrigin + (t * RayDirection);
        NextNormal = Plane.N; 
      }
    }  

    // Check spheres
    for(uint32_t i=0;i<(World->SphereCount);i++)
    {
      sphere Sphere = World->Spheres[i];

      float t = RayIntersectSphere(
        RayOrigin, RayDirection, &Sphere
      );

      if((t > MinHitDistance) && (t < HitDistance))
      {
        HitDistance = t;
        HitMatIndex = Sphere.MatIndex;

        NextOrigin = RayOrigin + (t * RayDirection);
        NextNormal = Normalize(NextOrigin - Sphere.P);
      }
    }

    // Check boxes
    for(uint32_t i=0;i<(World->BoxCount);i++)
    {
      v3 HitNormalMaybe = {};
      box Box = World->Boxes[i];

      float t = RayIntersectBox(
        RayOrigin, RayDirection, &Box, &HitNormalMaybe
      );

      if((t > MinHitDistance) && (t < HitDistance))
      {
        HitDistance = t;
        HitMatIndex = Box.MatIndex;

        NextOrigin = RayOrigin + (t * RayDirection);
        NextNormal = HitNormalMaybe;
      }
    }

    // Process the hit
    if(HitMatIndex)
    {
      material Mat = World->Materials[HitMatIndex];
      Result += Hadamard(Attenuation, Mat.EmitColor);
#if 0
      float CosAtten = Inner(-RayDirection, NextNormal);
      if(CosAtten < 0)
      {
        CosAtten = 0;
      }
#else
      float CosAtten = 1.0f;
#endif
      Attenuation = Hadamard(Attenuation, CosAtten * Mat.RefColor);

      RayOrigin = NextOrigin;

      v3 PureBounce = 
        RayDirection - (2.0f * Inner(RayDirection, NextNormal) * NextNormal);

      v3 RandomBounce = Normalize(
        NextNormal + V3(RandomBilateral(), RandomBilateral(), RandomBilateral())
      );

      RayDirection = Normalize(Lerp(RandomBounce, Mat.Scatter, PureBounce));
    }
    else
    {
      material Mat = World->Materials[HitMatIndex];
      Result += Hadamard(Attenuation, Mat.EmitColor);

      break;
    }
  }

  return Result;
}

// ----------------------------------------------------------------------------
int main(int argc, char const *argv[])
{
  static float img[IMAGE_SIZE]; 

  printf("Program init\n");

  // Colors etc. for world elements
  static material Materials[9];
  Materials[0].EmitColor = V3(0.3,0.4,0.5);
  Materials[1].RefColor = V3(0.3,0.3,0.3);
  Materials[1].Scatter = 0.0f;
  Materials[2].RefColor = V3(0.7,0.5,0.3);
  Materials[2].Scatter = 0.91; 
  Materials[3].RefColor = V3(0.3,0.2,0.2);
  Materials[3].Scatter = 0.92;
  Materials[4].RefColor = V3(0.2,0.8,0.2);
  Materials[4].Scatter = 0.93;
  Materials[5].RefColor = V3(0.4,0.5,0.8);
  Materials[5].Scatter = 0.94;
  Materials[6].RefColor = V3(0.9,0.5,0.5);
  Materials[6].Scatter = 0.95;
  Materials[7].RefColor = V3(0.3,0.7,0.8);
  Materials[7].Scatter = 0.96;
  Materials[8].RefColor = V3(0.8,0.6,0.5);
  Materials[8].EmitColor = V3(0.1,0.1,0.1);
  Materials[8].Scatter = 0.97;

  // We have only one plane
  plane Plane = {};
  Plane.N = Normalize(V3(0,0,1));
  Plane.d = 0.5;
  Plane.MatIndex = 1;

  // Multiple spheres ...
  sphere Spheres[5];
  Spheres[0].P = V3(1, -5, -0.2);
  Spheres[0].r = 2.0f;
  Spheres[0].MatIndex = 2;
  Spheres[1].P = V3(-20, 8, 1.1);
  Spheres[1].r = 4.0f;
  Spheres[1].MatIndex = 3;
  Spheres[2].P = V3(7.0, -1, 2.0);
  Spheres[2].r = 1.0;
  Spheres[2].MatIndex = 4;
  Spheres[3].P = V3(0, 30, 4);
  Spheres[3].r = 10;
  Spheres[3].MatIndex = 5;
  Spheres[4].P = V3(-4, 10, 3.5);
  Spheres[4].r = 1.0;
  Spheres[4].MatIndex = 4;

  // Lets put some 'Axis Aligned' boxes
  box Boxes[3];
  Boxes[0].P0 = V3(-2.5, 5, 0.2);
  Boxes[0].P1 = Boxes[0].P0 + V3(2,2,2);
  Boxes[0].MatIndex = 5;
  Boxes[1].P0 = V3(5, 10, 0.5);
  Boxes[1].P1 = Boxes[1].P0 + V3(3,3,3);
  Boxes[1].MatIndex = 6;
  Boxes[2].P0 = V3(-30, 30, 0.5);
  Boxes[2].P1 = Boxes[2].P0 + V3(8,8,8);
  Boxes[2].MatIndex = 8;

  // Pack everything into the world struct
  world World = {};
  World.MaterialCount = 9;
  World.Materials = Materials;
  World.PlaneCount = 1; 
  World.Planes = &Plane;
  World.SphereCount = 5;
  World.Spheres = Spheres;
  World.BoxCount = 3;
  World.Boxes = Boxes;

  // Calculate camera position and direction vectors
  v3 CameraP = V3(9,-20,6);
  v3 CameraZ = Normalize(CameraP);
  v3 CameraX = Normalize(Cross(V3(0,0,1), CameraZ));
  v3 CameraY = Normalize(Cross(CameraZ, CameraX));

  // Correction for aspect ratio
  float FilmW = 1.0f;
  float FilmH = 1.0f;
  if(IMAGE_HEIGHT > IMAGE_WIDTH)
  {
    FilmH = 1.0f;
    FilmW = (float)IMAGE_WIDTH / (float)IMAGE_HEIGHT;
  }
  else
  {
    FilmH = (float)IMAGE_HEIGHT / (float)IMAGE_WIDTH;
    FilmW  = 1.0f;
  }
  float HalfFilmW = 0.5f * FilmW;
  float HalfFilmH = 0.5f * FilmH;

  // ...
  float HalfPixW = 0.5f / (float)IMAGE_WIDTH;
  float HalfPixH = 0.5f / (float)IMAGE_HEIGHT;
  
  // ...
  float FilmDist = 1.0f;
  v3 FilmCenter = CameraP - (FilmDist * CameraZ);

  // How many rays per pixel? 
  uint32_t RaysPerPixels = 128;
  float Contrib = 1.0f / (float)RaysPerPixels;

  // Image generation loop
  for(uint32_t y=0;y<IMAGE_HEIGHT;y++)
  {
    // Progress indicator 
    printf("\rCompleted ratio: %3.1f / 100.0", 
      (float)(100 * y) / (float)IMAGE_HEIGHT
    );
    fflush(stdout);

    // ...
    float FilmY = -1.0f + (2.0f * ((float)y / (float)IMAGE_HEIGHT));

    // ...
    for(uint32_t x=0;x<IMAGE_WIDTH;x++)
    {      
      float FilmX = -1.0f + (2.0f * ((float)x / (float)IMAGE_WIDTH));

      v3 Color = {};

      // Send multiple rays per pixel to increase the resolution
      for(uint32_t i=0; i<RaysPerPixels; i++)
      {
        // Small random dither in each ray per pixel
        float OffX = FilmX + (RandomUnilateral() * HalfPixW);
        float OffY = FilmY + (RandomUnilateral() * HalfPixH);

        // Calculate film posiiton based on the dither offsets
        v3 FilmP = {};
        FilmP  = FilmCenter;
        FilmP += (OffX * HalfFilmW * CameraX);
        FilmP += (OffY * HalfFilmH * CameraY);

        // ...
        v3 RayOrigin = CameraP;
        v3 RayDirection = Normalize(FilmP - CameraP);

        // Do the ray casting and accumulate the color ...
        Color += (Contrib * (RayCast(&World, RayOrigin, RayDirection)));
      }

      // ...
      ImgPutColor(img, x, y, Color);
    }
  }

  ImgWrite(img);
  printf("\nExit\n");
  return 0;
}

// ----------------------------------------------------------------------------
void ImgPutColor(float* img, int x, int y, v3 rgb)
{
  int baseInd = img_ind(x, y);
  img[baseInd + 0] = ExactLinearTosRGB(rgb.r);
  img[baseInd + 1] = ExactLinearTosRGB(rgb.g);
  img[baseInd + 2] = ExactLinearTosRGB(rgb.b);
}

// ----------------------------------------------------------------------------
int ImgWrite(float* img)
{
  static uint8_t img_raw[IMAGE_SIZE];

  for(uint32_t i=0;i<IMAGE_SIZE;i++)
  {
    img_raw[i] = (uint8_t)(img[i] * 255.0f);
  }
  printf("\n");

  printf("Image export ...\r\n");

  stbi_flip_vertically_on_write(1);
  int rv = stbi_write_png(
    FILENAME, IMAGE_WIDTH, IMAGE_HEIGHT, 3, img_raw, (3 * IMAGE_WIDTH)
  );

  printf("Image done.\r\n");
  return rv;
}

// ----------------------------------------------------------------------------
float ExactLinearTosRGB(float L)
{
  if(L > 1.0f)
  {
    L = 1.0f;
  }

  if(L < 0)
  {
    L = 0;
  }

  float S;
  if(L < 0.0031308f)
  {
    S = L * 12.92f; 
  }
  else
  { 
    S = (1.055f * pow(L, 1.0f/2.4f)) - 0.055f;
  }
  return S;
}

// ----------------------------------------------------------------------------
float RandomUnilateral()
{
  float Result = (float)rand() / (float)RAND_MAX;
  return Result;
}

// ----------------------------------------------------------------------------
float RandomBilateral()
{
  return -1.0f + (2.0f * RandomUnilateral());
}
