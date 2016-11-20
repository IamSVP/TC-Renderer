//
// Created by Srihari Pratapa on 11/12/16.
//

#include "SphereRenderer.h"
#include "ObjLoader/objloader.hpp"
#include "ObjLoader/vboindexer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <chrono>
#include "Shaders.shdrs"
//TODO:: USE enums to set the texture paths
//TODO:: USE enums to set the texture data
//TODO:: Write all the functions from the initial data

SphereRenderer::SphereRenderer() { }

void SphereRenderer::UpdateViewport() {

  // Init Projection matrices
  int32_t viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  const float CAM_NEAR = 5.f;
  const float CAM_FAR = 10000.f;
  if (viewport[2] < viewport[3]) {
    float aspect =
        static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);
    _mat_projection =
        ndk_helper::Mat4::Perspective(aspect, 1.0f, CAM_NEAR, CAM_FAR);
  } else {
    float aspect =
        static_cast<float>(viewport[3]) / static_cast<float>(viewport[2]);
    _mat_projection =
        ndk_helper::Mat4::Perspective(1.0f, aspect, CAM_NEAR, CAM_FAR);
  }
}

void SphereRenderer::InitializeScene() {

  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  bool res = loadOBJ(_obj_path.c_str(), vertices, uvs, normals);
  LOGE("object path....%s\n", _obj_path.c_str());
  std::vector<unsigned short> indices;
  std::vector<glm::vec3> indexed_vertices;
  std::vector<glm::vec2> indexed_uvs;
  std::vector<glm::vec3> indexed_normals;
  indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

  CHECK_GL(glGenBuffers, 1, &_vertex_buffer);
  CHECK_GL(glBindBuffer, GL_ARRAY_BUFFER, _vertex_buffer);
  CHECK_GL(glBufferData,GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

  CHECK_GL(glGenBuffers, 1, &_uv_buffer);
  CHECK_GL(glBindBuffer, GL_ARRAY_BUFFER, _uv_buffer);
  CHECK_GL(glBufferData, GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);


  // Generate a buffer for the indices as well
  CHECK_GL(glGenBuffers, 1, &_index_buffer);
  CHECK_GL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _index_buffer);
  CHECK_GL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
  _num_indices = indices.size();
  LOGE("Indices size %d\n", _num_indices);

}


void SphereRenderer::InitializeFullTexture() {

  CHECK_GL(glGenTextures, 1, &_texture_id);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  // CHECK_GL(glTexImage2D, GL_TEXTURE_2D,0,GL_RGBA8UI, vImageWidth, vImageHeight,0,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,NULL);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, GL_RGBA8,vImage_width, vImage_height );
  CHECK_GL(glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindImageTexture, 2, _texture_id, 0, GL_FALSE,0, GL_WRITE_ONLY, GL_RGBA8UI); // possible error
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
}
void SphereRenderer::InitializeDXT1Texuture() {
  CHECK_GL(glGenTextures, 1, &_texture_id);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGB_DXT1, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);

}
void SphereRenderer::InitializeCRNTexture() {

  CHECK_GL(glGenTextures, 1, &_texture_id);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGB_DXT1, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
}
void SphereRenderer::InitializeASTC8x8Texture() {

  CHECK_GL(glGenTextures, 1, &_texture_id);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGBA_ASTC_8x8, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);

}
void SphereRenderer::InitializeASTC4x4Texture() {

  CHECK_GL(glGenTextures, 1, &_texture_id);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGBA_ASTC_4x4, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
}
void SphereRenderer::InitializeMPTCTexture() {

  CHECK_GL(glGenTextures, 1, &_texture_id);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGB_DXT1, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);

  _num_blocks = _texture_width/4 * _texture_height/4;
  prev_dxt = new PhysicalDXTBlock[_num_blocks];
  curr_dxt = new PhysicalDXTBlock[_num_blocks];

  ptr_decode_info = new MPTCDecodeInfo();
  ptr_buffer_struct = new BufferStruct();
  ptr_decode_info->is_start = true;
  _mptc_stream.open(_mptc_file_path.c_str(), std::ios::binary);
  //InitBufferedDecode(6, ptr_buffer_struct, _mptc_stream, _num_blocks);

  //fp_mptc = fopen(_mptc_file_path.c_str(), "rb");
  LOGE("Called load Texture Data,,,,,");
}


