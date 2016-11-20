//
// Created by Srihari Pratapa on 11/12/16.
//
// Sphere Renderer for 360 video
// Renders a sphere with a texture from several formats
#ifndef __TC_RENDERER_SPHERERENDERER_H__
#define __TC_RENDERER_SPHERERENDERER_H__

#include <jni.h>
#include <errno.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

#include <EGL/egl.h>
#include <GLES3/gl31.h>



#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include "decoder.h"

#define DEBUG

#include "gl_guards.h"

enum TC_TYPES {
  ASTC4x4,
  ASTC8x8,
  ASTC12x12,
  DXT1,
  MPTC,
  JPG,
  CRN,

};
static const uint32_t vImage_width = 2560;
static const uint32_t vImage_height = 1280;
static const  TC_TYPES vTC_type = TC_TYPES::JPG;
static const uint32_t vMax_tex_count = 63;

#define COMPRESSED_RGBA_ASTC_4x4 0x93B0
#define COMPRESSED_RGBA_ASTC_8x8 0x93B7
#define COMPRESSED_RGBA_ASTC_12x12 0x93BD
#define COMPRESSED_RGB8_ETC1 0x8D64
#define COMPRESSED_RGB_DXT1 0x83F1

class SphereRenderer {


 public:
  SphereRenderer();
  ~SphereRenderer();
  void Init(const char* storage_path);
  void Render();
  void Update(float delta_time);
  bool Bind(ndk_helper::TapCamera *camera);
  void Unload();
  void UpdateViewport();


  ndk_helper::Mat4 _mat_projection;
  ndk_helper::Mat4 _mat_view;
  ndk_helper::Mat4 _mat_model;
  ndk_helper::Mat4 _mat_scaling;
  ndk_helper::Mat4 _mat_mvp;

  ndk_helper::TapCamera* _camera;

 private:


  void InitializeScene();
  void LoadShaders();
  void LoadTexture(TC_TYPES curr_type);
  void InitializeTexture(TC_TYPES curr_type);
  void LoadShaders(const char* vertex_shader, const char* frag_shader);

  // Initialize Texture Functions....
  void InitializeFullTexture();
  void InitializeDXT1Texuture();
  void InitializeASTC4x4Texture();
  void InitializeASTC8x8Texture();
  void InitializeMPTCTexture();
  void InitializeCRNTexture();

  // Texture Loading functions....
  void LoadTextureDataJPG();
  void LoadTextureDataDXT1();
  void LoadTextureDataCRN();
  void LoadTextureDataASTC4x4();
  void LoadTextureDataASTC8x8();
  void LoadTextureDataPBO();
  void LoadTextureDataMPTC();




  GLuint _program_id;
  GLint _pos_loc;
  GLint _uv_loc;
  GLint _tex_loc;
  GLint _matrix_id;

  GLuint _vertex_buffer;
  GLuint _index_buffer;
  GLuint _uv_buffer;
  GLuint _vertex_array_id;

  GLuint _texture_id;

  std::string _texture_path;
  std::string _obj_path;
  std::string _metrics_path;
  std::string _mptc_path;
  std::ofstream _metrics_out_stream;

  uint64_t _texture_number;

  uint32_t _texture_width;
  uint32_t _texture_height;
  TC_TYPES  _curr_TC_type;


  uint32_t m_numframes;
  std::vector<uint64_t> m_CPULoad;
  std::vector<uint64_t> m_CPUDecode;
  std::vector<uint64_t> m_GPULoad;
  std::vector<uint64_t> m_GPUDecode;
  std::vector<uint64_t> m_TotalFps;

  uint32_t _num_indices;
  std::string _mptc_file_path;
  uint32_t _num_blocks;

  //MPTC Data
  BufferStruct *ptr_buffer_struct;
  std::ifstream _mptc_stream;
  FILE *fp_mptc;
  PhysicalDXTBlock *curr_dxt;
  PhysicalDXTBlock *prev_dxt;
  PhysicalDXTBlock *temp_dxt;
  MPTCDecodeInfo *ptr_decode_info;
};


#endif //TC_RENDERER_SPHERERENDERER_H
