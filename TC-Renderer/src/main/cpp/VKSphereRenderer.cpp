//
// Created by Srihari Pratapa on 5/7/17.
//

#include "VKSphereRenderer.h"


void VKSphereRenderer::Init(const char *storage_path, android_app *app) {
  _texture_path = std::string(storage_path) + "/Textures";
  _obj_path = std::string(storage_path) + "/Obj/sphere.obj";
  _metrics_path = std::string(storage_path) + "/output.txt";
#ifndef  NDEBUG
  LOGI("Still in debug mode!");
#endif
   //TODO:: ADD MPTC file paths and input streamsa
   //Set MPTC pointers to NULL. Set the initializations in LoadTexture()
  _mptc_file_path = std::string(storage_path) + "/MPTC/360MegaC2K_16_50.mpt";
  LOGE("MPTC PATH %s\n", _mptc_file_path.c_str());
  _mpeg_file_path = std::string(storage_path) + "/dongtan_day.mp4";

//  ptr_buffer_struct = NULL;
//  fp_mptc = NULL;
//  curr_dxt = NULL;
//  prev_dxt = NULL;
//  temp_dxt = NULL;
//  ptr_decode_info = NULL;

  _texture_number = 1;
  _texture_height = vImage_height;
  _texture_width = vImage_width;

  _vk_context

}