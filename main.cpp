// ----------------------------------------------------------------------------
// 
// Simple (?) raycaster written by watching 'Handmade Hero' Raycaster series
// and manually copying the code pieces from video by hand. Not an exact copy
// but the main algorithm is based on the video version.
// 
// More of an 'active learning' example.
// 
// TODO: Try to add a Cube object into raycaster engine ...
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
  static material Materials[6];
  Materials[0].EmitColor = V3(0.3,0.4,0.5);
  Materials[1].RefColor = V3(0.3,0.3,0.3);
  Materials[1].Scatter = 0.0f;
  Materials[2].RefColor = V3(0.7,0.5,0.3);
  Materials[2].Scatter = 0.9f;
  Materials[3].RefColor = V3(0.3,0.2,0.2);
  Materials[3].Scatter = 0.9f;
  Materials[4].RefColor = V3(0.2,0.8,0.2);
  Materials[4].Scatter = 0.5f;
  Materials[5].RefColor = V3(0.4,0.3,0.4);
  Materials[5].Scatter = 0.9;

  // We have only one plane
  plane Plane = {};
  Plane.N = Normalize(V3(0,0,1));
  Plane.d = 0;
  Plane.MatIndex = 1;

  // Multiple spheres ...
  sphere Spheres[4];
  Spheres[0].P = V3(3, 01, 0);
  Spheres[0].r = 1.0f;
  Spheres[0].MatIndex = 2;
  Spheres[1].P = V3(-10, 10, 5);
  Spheres[1].r = 3.0f;
  Spheres[1].MatIndex = 3;
  Spheres[2].P = V3(5, 0, 3);
  Spheres[2].r = 1.0f;
  Spheres[2].MatIndex = 4;
  Spheres[3].P = V3(0, 30, 4);
  Spheres[3].r = 10;
  Spheres[3].MatIndex = 5;

  // Pack everything into the world struct
  world World = {};
  World.MaterialCount = 6;
  World.Materials = Materials;
  World.PlaneCount = 1; 
  World.Planes = &Plane;
  World.SphereCount = 4;
  World.Spheres = Spheres;

  // Calculate camera position and direction vectors
  v3 CameraP = V3(0,-10,1);
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
  uint32_t RaysPerPixels = 32; 
  float Contrib = 1.0f / (float)RaysPerPixels;

  // Image generation loop
  for(uint32_t y=0;y<IMAGE_HEIGHT;y++)
  {
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
  printf("Exit\n");
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
