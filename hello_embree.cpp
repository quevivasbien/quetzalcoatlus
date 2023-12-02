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
Scene init_scene(RTCDevice &&device)
{
  Scene scene(std::move(device));

  // add_triangle(scene, Pt3(0, 0, 0), Pt3(1, 0, 0), Pt3(0, 1, 0));

  add_sphere(scene, Pt3(0.f, 0.f, 0.f), 0.5f);
  scene.commit();

  return scene;
}

/* -------------------------------------------------------------------------- */

int main()
{
  RTCDevice device = initialize_device();
  Scene scene = init_scene(std::move(device));

  Ray ray1(Pt3(0.33f, 0.33f, -1), Pt3(0, 0, 1));
  Ray ray2(Pt3(1.00f, 1.00f, -1), Pt3(0, 0, 1));

  auto hit1 = scene.ray_intersect(ray1);
  auto hit2 = scene.ray_intersect(ray2);

  if (hit1)
  {
    printf("Found intersection on geometry %d, at t=%f\n",
           hit1->geom_id,
           hit1->t);
  }
  else
  {
    printf("No intersection\n");
  }

  if (hit2)
  {
    printf("Found intersection on geometry %d, at t=%f\n",
           hit2->geom_id,
           hit2->t);
  }
  else
  {
    printf("No intersection\n");
  }

  return 0;
}
