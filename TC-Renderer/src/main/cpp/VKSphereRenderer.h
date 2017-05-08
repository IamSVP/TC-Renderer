//
// Created by Srihari Pratapa on 5/7/17.
//

#ifndef TC_RENDERER_VKSPHERERENDERER_H
#define TC_RENDERER_VKSPHERERENDERER_H

#include <jni.h>
#include <errno.h>

#include <vector>
#include <string>
#include <iostream>


#include "gpu/vk_gpu.h"
#include "vk_guards.h"

enum TC_TYPES {
  ASTC4x4,
  ASTC8x8,
  ASTC12x12,
  DXT1,
  MPTC,
  JPG,
  CRN,
  MPEG,
};



#endif //TC_RENDERER_VKSPHERERENDERER_H
