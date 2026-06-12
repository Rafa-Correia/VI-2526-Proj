#include "Primitive/AccelerationStructures/BoundingVolumeHierarchy.hpp"
#include "Math/Vector.hpp"
#include "Primitive/AccelerationStructures/AccelerationStructure.hpp"
#include "Primitive/BoundingBox.hpp"
#include "Primitive/Geometry/Geometry.hpp"
#include "Ray/Intersection.hpp"
#include "Scene/Scene.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <limits>
#include <vector>

namespace VI
{
BVH BVH::Create(const Scene& scene)
{
  BVH bvh;
  bvh.Build(scene);

  return bvh;
}

void BVH::Build(const Scene& scene)
{
  auto begin = std::chrono::system_clock::now();

  size_t primCnt = scene.GetPrimitiveCount();

  m_Nodes.clear();
  m_PrimRefs.clear();

  m_Nodes.reserve(primCnt * 4); // reserves sufficient size (maybe?? maybe not but it should work)
  m_PrimRefs.reserve(primCnt);

  std::vector<PrimRef> refs;
  refs.reserve(primCnt);

  for (size_t i = 0; i < primCnt; i++)
  {
    refs.emplace_back(i, GetBoundingBox(scene.GetPrimitive(i).Geometry));
  }

  BuildNode(refs, 0, refs.size());

  auto end = std::chrono::system_clock::now();

  auto duration = std::chrono::duration<double>(end - begin);

  std::cout << "BVH Build time: " << duration.count() << " sec" << std::endl;

  std::cout << "Nodes: " << m_Nodes.size() << std::endl;
}

int BVH::BuildNode(std::vector<PrimRef> refs, size_t begin, size_t end)
{
  // return 0;

  size_t curr_node_index = m_Nodes.size();
  m_Nodes.emplace_back();

  size_t count = end - begin;

  if (count <= LeafPrimCount)
  {
    // this node is a leaf
    size_t prim = m_PrimRefs.size();
    m_Nodes[curr_node_index].firstPrim = prim;
    m_Nodes[curr_node_index].primCount = count;

    for (size_t i = begin; i < end; i++)
    {
      m_Nodes[curr_node_index].bounds.Update(refs[i].bounds);
      m_PrimRefs.push_back(refs[i]);
    }

    return curr_node_index;
  }

  BoundingBox center_bounds;

  for (size_t i = begin; i < end; i++)
  {
    m_Nodes[curr_node_index].bounds.Update(refs[i].bounds);
    center_bounds.Update(refs[i].bounds.Center());
  }

  Vector extents = center_bounds.Max - center_bounds.Min;

  size_t axis = 0;

  if (extents.y > extents[axis])
    axis = 1;

  if (extents.z > extents[axis])
    axis = 2;

  // axis now contains longest axis, and we'll cut along it (x - 0, y - 1, z - 2)
  // float mid_point = (center_bounds.Max[axis] + center_bounds.Min[axis]) / 2.f;

  // btw, sorting every primitive reference by axis!!
  std::sort(
      refs.begin() + begin,
      refs.begin() + end,
      [axis](const PrimRef& a,
             const PrimRef& b)
      {
        return a.bounds.Center()[axis] < b.bounds.Center()[axis];
      });

  // size_t cutoff =

  // cutting off at a point where half the objects a rre on one side
  // and half on the other is very, bery expensive for larger scenes.

  const size_t mid = begin + count / 2;

  m_Nodes[curr_node_index].leftChild = BuildNode(refs, begin, mid);
  m_Nodes[curr_node_index].rightChild = BuildNode(refs, mid, end);

  return curr_node_index;
}

// define this to bypass BVH trace and use a naive aproach
// #define USE_PASSTHROUGH

#ifdef USE_PASSTHROUGH

bool BVH::Trace(const Ray& ray, const Scene& scene, Intersection& intersection) const
{
  bool hit = false;

  size_t prim_cnt = scene.GetPrimitiveCount();

  float DMAX = std::numeric_limits<float>::max();

  for (size_t i = 0; i < prim_cnt; i++)
  {
    Primitive prim = scene.GetPrimitive(i);

    Intersection tmp{};

    if (Intersect(prim.Geometry, ray, tmp))
    {
      // std::cout << "Intersected" << std::endl;
      if (tmp.Distance < DMAX)
      {
        // std::cout << "Intersect is closer" << std::endl;

        hit = true;

        intersection.Distance = tmp.Distance;
        intersection.Normal = tmp.Normal;
        intersection.ObjectIndex = i;
        intersection.PrimitiveIndex = tmp.PrimitiveIndex;
        intersection.Position = tmp.Position;
        intersection.TexCoord = tmp.TexCoord;

        DMAX = intersection.Distance;

        // intersection = tmp;
      }
    }
  }

  return hit;
}

#else

bool BVH::Trace(const Ray& ray, const Scene& scene, Intersection& intersection) const
{
  bool hit = false;

  float TMAX = std::numeric_limits<float>::max();

  std::vector<size_t> stack(64);
  size_t current_stack_size = 0;

  stack[current_stack_size++] = 0; // stack begins at root node

  while (current_stack_size > 0)
  {
    size_t idx = stack[--current_stack_size]; // pop top stack member

    float tmin, tmax;
    if (!m_Nodes[idx].bounds.Intersect(ray, tmin, tmax)) // if not intersect, skip node and children
      continue;
    if (tmin > TMAX) // if intersection is farther than closest hit, skip node and children
      continue;

    if (m_Nodes[idx].isLeaf())
    {
      for (size_t i = m_Nodes[idx].firstPrim; i < m_Nodes[idx].firstPrim + m_Nodes[idx].primCount; i++)
      {
        Intersection tmp{};

        if (!Intersect(scene.GetPrimitive(m_PrimRefs[i].primIndex).Geometry, ray, tmp))
        {
          // std::cout << "DID NOT INTERSECT WITH PRIMITIVE\n";
          continue;
        }

        if (tmp.Distance < TMAX)
        {
          // std::cout << "CURRENT INTERSECTION IS BETTER" << std::endl;

          TMAX = tmp.Distance;

          intersection = tmp;
          intersection.ObjectIndex = m_PrimRefs[i].primIndex;

          hit = true;
        }
      }

      continue;
    }

    stack[current_stack_size++] = m_Nodes[idx].leftChild;
    stack[current_stack_size++] = m_Nodes[idx].rightChild;

    // check intersection
    // check children if intersects bounds (and not leaf)
  }

  return hit;
}

#endif

} // namespace VI