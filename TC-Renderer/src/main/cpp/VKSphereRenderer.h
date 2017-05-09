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
#include <tapCamera.h>
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

static const uint32_t vImage_width = 2560;
static const uint32_t vImage_height = 1280;
static TC_TYPES vTC_type = TC_TYPES::MPTC;
static const uint32_t vMax_tex_count = 480;


//IMPFIX: These things will be different in VULKAN
//!!! Change Them ASAP
#define COMPRESSED_RGBA_ASTC_4x4 0x93B0
#define COMPRESSED_RGBA_ASTC_8x8 0x93B7
#define COMPRESSED_RGBA_ASTC_12x12 0x93BD
#define COMPRESSED_RGB8_ETC1 0x8D64
#define COMPRESSED_RGB_DXT1 0x83F1

class VKSphereRenderer {
public:
  VKSphereRenderer();
  ~VKSphereRenderer();
  void Init(const char* storage_path, android_app *app);
  void Render();
  void Update(float delta_time);
  bool Bind(ndk_helper::TapCamera *camera);
  void Unload();
  void UpdateViewport();

  glm::mat4 _mat_projection;
  glm::mat4 _mat_view;
  glm::mat4 _mat_model;
  glm::mat4 _mat_scaling;
  glm::mat4 _mat_mvp;

  ndk_helper::TapCamera* _camera;

private:
  void IntializeScene();
  void LoadShaders();
  void LoadTexture(TC_TYPES curr_type);
  void IntializeTexture(TC_TYPES curr_type);
  void LoadShaders(const char* vertex_shader, const char* frag_shader);

  GPU::VKGPUContext _vk_context;
  std::string _texture_path;
  std::string _obj_path;
  std::string _metrics_path;
  std::string _mptc_path;

  uint64_t _texture_number;
  uint32_t _texture_width;
  uint32_t _texture_height;
  TC_TYPES _curr_TC_type;


  double GPU_Load;
  double Total_time;
  double CPU_load;
  uint32_t m_numframes;
  std::vector<uint64_t> m_CPULoad;
  std::vector<uint64_t> m_CPUDecode;
  std::vector<uint64_t> m_GPULoad;
  std::vector<uint64_t> m_GPUDecode;
  std::vector<uint64_t> m_TotalFps;

  uint32_t _num_indices;
  std::string _mptc_file_path;
  std::string _mpeg_file_path;
  uint32_t _num_blocks;

  bool load_texture;
  uint32_t load_count;


//  //MPTC Data
//  BufferStruct *ptr_buffer_struct;
//  std::ifstream _mptc_stream;
//  FILE *fp_mptc;
//  PhysicalDXTBlock *curr_dxt;
//  PhysicalDXTBlock *prev_dxt;
//  PhysicalDXTBlock *temp_dxt;
//  MPTCDecodeInfo *ptr_decode_info;
};



#endif //TC_RENDERER_VKSPHERERENDERER_H
