#include "Camera/Camera.hpp"

#include "Math/Random.hpp"
#include "Math/Vector.hpp"
#include "Ray/Ray.hpp"

#include <glm/ext/vector_float2.hpp>
#include <glm/geometric.hpp>

#include <cmath>

namespace VI
{
Camera::Camera(Point eye, Point at, Vector up, int width, int height, float fov_h)
{
  m_Eye = eye;
  m_At = at;
  m_Up = up;

  m_Width = width;
  m_Height = height;

  this->fov_h = fov_h;

  float focus_dist = 1.0f;

  Vector forward = glm::normalize(m_At - m_Eye);
  Vector right = glm::normalize(glm::cross(forward, up));

  m_Up = glm::normalize(glm::cross(right, forward));

  float tan_halfh = std::tan(fov_h / 2.f);
  float viewport_height = 2.0 * tan_halfh * focus_dist;
  float viewport_width = viewport_height * m_Width / m_Height;

  Vector viewport_u = viewport_width * right;
  Vector viewport_v = -viewport_height * m_Up;

  m_PixelDeltaU = viewport_u / static_cast<float>(m_Width);
  m_PixelDeltaV = viewport_v / static_cast<float>(m_Height);

  Point viewport_upper_left = m_Eye + focus_dist * forward;
  viewport_upper_left = viewport_upper_left - (viewport_u / 2.f) - (viewport_v / 2.f);
  m_Pixel00Location = viewport_upper_left + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);
}

Ray Camera::GenerateRay(int x, int y, glm::vec2 jitter) const
{
  Point pc{x + jitter.x, y + jitter.y, 0};

  Point pixel_sample = m_Pixel00Location + (pc.x * m_PixelDeltaU) + (pc.y * m_PixelDeltaV);

  Point origin = m_Eye;

  Vector direction = glm::normalize(pixel_sample - origin);

  return {.Origin = origin, .Direction = direction};
}

/**
 * ORTHOGRAPIC!!!
 */

Orthographic::Orthographic(Point eye, Point at, Vector up, int width, int height, float sizex, float sizey)
{
  m_Eye = eye;
  m_At = at;
  m_Up = up;
  m_Width = width;
  m_Height = height;

  m_Dir = glm::normalize(m_At - m_Eye);

  Vector right = glm::normalize(glm::cross(m_Dir, m_Up));
  Vector c_up = glm::normalize(glm::cross(right, m_Dir));

  m_Pixel00Location = m_Eye - (right * (sizex / 2.0f)) + (c_up * (sizey / 2.0f));

  m_PixelDeltaU = right * (sizex / static_cast<float>(m_Width));
  m_PixelDeltaV = c_up * (-1.0f * sizey / static_cast<float>(m_Height));
}

Orthographic::Orthographic(const Camera& other, float sizex, float sizey)
{
  m_Eye = other.GetEye();
  m_At = other.GetAt();
  m_Up = other.GetUp();

  Resolution res = other.GetResolution();

  m_Width = static_cast<int>(res.Width);
  m_Height = static_cast<int>(res.Height);

  m_Dir = glm::normalize(m_At - m_Eye);

  Vector right = glm::normalize(glm::cross(m_Dir, m_Up));
  Vector c_up = glm::normalize(glm::cross(right, m_Dir));

  m_Pixel00Location = m_Eye - (right * (sizex / 2.0f)) + (c_up * (sizey / 2.0f));

  m_PixelDeltaU = right * (sizex / static_cast<float>(m_Width));
  m_PixelDeltaV = c_up * (-1.0f * sizey / static_cast<float>(m_Height));
}

Ray Orthographic::GenerateRay(int x, int y, glm::vec2 jitter) const
{
  Point origin = m_Pixel00Location + (m_PixelDeltaU * (static_cast<float>(x) + jitter.x)) + (m_PixelDeltaV * (static_cast<float>(y) + jitter.y));

  return {origin, Vector(m_Dir)};
}

/**
 * DOF
 */

DOF::DOF(Point eye, Point at, Vector up, int width, int height, float fov_h, float defocus_strength, float focus_dist)
{
  m_Eye = eye;
  m_At = at;
  m_Up = up;

  m_Width = width;
  m_Height = height;

  Vector forward = glm::normalize(m_At - m_Eye);
  Vector right = glm::normalize(glm::cross(forward, up));

  m_Up = glm::normalize(glm::cross(right, forward));

  float tan_halfh = std::tan(fov_h / 2.f);
  float viewport_height = 2.0 * tan_halfh * focus_dist;
  float viewport_width = viewport_height * m_Width / m_Height;

  Vector viewport_u = viewport_width * right;
  Vector viewport_v = -viewport_height * m_Up;

  m_PixelDeltaU = viewport_u / static_cast<float>(m_Width);
  m_PixelDeltaV = viewport_v / static_cast<float>(m_Height);

  Point viewport_upper_left = m_Eye + focus_dist * forward;
  viewport_upper_left = viewport_upper_left - (viewport_u / 2.f) - (viewport_v / 2.f);
  m_Pixel00Location = viewport_upper_left + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);

  float defocus_radius = focus_dist * std::tan(defocus_strength / 2.f);

  m_DefocusDiskRight = right * defocus_radius;
  m_DefocusDiskUp = m_Up * defocus_radius;
}

