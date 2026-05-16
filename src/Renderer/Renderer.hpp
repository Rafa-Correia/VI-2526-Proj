#pragma once

#include "Camera/Camera.hpp"
#include "Image/Image.hpp"
#include "Math/Random.hpp"
#include "Shaders/Shader.hpp"
#include "Utils/ProgressBar.hpp"

#include <atomic>
#include <future>
#include <iostream>
#include <vector>

namespace VI
{
class Scene;
class Camera;

class Renderer final
{
public:
  template <Shader S>
  Image Render(const Scene& scene, const Camera& camera, const S& shader, int samples_per_pixel = 1, bool do_jittering = false)
  {
    std::cout << "Call to Renderer::Render(...)" << std::endl;

    auto [width, height] = camera.GetResolution();

    Image image{static_cast<int>(width), static_cast<int>(height)};
    // ProgressBar progress{static_cast<int>(width * height)};

    // std::vector<std::future<void>> futures;

    // float spp_factor = 1.0f / samples_per_pixel;

    // int thread_cnt = std::thread::hardware_concurrency();
    int thread_cnt = 8;

    std::cout << "Up to " << thread_cnt << " concurrent threads supported." << std::endl;

    std::vector<std::thread> threads(thread_cnt);

    std::atomic<int> next_row = 0;

    for (int i = 0; i < thread_cnt; i++)
    {
      threads[i] = std::thread(
          [&, i]()
          {
            thread_callable(
                next_row,
                image,
                /* progress, */
                scene,
                camera,
                shader,
                samples_per_pixel,
                do_jittering);
          });
    }

    for (int i = 0; i < thread_cnt; i++)
    {
      threads[i].join();
    }

    /* for (int y = 0; y < static_cast<int>(height); ++y)
    {
      for (int x = 0; x < static_cast<int>(width); ++x)
      {
        RGB color = RGB{0.0f};
        for (int s = 0; s < samples_per_pixel; s++)
        {
          Vector jitter = {0.5, 0.5, 0.0};
          if (do_jittering)
          {
            jitter = Random::RandomVec3(0, 1);
          }
          const Ray ray = camera.GenerateRay(x, y, jitter);
          color += shader.Execute(ray, scene);
        }
        image.Set(x, y, color * spp_factor);
        progress.Increment();
      }
    } */

    // progress.Finish();

    return image;
  }

private:
  template <Shader S>
  void thread_callable(std::atomic<int>& next_row, Image& img /* , ProgressBar& prog */, const Scene& sc, const Camera& cam, const S& sh, int spp, bool do_jittering)
  {
    // auto begin = std::chrono::system_clock::now();

    int height = img.GetHeight();
    int width = img.GetWidth();

    // std::cout << "thread_callable(...): " << thread_idx << std::endl;

    float spp_factor = 1.0f / spp;

    while (1)
    {
      const int y = next_row.fetch_add(1, std::memory_order_relaxed);

      // No more work left
      if (y >= height)
      {
        break;
      }

      for (int x = 0; x < static_cast<int>(width); ++x)
      {
        RGB color = RGB{0.0f};
        for (int s = 0; s < spp; s++)
        {
          Vector jitter = {0.5, 0.5, 0.0};
          if (do_jittering)
          {
            jitter = Random::RandomVec3(0, 1);
          }
          const Ray ray = cam.GenerateRay(x, y, jitter);
          color += sh.Execute(ray, sc);
        }
        img.Set(x, y, color * spp_factor);
        // prog.Increment();
      }
    }

    // auto end = std::chrono::system_clock::now();

    // auto duration = std::chrono::duration<double>(end - begin);

    // std::cout << "Thread " << thread_idx << " took: " << duration.count() << " sec" << '\n';
  }
};
} // namespace VI
