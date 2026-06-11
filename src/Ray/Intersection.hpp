#pragma once

#include "Math/Vector.hpp"
#include <limits>

namespace VI
{

struct Intersection
{
  Point Position{0.f};
  Vector Normal{0.f};
  Vec2 TexCoord{0.f};
  float Distance{std::numeric_limits<float>::max()};
  int ObjectIndex{-1};
  int PrimitiveIndex{-1};
};

} // namespace VI