void SphereRenderer::LoadTextureDataASTC4x4() {

  unsigned char *blocks;
  size_t  TexSz = _texture_height * _texture_width;
  blocks = (unsigned char *)malloc( sizeof(unsigned char) * TexSz);

  char img_path[256];

  sprintf(img_path, "%s/360MegaCoaster720/ASTC4x4/360MegaCoaster720%06d.astc",_texture_path.c_str(), _texture_number);
  LOGE("Path %s", img_path);
  unsigned char t[16];
  FILE *fp = fopen(img_path, "rb");
  fread(t,1,16,fp);
  std::chrono::high_resolution_clock::time_point CPULoad_Start = std::chrono::high_resolution_clock::now();
  size_t numBytes = fread(blocks, 1, TexSz, fp);
  if( numBytes <= 0 )
    LOGE("TexSz bytes :%lld\n",TexSz);
  else
    LOGE("Num bytes :%lld\n",numBytes);
  std::chrono::high_resolution_clock::time_point CPULoad_End = std::chrono::high_resolution_clock::now();

  std::chrono::nanoseconds CPULoad_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(CPULoad_End - CPULoad_Start);
  m_CPULoad.push_back(CPULoad_Time.count());



//    CHECK_GL(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, m_PboId[0]);
//
//    blocks = (GLubyte*)CHECK_GL(glMapBufferRange, GL_PIXEL_UNPACK_BUFFER, 0, (vImageHeight * vImageWidth)/2, GL_WRITE_ONLY);
//    if(blocks){
//
//        input.read(blocks, (vImageHeight * vImageWidth)/2);
//        CHECK_GL(glUnmapBuffer, GL_PIXEL_UNPACK_BUFFER);
//    }

  // GPU Loading........
  std::chrono::high_resolution_clock::time_point GPULoad_Start = std::chrono::high_resolution_clock::now();
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  CHECK_GL(glCompressedTexSubImage2D, GL_TEXTURE_2D,    // Type of texture
           0,                // level (0 being the top level i.e. full size)
           0, 0,             // Offset
           _texture_width,       // Width of the texture
           _texture_height,      // Height of the texture,
           COMPRESSED_RGBA_ASTC_4x4,          // Data format
           TexSz, // Type of texture data
           blocks);

  std::chrono::high_resolution_clock ::time_point GPULoad_End = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds GPULoad_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(GPULoad_End - GPULoad_Start);
  m_GPULoad.push_back(GPULoad_Time.count());

  // CHECK_GL(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, 0);
  // ALOGE("fooooddd here\n");
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
  free(blocks);
  fclose(fp);
}
void SphereRenderer::LoadTextureDataASTC8x8() { }
void SphereRenderer::LoadTextureDataCRN() { }
void SphereRenderer::LoadTextureDataDXT1() { }
void SphereRenderer::LoadTextureDataMPTC() {

  //*****************ADD CHRONO****************//
  //*******************************************//
  //******************************************//
  //*****************************************//
  //LOGE("Called load Texture Data");
  std::chrono::high_resolution_clock::time_point CPUDecode_Start = std::chrono::high_resolution_clock::now();
  GetFrameMultiThread(_mptc_stream,
                      prev_dxt,
                      curr_dxt,
                      ptr_decode_info);
  //GetFrame(_mptc_stream, prev_dxt, curr_dxt, ptr_decode_info);

  std::chrono::high_resolution_clock::time_point CPUDecode_End = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds CPUDecode_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(CPUDecode_End - CPUDecode_Start);
  m_CPUDecode.push_back(CPUDecode_Time.count());


  std::chrono::high_resolution_clock::time_point GPULoad_Start = std::chrono::high_resolution_clock::now();
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);
  CHECK_GL(glCompressedTexSubImage2D, GL_TEXTURE_2D,    // Type of texture
           0,                // level (0 being the top level i.e. full size)
           0, 0,             // Offset
           _texture_width,       // Width of the texture
           _texture_height,      // Height of the texture,
           COMPRESSED_RGB_DXT1,          // Data format
           _texture_width*_texture_height / 2, // Type of texture data
           //ptr_buffer_struct->buffered_dxts[ptr_buffer_struct->curr_dxt_idx]);
           curr_dxt);

  LOGE("curr_dxt id value:%d\n", ptr_buffer_struct->curr_dxt_idx);
  //GetBufferedFrame(ptr_buffer_struct, curr_dxt, _mptc_stream);
  std::chrono::high_resolution_clock ::time_point GPULoad_End = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds GPULoad_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(GPULoad_End - GPULoad_Start);
  m_GPULoad.push_back(GPULoad_Time.count());

  // CHECK_GL(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, 0);
  // ALOGE("fooooddd here\n");
  //temp_dxt = prev_dxt;
  //prev_dxt = curr_dxt;
  //curr_dxt = temp_dxt;
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);


}
void SphereRenderer::LoadTextureDataJPG() {


  char img_path[256];
  //sprintf(img_path, "%s/360MegaCoaster720/JPG/360MegaCoaster720%06d.jpg",_texture_path.c_str(),_texture_number);
  sprintf(img_path, "%s/360Shark2K/360Shark2K%04d.jpg", _texture_path.c_str(), _texture_number);
  LOGE("Path%s", img_path);
  int w,h,n;

  unsigned char *ImageDataPtr = (unsigned char *)malloc((_texture_height * _texture_width * 4));
  FILE *fp = fopen(img_path, "rb");


  //CPU LOADING.....
  std::chrono::high_resolution_clock::time_point CPULoad_Start = std::chrono::high_resolution_clock::now();
  fread(ImageDataPtr, 1, _texture_height * _texture_width * 4, fp);
  std::chrono::high_resolution_clock::time_point CPULoad_End = std::chrono::high_resolution_clock::now();

  std::chrono::nanoseconds CPULoad_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(CPULoad_End - CPULoad_Start);
  m_CPULoad.push_back(CPULoad_Time.count());


  // CPU Decoding.....
  std::chrono::high_resolution_clock::time_point CPUDecode_Start = std::chrono::high_resolution_clock::now();
  unsigned char *TextureData = stbi_load_from_memory(ImageDataPtr, (_texture_height * _texture_width * 4), &w, &h, &n, 4);
  std::chrono::high_resolution_clock::time_point CPUDecode_End = std::chrono::high_resolution_clock::now();

  std::chrono::nanoseconds CPUDecode_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(CPUDecode_End - CPUDecode_Start);
  m_CPUDecode.push_back(CPUDecode_Time.count());
  free(ImageDataPtr);

  LOGE("checking the loaded texture: %d %d %d \n", w, h, _texture_number);


  // GPU Loading........
  std::chrono::high_resolution_clock::time_point GPULoad_Start = std::chrono::high_resolution_clock::now();
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);

  CHECK_GL(glTexSubImage2D, GL_TEXTURE_2D,
           0,
           0,0,
           _texture_width,
           _texture_height,
           GL_RGBA,
           GL_UNSIGNED_BYTE,
           TextureData);
  std::chrono::high_resolution_clock ::time_point GPULoad_End = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds GPULoad_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(GPULoad_End - GPULoad_Start);
  m_GPULoad.push_back(GPULoad_Time.count());

  CHECK_GL(glBindTexture,GL_TEXTURE_2D, 0);

  stbi_image_free(TextureData);
  fclose(fp);

}
void SphereRenderer::LoadTextureDataPBO() { }

