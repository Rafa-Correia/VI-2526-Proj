#pragma once

#include "Primitive/AccelerationStructures/AccelerationStructure.hpp"
#include "Primitive/BoundingBox.hpp"
#include <cstddef>
#include <vector>

namespace VI
{

class BVH final : public AccelerationStructure
{
public:
  static BVH Create(const Scene& scene);

  bool Trace(const Ray& ray, const Scene& scene, Intersection& intersection) const override;

  void Build(const Scene& scene) override;

private:
  static const size_t LeafPrimCount = 1;

  struct PrimRef
  {
    int primIndex;
    BoundingBox bounds;
  };

  struct BVHNode
  {
    BoundingBox bounds;

    int leftChild = -1;
    int rightChild = -1;

    size_t firstPrim = 0;
    size_t primCount = 0; // number of primitives in leaf node (0 if not leaf)

    bool isLeaf() const
    {
      return primCount > 0;
    }
  };

  int BuildNode(std::vector<PrimRef> refs, size_t begin, size_t end);

  std::vector<BVHNode> m_Nodes;
  std::vector<PrimRef> m_PrimRefs;
};

} // namespace VI