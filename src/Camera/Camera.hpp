#pragma once

#include "Math/Vector.hpp"
#include "Ray/Ray.hpp"
#include "glm/ext/scalar_constants.hpp"

#include <glm/ext/vector_float2.hpp>
#include <iostream>
// #include <limits>

namespace VI
{
struct Resolution
{
  float Width, Height;
};

class ICamera
{
protected:
  Point m_Eye, m_At;
  Vector m_Up;

  int m_Width, m_Height;

public:
  virtual Ray GenerateRay(int x, int y, glm::vec2 jitter = {0.5f, 0.5f}) const = 0;

  virtual Resolution GetResolution() const noexcept = 0;
};

class Camera final : public ICamera
{
public:
  Camera(Point eye, Point at, Vector up, int width, int height, float fov_h);

  Ray GenerateRay(int x, int y, glm::vec2 jitter = {0.5f, 0.5f}) const override;

  inline Resolution GetResolution() const noexcept override
  {
    return {static_cast<float>(m_Width), static_cast<float>(m_Height)};
  }

  inline Point GetEye() const
  {
    return Point(this->m_Eye);
  }

  inline Point GetAt() const
  {
    return Point(this->m_At);
  }

  inline Vector GetUp() const
  {
    return Vector(this->m_Up);
  }

  inline float GetFOV() const
  {
    return fov_h;
  }

  void print_info() const
  {
    // debug only!!!

    std::cout << "Camera info:\n"
              << "\tEye: " << m_Eye << "\n"
              << "\tAt: " << m_At << "\n"
              << "\tFoV (rad): " << fov_h << "\n"
              << "\tFoV (deg): " << fov_h * 360 / (2 * 3.14159f)
              << std::endl;
  }

private:
  Point m_Pixel00Location;
  Vector m_PixelDeltaU, m_PixelDeltaV;

  float fov_h;
};

class Orthographic final : public ICamera
{
public:
  Orthographic(Point eye, Point at, Vector up, int width, int height, float sizex = 4, float sizey = 3);
  Orthographic(const Camera& other, float sizex, float sizey);

  Ray GenerateRay(int x, int y, glm::vec2 jitter = {0.5f, 0.5f}) const override;

  inline Resolution GetResolution() const noexcept override
  {
    return {static_cast<float>(m_Width), static_cast<float>(m_Height)};
  }

private:
  Point m_Pixel00Location;
  Vector m_PixelDeltaU, m_PixelDeltaV;

  Vector m_Dir;
};

class DOF final : public ICamera
{
public:
  DOF(Point eye, Point at, Vector up, int width, int height, float fov_h, float defocus_strength = 0.f, float focus_dist = 1.f);
  DOF(const Camera& other, float defocus_strength = 0.f, float focus_dist = 1.f);

  Ray GenerateRay(int x, int y, glm::vec2 jitter = {0.5f, 0.5f}) const override;

  inline Resolution GetResolution() const noexcept override
  {
    return {static_cast<float>(m_Width), static_cast<float>(m_Height)};
  }

private:
  Point m_Pixel00Location;
  Vector m_PixelDeltaU, m_PixelDeltaV;

  Vector m_DefocusDiskRight, m_DefocusDiskUp;
  float m_DefocusAngle;
};

class Fisheye final : public ICamera
{
public:
  Fisheye(Point eye, Point at, Vector up, int width, int height, float hfov = glm::pi<float>());
  Fisheye(const Camera& other, float hfov = glm::pi<float>());

  Ray GenerateRay(int x, int y, glm::vec2 jitter = {0.5f, 0.5f}) const override;

  inline Resolution GetResolution() const noexcept override
  {
    return {static_cast<float>(m_Width), static_cast<float>(m_Height)};
  }

private:
  Vector m_Dir, m_Right, m_CUp;

  float m_HFoV;
};

class Panorama final : public ICamera
{
public:
  Panorama(Point eye, Point at, Vector up, int width, int height, float hfovu = glm::pi<float>(), float hfovv = 45.f * 3.14159f / 180.f);
  Panorama(const Camera& other, float hfovu = glm::pi<float>(), float hfovv = 45.f * 3.14159f / 180.f);

  Ray GenerateRay(int x, int y, glm::vec2 jitter = {0.5f, 0.5f}) const override;

  inline Resolution GetResolution() const noexcept override
  {
    return {static_cast<float>(m_Width), static_cast<float>(m_Height)};
  }

private:
  Vector m_Dir, m_Right, m_CUp;

  float m_HFoVU, m_HFoVV;
};

class Lens final : public ICamera
{
public:
  Lens(Point eye, Point at, Vector up, int width, int height, float hfov, float lens_radius, float focal_distance);
  Lens(const Camera& other, float lens_radius, float focal_distance);

  Ray GenerateRay(int x, int y, glm::vec2 jitter = {0.5f, 0.5f}) const override;

  inline Resolution GetResolution() const noexcept override
  {
    return {static_cast<float>(m_Width), static_cast<float>(m_Height)};
  }

  inline Vector GetDir() const
  {
    return m_Dir;
  }

private:
  Vector m_Dir, m_Right, m_CUp;
  float m_HFoV, m_LensRadius, m_FocalDist;

  float m_HalfWidth, m_HalfHeight;
};

} // namespace VI