void SphereRenderer::InitializeTexture(TC_TYPES curr_type) {

//TODO:: Add separate function for each texture and call them
  switch (curr_type) {

    case ASTC4x4:
      InitializeASTC4x4Texture();
      break;
    case ASTC8x8:
      InitializeASTC8x8Texture();
      break;
    case ASTC12x12:
      break;
    case DXT1:
      InitializeDXT1Texuture();
      break;
    case MPTC:
      InitializeMPTCTexture();
      break;
    case JPG:
      InitializeFullTexture();
      break;
    case CRN:
      InitializeCRNTexture();
      break;
  }

}

void SphereRenderer::LoadTexture(TC_TYPES curr_type) {

  switch (curr_type) {
    case ASTC4x4:
      LoadTextureDataASTC4x4();
      break;
    case ASTC8x8:
      LoadTextureDataASTC8x8();
      break;
    case ASTC12x12:
      break;
    case DXT1:
      LoadTextureDataDXT1();
      break;
    case MPTC:
      LoadTextureDataMPTC();
      break;
    case JPG:
      LoadTextureDataJPG();
      break;
    case CRN:
      LoadTextureDataCRN();
      break;
  }
}

void SphereRenderer::LoadShaders(const char *vertex_shader, const char *frag_shader) {

  // Get Id's for the create shaders
  GLuint verShdrId = CHECK_GL(glCreateShader,GL_VERTEX_SHADER );
  GLuint fragShdrId = CHECK_GL(glCreateShader, GL_FRAGMENT_SHADER);

  // Give the Shader Source and give the Id to compile
  CHECK_GL(glShaderSource,verShdrId, 1, &vVertexProg, NULL );
  CHECK_GL(glCompileShader, verShdrId);

  // check vertex shader
#ifdef DEBUG
  int result, logLength;
  CHECK_GL(glGetShaderiv, verShdrId, GL_COMPILE_STATUS, &result);

  if(result != GL_TRUE){
    CHECK_GL(glGetShaderiv, verShdrId, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> VertexShaderErrorMsg(logLength);
    CHECK_GL(glGetShaderInfoLog, verShdrId, logLength, NULL, &VertexShaderErrorMsg[0]);
    LOGE("Error while compiling vertex shader", &VertexShaderErrorMsg[0]);
    exit(1);
  }
#endif
  // create the fragment shader
  CHECK_GL(glShaderSource, fragShdrId, 1, &vFragProg, NULL);
  CHECK_GL(glCompileShader, fragShdrId);
  CHECK_GL(glGetShaderiv, fragShdrId, GL_COMPILE_STATUS, &result);

#ifdef DEBUG
  if(result != GL_TRUE)
  {
    CHECK_GL(glGetShaderiv, fragShdrId, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> FragmentShaderErrMsg(logLength);
    CHECK_GL(glGetShaderInfoLog, fragShdrId, logLength, NULL, &FragmentShaderErrMsg[0]);
    LOGE("Error while comipling fragment shader", &FragmentShaderErrMsg[0]);
    exit(1);
  }
#endif
  _program_id = CHECK_GL(glCreateProgram);
  CHECK_GL(glAttachShader, _program_id, verShdrId);
  CHECK_GL(glAttachShader, _program_id, fragShdrId);
  CHECK_GL(glLinkProgram, _program_id);

#ifdef  DEBUG
  CHECK_GL(glGetProgramiv, _program_id, GL_LINK_STATUS, &result);
  if(result != GL_TRUE){

    CHECK_GL(glGetProgramiv, _program_id, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> ProgramErrMsg(logLength);
    CHECK_GL(glGetProgramInfoLog, _program_id, logLength, NULL, &ProgramErrMsg[0]);
    LOGE("Error while Linking the program", &ProgramErrMsg[0]);

  }
#endif

  if(verShdrId) CHECK_GL(glDeleteShader, verShdrId);
  if(fragShdrId) CHECK_GL(glDeleteShader, fragShdrId);

}


void SphereRenderer::Update(float fTime) {
  const float CAM_X = 0.f;
  const float CAM_Y = 0.f;
  const float CAM_Z = 700.f;

  _mat_view = ndk_helper::Mat4::LookAt(ndk_helper::Vec3(CAM_X, CAM_Y, CAM_Z),
                                       ndk_helper::Vec3(0.f, 0.f, 0.f),
                                       ndk_helper::Vec3(0.f, 1.f, 0.f));

  if (_camera) {
    _camera->Update();
    _mat_view = _camera->GetTransformMatrix() * _mat_view *
        _camera->GetRotationMatrix() * _mat_model;
  } else {
    _mat_view = _mat_view * _mat_model;
  }
}

void SphereRenderer::Init(const char* storage_path) {

  _texture_path = std::string(storage_path) + "/Textures";
  _obj_path = std::string(storage_path) + "/Obj/sphere.obj";
  _metrics_path = std::string(storage_path) + "/output.txt";
#ifndef  NDEBUG
  LOGE("Still in debug mode!");
#endif
   //TODO:: ADD MPTC file paths and input streamsa
   //Set MPTC pointers to NULL. Set the initializations in LoadTexture()
  _mptc_file_path = std::string(storage_path) + "/MPTC/360MegaC2K_16_50.mpt";
  LOGE("MPTC PATH %s\n", _mptc_file_path.c_str());

  ptr_buffer_struct = NULL;
  fp_mptc = NULL;
  curr_dxt = NULL;
  prev_dxt = NULL;
  temp_dxt = NULL;
  ptr_decode_info = NULL;

  _texture_number = 1;
  _texture_height = vImage_height;
  _texture_width = vImage_width;

  CHECK_GL(glGenVertexArrays, 1, &_vertex_array_id);
  CHECK_GL(glBindVertexArray, _vertex_array_id);

  CHECK_GL(glDisable, GL_CULL_FACE);
  LoadShaders(vVertexProg, vFragProg);

  _texture_height = vImage_height;
  _texture_width = vImage_width;
  _curr_TC_type = vTC_type;

  LoadShaders(vVertexProg, vFragProg);
  InitializeScene();
  InitializeTexture(_curr_TC_type);
  UpdateViewport();

  _mat_model = ndk_helper::Mat4::Translation(0, 0, 0.f);

  ndk_helper::Mat4 mat = ndk_helper::Mat4::RotationX(0);
  _mat_model = mat * _mat_model;

  _pos_loc = CHECK_GL(glGetAttribLocation, _program_id, "position");
  assert ( _pos_loc >= 0 );

  _uv_loc = CHECK_GL(glGetAttribLocation, _program_id, "texCoord");
  assert ( _uv_loc >= 0 );

  _tex_loc = CHECK_GL(glGetUniformLocation, _program_id, "tex");
  assert ( _tex_loc >= 0 );




}

bool SphereRenderer::Bind(ndk_helper::TapCamera* camera) {
  _camera = camera;
  return true;
}

SphereRenderer::~SphereRenderer() {
  Unload();

}

void SphereRenderer::Render() {

  // Feed Projection and Model View matrices to the shaders
  ndk_helper::Mat4 mat_vp = _mat_projection * _mat_view;
  //_texture_number = 5;
  _texture_number = (_texture_number + 1) % vMax_tex_count + 1;
  LoadTexture(_curr_TC_type);
  CHECK_GL(glUseProgram, _program_id);
  CHECK_GL(glClear, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  CHECK_GL(glUniformMatrix4fv, _matrix_id, 1, GL_FALSE, mat_vp.Ptr());

  CHECK_GL(glActiveTexture, GL_TEXTURE0);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id);


  CHECK_GL(glEnableVertexAttribArray, _pos_loc);
  CHECK_GL(glBindBuffer, GL_ARRAY_BUFFER, _vertex_buffer);
  CHECK_GL(glVertexAttribPointer,_pos_loc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  CHECK_GL(glBindBuffer,GL_ARRAY_BUFFER, 0);

  CHECK_GL(glEnableVertexAttribArray,_uv_loc);
  CHECK_GL(glBindBuffer,GL_ARRAY_BUFFER, _uv_buffer);
  CHECK_GL(glVertexAttribPointer,_uv_loc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  CHECK_GL(glBindBuffer,GL_ARRAY_BUFFER, 0);

  CHECK_GL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, _index_buffer);
  CHECK_GL(glDrawElements, GL_TRIANGLES, _num_indices , GL_UNSIGNED_SHORT, NULL);
  LOGE("Num indices %d\n", _num_indices);


  CHECK_GL(glDisableVertexAttribArray,_pos_loc);
  CHECK_GL(glDisableVertexAttribArray,_uv_loc);
  CHECK_GL(glBindBuffer, GL_ARRAY_BUFFER, 0);
  CHECK_GL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
  CHECK_GL(glUseProgram, 0);

}

void SphereRenderer::Unload() {
  CHECK_GL(glDeleteBuffers, 1, &_vertex_buffer);
  CHECK_GL(glDeleteBuffers, 1, &_uv_buffer);
  CHECK_GL(glDeleteBuffers, 1, &_index_buffer);
  CHECK_GL(glDeleteProgram, _program_id);
  CHECK_GL(glDeleteTextures, 1, &_texture_id);
  //CHECK_GL(glDeleteBuffers, 2, m_PboId);
  CHECK_GL(glDeleteVertexArrays, 1, &_vertex_array_id);
  //if(curr_dxt) delete [] curr_dxt;
  //if(prev_dxt) delete [] prev_dxt;
  if(ptr_decode_info) delete ptr_decode_info;
  if(ptr_buffer_struct) delete ptr_buffer_struct;

}