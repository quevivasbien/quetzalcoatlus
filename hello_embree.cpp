// Copyright 2009-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <embree4/rtcore.h>
#include <stdio.h>
#include <math.h>
#include <limits>

#include "scene.hpp"

/*
 * Create a scene, which is a collection of geometry objects. Scenes are
 * what the intersect / occluded functions work on. You can think of a
 * scene as an acceleration structure, e.g. a bounding-volume hierarchy.
 *
 * Scenes, like devices, are reference-counted.
 */
Scene init_scene(RTCDevice &&device, Material* material)
{
  Scene scene(std::move(device));

  // add_triangle(scene, Pt3(0, 0, 0), Pt3(1, 0, 0), Pt3(0, 1, 0));

  scene.add_sphere(Pt3(0.f, 0.f, 0.f), 0.5f, material);
  scene.commit();

  return scene;
}

/* -------------------------------------------------------------------------- */

int main()
{
  RTCDevice device = initialize_device();
  LambertMaterial material(SolidColor(0.5f, 0.5f, 0.5f));
  Scene scene = init_scene(std::move(device), &material);

  Ray ray1(Pt3(0.33f, 0.33f, -1), Pt3(0, 0, 1));
  Ray ray2(Pt3(1.00f, 1.00f, -1), Pt3(0, 0, 1));

  Sampler sampler(0, 2);

  auto hit1 = scene.ray_intersect(ray1, sampler);
  auto hit2 = scene.ray_intersect(ray2, sampler);

  if (hit1) {
    std::cout << "hit1: color = " << hit1->color.x << " " << hit1->color.y << " " << hit1->color.z << std::endl;
  }
  else {
    std::cout << "hit1: no intersection" << std::endl;
  }

  if (hit2) {
    std::cout << "hit2: color = " << hit2->color.x << " " << hit2->color.y << " " << hit2->color.z << std::endl;
  }
  else {
    std::cout << "hit2: no intersection" << std::endl;
  }

  return 0;
}