DOF::DOF(const Camera& other, float defocus_strength, float focus_dist)
{
  m_Eye = other.GetEye();
  m_At = other.GetAt();
  m_Up = other.GetUp();

  Resolution res = other.GetResolution();

  m_Width = static_cast<int>(res.Width);
  m_Height = static_cast<int>(res.Height);

  float fov_h = other.GetFOV();

  Vector forward = glm::normalize(m_At - m_Eye);
  Vector right = glm::normalize(glm::cross(forward, m_Up));

  m_Up = glm::normalize(glm::cross(right, forward));

  float tan_halfh = std::tan(fov_h / 2.f);
  float viewport_height = 2.0 * tan_halfh * focus_dist;
  float viewport_width = viewport_height * m_Width / m_Height;

  Vector viewport_u = viewport_width * right;
  Vector viewport_v = -viewport_height * m_Up;

  m_PixelDeltaU = viewport_u / static_cast<float>(m_Width);
  m_PixelDeltaV = viewport_v / static_cast<float>(m_Height);

  Point viewport_upper_left = m_Eye + focus_dist * forward;
  viewport_upper_left = viewport_upper_left - (viewport_u / 2.f) - (viewport_v / 2.f);
  m_Pixel00Location = viewport_upper_left + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);

  float defocus_radius = focus_dist * std::tan(defocus_strength / 2.f);

  m_DefocusDiskRight = right * defocus_radius;
  m_DefocusDiskUp = m_Up * defocus_radius;
}

Ray DOF::GenerateRay(int x, int y, glm::vec2 jitter) const
{
  Point pc{x + jitter.x, y + jitter.y, 0};

  Point pixel_sample = m_Pixel00Location + (pc.x * m_PixelDeltaU) + (pc.y * m_PixelDeltaV);

  Point origin = m_Eye;

  if (m_DefocusAngle > 0.f)
  {
    Point p = Random::RandomInUnitDisk();
    origin = m_Eye + p.x * m_DefocusDiskRight + p.y * m_DefocusDiskUp;
  }

  Vector direction = glm::normalize(pixel_sample - origin);

  return {.Origin = origin, .Direction = direction};
}

// FISHEYE

Fisheye::Fisheye(Point eye, Point at, Vector up, int width, int height, float hfov)
{
  m_Eye = eye;
  m_At = at;
  m_Up = up;

  m_Dir = glm::normalize(m_At - m_Eye);
  m_Right = glm::normalize(glm::cross(m_Dir, m_Up));
  m_CUp = glm::normalize(glm::cross(m_Right, m_Dir));

  m_Width = width;
  m_Height = height;

  m_HFoV = hfov;
}

Fisheye::Fisheye(const Camera& other, float hfov)
{
  m_Eye = other.GetEye();
  m_At = other.GetAt();
  m_Up = other.GetUp();

  m_Dir = glm::normalize(m_At - m_Eye);
  m_Right = glm::normalize(glm::cross(m_Dir, m_Up));
  m_CUp = glm::normalize(glm::cross(m_Right, m_Dir));

  Resolution res = other.GetResolution();

  m_Width = static_cast<int>(res.Width);
  m_Height = static_cast<int>(res.Height);

  m_HFoV = hfov;
}

Ray Fisheye::GenerateRay(int x, int y, glm::vec2 jitter) const
{
  float u = (2.0f * ((x + jitter.x) / static_cast<float>(m_Width))) - 1.0f;

  float v = -((2.0f * ((y + jitter.y) / static_cast<float>(m_Height))) - 1.0f);

  float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

  u *= aspect;

  float r = std::sqrt(u * u + v * v);

  if (r > 1.0f)
    return {m_Eye, m_Dir};

  float phi = std::atan2(v, u);

  float theta = r * (m_HFoV * 0.5f);

  float sinTheta = std::sin(theta);
  float cosTheta = std::cos(theta);

  Vector localDir(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

  Vector worldDir = localDir.x * m_Right + localDir.y * m_CUp + localDir.z * m_Dir;

  worldDir = glm::normalize(worldDir);

  return {m_Eye, worldDir};
}

// Panorama

Panorama::Panorama(Point eye, Point at, Vector up, int width, int height, float hfovu, float hfovv)
{
  m_Eye = eye;
  m_At = at;
  m_Up = up;

  m_Width = width;
  m_Height = height;

  m_Dir = glm::normalize(m_At - m_Eye);
  m_Right = glm::normalize(glm::cross(m_Dir, m_Up));
  m_CUp = glm::normalize(glm::cross(m_Right, m_Dir));

  m_HFoVU = hfovu;
  m_HFoVV = hfovv;
}

Panorama::Panorama(const Camera& other, float hfovu, float hfovv)
{
  m_Eye = other.GetEye();
  m_At = other.GetAt();
  m_Up = other.GetUp();

  m_Dir = glm::normalize(m_At - m_Eye);
  m_Right = glm::normalize(glm::cross(m_Dir, m_Up));
  m_CUp = glm::normalize(glm::cross(m_Right, m_Dir));

  Resolution res = other.GetResolution();

  m_Width = static_cast<int>(res.Width);
  m_Height = static_cast<int>(res.Height);

  m_HFoVU = hfovu;
  m_HFoVV = hfovv;
}

Ray Panorama::GenerateRay(int x, int y, glm::vec2 jitter) const
{
  float u = (x + jitter.x) / m_Width;
  float v = (y + jitter.y) / m_Height;

  u = 2.0f * u - 1.0f;
  v = 1.0f - 2.0f * v;

  float theta = u * m_HFoVU; // yaw (left-right)
  float phi = v * m_HFoVV;   // pitch (up-down)

  float cosPhi = std::cos(phi);

  Vector dir_cam;
  dir_cam.x = std::sin(theta) * cosPhi;
  dir_cam.y = std::sin(phi);
  dir_cam.z = std::cos(theta) * cosPhi;

  Vector world_dir = dir_cam.x * m_Right + dir_cam.y * m_CUp + dir_cam.z * m_Dir;
  world_dir = glm::normalize(world_dir);

  return {m_Eye, world_dir};
}

} // namespace VI
