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

typedef struct {
int fd;
AMediaExtractor* ex;
AMediaCodec *codec;
int64_t renderstart;
bool sawInputEOS;
bool sawOutputEOS;
bool isPlaying;
bool renderonce;
} workerdata;

workerdata worker_data = {-1, NULL, NULL, 0, false, false, false, false};
workerdata *d;

int64_t systemnanotime() {
  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec * 1000000000LL + now.tv_nsec;
}

static double GetAverageTimeMS(const std::vector<uint64_t> &times) {
	double sum = 0.0;
	for (const auto &t : times) {
		sum += static_cast<double>(t) / 1e6;
	}
	return sum / static_cast<double>(times.size());
}

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

  int type = static_cast<int>(TC_TYPES::JPG);
  CHECK_GL(glGenTextures, 1, &_texture_id[type]);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[type]);
  // CHECK_GL(glTexImage2D, GL_TEXTURE_2D,0,GL_RGBA8UI, vImageWidth, vImageHeight,0,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,NULL);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, GL_RGBA8,vImage_width, vImage_height );
  CHECK_GL(glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindImageTexture, 2, _texture_id[type], 0, GL_FALSE,0, GL_WRITE_ONLY, GL_RGBA8UI); // possible error
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
}
void SphereRenderer::InitializeDXT1Texuture() {

  int type = static_cast<int>(TC_TYPES::DXT1);
  CHECK_GL(glGenTextures, 1, &_texture_id[type]);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[type]);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGB_DXT1, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);

}
void SphereRenderer::InitializeCRNTexture() {

  int type = static_cast<int>(TC_TYPES::CRN);
  CHECK_GL(glGenTextures, 1, &_texture_id[type]);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[type]);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGB_DXT1, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
}
void SphereRenderer::InitializeASTC8x8Texture() {

  int type = static_cast<int>(TC_TYPES::ASTC8x8);
  CHECK_GL(glGenTextures, 1, &_texture_id[type]);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[type]);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGBA_ASTC_8x8, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);

}
void SphereRenderer::InitializeASTC4x4Texture() {

  int type = static_cast<int>(TC_TYPES::ASTC4x4);
  CHECK_GL(glGenTextures, 1, &_texture_id[type]);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[type]);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, COMPRESSED_RGBA_ASTC_4x4, vImage_width, vImage_height);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
}
void SphereRenderer::InitializeMPTCTexture() {

  int type = static_cast<int>(TC_TYPES::MPTC);
  CHECK_GL(glGenTextures, 1, &_texture_id[type]);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[type]);
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
  InitBufferedDecode(7, ptr_buffer_struct, _mptc_stream, _num_blocks);

  //fp_mptc = fopen(_mptc_file_path.c_str(), "rb");
  LOGE("Called load Texture Data,,,,,");
}

void SphereRenderer::InitializeMPEGTexture() {

  int type = static_cast<int>(TC_TYPES::MPEG);
  CHECK_GL(glGenTextures, 1, &_texture_id[type]);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[type]);
  // CHECK_GL(glTexImage2D, GL_TEXTURE_2D,0,GL_RGBA8UI, vImageWidth, vImageHeight,0,GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,NULL);
  CHECK_GL(glTexStorage2D, GL_TEXTURE_2D, 1, GL_RGBA8,vImage_width, vImage_height );
  CHECK_GL(glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  CHECK_GL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CHECK_GL(glBindImageTexture, 2, _texture_id[type], 0, GL_FALSE,0, GL_WRITE_ONLY, GL_RGBA8UI); // possible error
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);

  int fd = open(_mpeg_file_path.c_str(), O_RDONLY);
  worker_data.fd = fd;

  d = &worker_data;

  AMediaExtractor *ex = AMediaExtractor_new();
  media_status_t err = AMediaExtractor_setDataSourceFd(ex, d->fd, 0 , LONG_MAX);
  close(d->fd);
  if (err != AMEDIA_OK) {
    LOGE("setDataSource error: %d", err);
  }
  int numtracks = AMediaExtractor_getTrackCount(ex);

  AMediaCodec *codec = NULL;

  LOGI("input has %d tracks", numtracks);

  for (int i = 0; i <= 0; i++) {
    AMediaFormat *format = AMediaExtractor_getTrackFormat(ex, i);
    const char *s = AMediaFormat_toString(format);
    LOGI("track %d format: %s", i, s);
    const char *mime;
    if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
      LOGI("no mime type %s\n", mime);
//            return JNI_FALSE;
    } else if (!strncmp(mime, "video/", 6)) {
      // Omitting most error handling for clarity.
      // Production code should check for errors.
      AMediaExtractor_selectTrack(ex, i);
      codec = AMediaCodec_createDecoderByType(mime);
      AMediaCodec_configure(codec, format, NULL, NULL, 0);

      d->ex = ex;
      d->codec = codec;
      d->renderstart = -1;
      d->sawInputEOS = false;
      d->sawOutputEOS = false;
      d->isPlaying = false;
      d->renderonce = true;
      AMediaCodec_start(codec);
    }
    AMediaFormat_delete(format);
  }


}
void SphereRenderer::LoadTextureDataASTC4x4() {

  int tc_type = static_cast<int>(TC_TYPES::ASTC4x4);
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
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[tc_type]);
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

//  GetFrameMultiThread(_mptc_stream,
//                      prev_dxt,
//                      curr_dxt,
//                      ptr_decode_info);
//  GetFrame(_mptc_stream, prev_dxt, curr_dxt, ptr_decode_info);


  int tc_type = static_cast<int>(TC_TYPES::MPTC);
  std::chrono::high_resolution_clock::time_point GPULoad_Start = std::chrono::high_resolution_clock::now();
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[tc_type]);
  CHECK_GL(glCompressedTexSubImage2D, GL_TEXTURE_2D,    // Type of texture
           0,                // level (0 being the top level i.e. full size)
           0, 0,             // Offset
           _texture_width,       // Width of the texture
           _texture_height,      // Height of the texture,
           COMPRESSED_RGB_DXT1,          // Data format
           _texture_width*_texture_height / 2, // Type of texture data
           //ptr_buffer_struct->buffered_dxts[ptr_buffer_struct->curr_dxt_idx]);
           curr_dxt);
  std::chrono::high_resolution_clock ::time_point GPULoad_End = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds GPULoad_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(GPULoad_End - GPULoad_Start);
  m_GPULoad.push_back(GPULoad_Time.count());


  LOGE("curr_dxt id value:%d\n", ptr_buffer_struct->curr_dxt_idx);


  std::chrono::high_resolution_clock::time_point CPUDecode_Start = std::chrono::high_resolution_clock::now();
  GetBufferedFrame(ptr_buffer_struct, curr_dxt, _mptc_stream);
  std::chrono::high_resolution_clock::time_point CPUDecode_End = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds CPUDecode_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(CPUDecode_End - CPUDecode_Start);
  m_CPUDecode.push_back(CPUDecode_Time.count());


  // CHECK_GL(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, 0);
  // ALOGE("fooooddd here\n");
  //temp_dxt = prev_dxt;
  //prev_dxt = curr_dxt;
  //curr_dxt = temp_dxt;
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);

}

void SphereRenderer::LoadTextureDataJPG() {
  char img_path[256];

  int tc_type = static_cast<int>(TC_TYPES::JPG);
  //sprintf(img_path, "%s/360MegaCoaster720/JPG/360MegaCoaster720%06d.jpg",_texture_path.c_str(),_texture_number);
  sprintf(img_path, "%s/360MegaC2K/JPG/360MegaC2K%03d.jpg", _texture_path.c_str(), _texture_number);
  LOGE("Path%s", img_path);
  int w,h,n;
  std::chrono::high_resolution_clock::time_point CPUDecode_Start = std::chrono::high_resolution_clock::now();
  unsigned char *ImageDataPtr = (unsigned char *)malloc((_texture_height * _texture_width * 4));
  FILE *fp = fopen(img_path, "rb");


  //CPU LOADING.....
  fread(ImageDataPtr, 1, _texture_height * _texture_width * 4, fp);
  std::chrono::high_resolution_clock::time_point CPULoad_End = std::chrono::high_resolution_clock::now();



  // CPU Decoding.....

  unsigned char *TextureData = stbi_load_from_memory(ImageDataPtr, (_texture_height * _texture_width * 4), &w, &h, &n, 4);
  std::chrono::high_resolution_clock::time_point CPUDecode_End = std::chrono::high_resolution_clock::now();

  std::chrono::nanoseconds CPUDecode_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(CPUDecode_End - CPUDecode_Start);
  m_CPUDecode.push_back(CPUDecode_Time.count());
  free(ImageDataPtr);

  LOGE("checking the loaded texture: %d %d %d \n", w, h, _texture_number);


  // GPU Loading........
  std::chrono::high_resolution_clock::time_point GPULoad_Start = std::chrono::high_resolution_clock::now();
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[tc_type]);

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
void SphereRenderer::LoadTextureDataMPEG() {

  ssize_t bufidx = -1;
  ssize_t outbufidx = -1;
  uint8_t *TextureData = NULL;
  size_t TexSz = 0;
  int tc_type = static_cast<int>(TC_TYPES::MPEG);
  std::chrono::high_resolution_clock::time_point CPU_start =
    std::chrono::high_resolution_clock::now();

  if (!d->sawInputEOS) {
    bufidx = AMediaCodec_dequeueInputBuffer(d->codec, -1);
    LOGE("input buffer %zd---------", bufidx);
    if (bufidx >= 0) {

      size_t bufsize;
      uint8_t *buf = AMediaCodec_getInputBuffer(d->codec, bufidx, &bufsize);
      ssize_t sampleSize = AMediaExtractor_readSampleData(d->ex, buf, bufsize);

      if (sampleSize < 0) {
        sampleSize = 0;
        d->sawInputEOS = true;
        LOGI("EOS");
      }

      int64_t presentationTimeUs = AMediaExtractor_getSampleTime(d->ex);

      AMediaCodec_queueInputBuffer(d->codec, bufidx, 0, sampleSize, presentationTimeUs,
                                   d->sawInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
      LOGE("sample size buffer %zd----------------", sampleSize);

      AMediaExtractor_advance(d->ex);
    }


  }

  if (!d->sawOutputEOS) {
    AMediaCodecBufferInfo info;
    LOGE("fucking waiting here...\n");
    ssize_t status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);

    if (status >= 0) {
      if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
        LOGI("output EOS");
        d->sawOutputEOS = true;
      }
      TextureData = AMediaCodec_getOutputBuffer(d->codec, status, &TexSz);
      AMediaCodec_releaseOutputBuffer(d->codec, status, info.size != 0);
      int64_t presentationNano = info.presentationTimeUs * 1000;
      if (d->renderstart < 0) {
        d->renderstart = systemnanotime() - presentationNano;
      }
      int64_t delay = (d->renderstart + presentationNano) - systemnanotime();
      if (delay > 0) {
        usleep(delay / 1000);
      }

      if (d->renderonce) {
        d->renderonce = false;
        return;
      }
      std::chrono::high_resolution_clock::time_point CPU_end =
      std::chrono::high_resolution_clock::now();

      std::chrono::nanoseconds cpu_load =
      std::chrono::duration_cast<std::chrono::nanoseconds>(CPU_end - CPU_start);
      m_CPUDecode.push_back(cpu_load.count());
      // GPU Loading........
      GLuint64 gpu_load_time;

      LOGI("Checking frame size %lld\n", TexSz);
      std::chrono::high_resolution_clock::time_point GPULoad_Start = std::chrono::high_resolution_clock::now();
      CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[tc_type]);

      CHECK_GL(glTexSubImage2D, GL_TEXTURE_2D,
               0,
               0, 0,
               _texture_width,
               _texture_height,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               TextureData);
      std::chrono::high_resolution_clock::time_point GPULoad_End = std::chrono::high_resolution_clock::now();
      std::chrono::nanoseconds GPULoad_Time = std::chrono::duration_cast<std::chrono::nanoseconds>(
      GPULoad_End - GPULoad_Start);
      m_GPULoad.push_back(GPULoad_Time.count());

      CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
    } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
      LOGI("output buffers changed");
    } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
      AMediaFormat *format = NULL;
      format = AMediaCodec_getOutputFormat(d->codec);
      LOGI("format changed to: %s", AMediaFormat_toString(format));
      AMediaFormat_delete(format);
    } else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
      LOGI("no output buffer right now");
    } else {
      LOGI("unexpected info code: %zd", status);
    }

  }

}

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
    case MPEG:
      InitializeMPEGTexture();
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
    case MPEG:
      LoadTextureDataMPEG();
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
#ifndef NDEBUG
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


#ifndef NDEBUG
  CHECK_GL(glGetShaderiv, fragShdrId, GL_COMPILE_STATUS, &result);
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

#ifndef  NDEBUG
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
  LOGI("Still in debug mode!");
#endif
   //TODO:: ADD MPTC file paths and input streamsa
   //Set MPTC pointers to NULL. Set the initializations in LoadTexture()
  _mptc_file_path = std::string(storage_path) + "/MPTC/360MegaC2K_16_50.mpt";
  LOGE("MPTC PATH %s\n", _mptc_file_path.c_str());
  _mpeg_file_path = std::string(storage_path) + "/dongtan_day.mp4";

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
  //InitializeTexture(_curr_TC_type);
  InitializeMPTCTexture();
  InitializeDXT1Texuture();
  InitializeFullTexture();
  UpdateViewport();
  CHECK_GL(glGenQueries, 2, GPULoadQuery);

  _mat_model = ndk_helper::Mat4::Translation(0, 0, 0.f);

  ndk_helper::Mat4 mat = ndk_helper::Mat4::RotationX(0);
  _mat_model = mat * _mat_model;

  _pos_loc = CHECK_GL(glGetAttribLocation, _program_id, "position");
  assert ( _pos_loc >= 0 );

  _uv_loc = CHECK_GL(glGetAttribLocation, _program_id, "texCoord");
  assert ( _uv_loc >= 0 );

  _tex_loc = CHECK_GL(glGetUniformLocation, _program_id, "tex");
  assert ( _tex_loc >= 0 );

  load_texture = true;
  load_count = 1;


}

bool SphereRenderer::Bind(ndk_helper::TapCamera* camera) {
  _camera = camera;
  return true;
}

SphereRenderer::~SphereRenderer() {
  Unload();

}

void SphereRenderer::Render() {
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  // Feed Projection and Model View matrices to the shaders
  ndk_helper::Mat4 mat_vp = _mat_projection * _mat_view;
  //_texture_number = 5;
  _texture_number = (_texture_number + 1) % vMax_tex_count + 1;
  static bool first_frame = true;
  if(load_texture)
    LoadTexture(_curr_TC_type);

<<<<<<< HEAD
//  if(_curr_TC_type == TC_TYPES::MPTC) {
//    load_texture = false;
//    load_count++;
//    if (load_count > 10) {
//      load_count = 1;
//      load_texture = true;
//    }
//  }
 //LOGE("TC Type %d\n", _curr_TC_type);

    int tc_type = static_cast<int>(_curr_TC_type);
=======
  if(_curr_TC_type == TC_TYPES::MPTC) {
    load_texture = false;
    load_count++;
    if (load_count > 10) {
      load_count = 1;
      load_texture = true;
    }
  }

>>>>>>> parent of dc05568... Added buttons to switch between algorithms, needs debugging
  CHECK_GL(glUseProgram, _program_id);
  CHECK_GL(glClear, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  CHECK_GL(glUniformMatrix4fv, _matrix_id, 1, GL_FALSE, mat_vp.Ptr());

  CHECK_GL(glActiveTexture, GL_TEXTURE0);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, _texture_id[tc_type]);


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
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds frame_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  m_TotalFps.push_back(frame_time.count());

  CHECK_GL(glDisableVertexAttribArray,_pos_loc);
  CHECK_GL(glDisableVertexAttribArray,_uv_loc);
  CHECK_GL(glBindBuffer, GL_ARRAY_BUFFER, 0);
  CHECK_GL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
  CHECK_GL(glBindTexture, GL_TEXTURE_2D, 0);
  CHECK_GL(glUseProgram, 0);

  m_numframes = m_numframes + 1;
  if(m_numframes >= 20){

    uint64_t GCPU_load, GCPU_decode, GGPU_Load, GFPS;

    if(!m_GPULoad.empty()) {
      GPU_Load = GetAverageTimeMS(m_GPULoad);
    }
    if(!m_TotalFps.empty()) {
      Total_time = GetAverageTimeMS(m_TotalFps);
    }
    if(!m_CPUDecode.empty()) {
      CPU_load = GetAverageTimeMS(m_CPUDecode);
    }

    //LOGI("CPU Load Time : %lld--%d\n", CPU_load, m_CPULoad.size());
    //LOGI("CPU Decode Time: %lld--%d\n", CPU_decode, m_CPUDecode.size());
    LOGI("GPU Load Time: %f\n", GPU_Load);
    //LOGI("FPS: %lld--%d\n", FPS, m_TotalFps.size());

    m_CPULoad.clear();
    m_CPUDecode.clear();
    m_GPULoad.clear();
    m_TotalFps.clear();


    m_numframes = 0;
  }


}

void SphereRenderer::Unload() {
  CHECK_GL(glDeleteBuffers, 1, &_vertex_buffer);
  CHECK_GL(glDeleteBuffers, 1, &_uv_buffer);
  CHECK_GL(glDeleteBuffers, 1, &_index_buffer);
  CHECK_GL(glDeleteProgram, _program_id);
  CHECK_GL(glDeleteTextures, 1, &_texture_id[TC_TYPES::JPG]);
  CHECK_GL(glDeleteTextures, 1, &_texture_id[TC_TYPES::MPTC]);
  CHECK_GL(glDeleteTextures, 1, &_texture_id[TC_TYPES::DXT1]);
  //CHECK_GL(glDeleteBuffers, 2, m_PboId);
  CHECK_GL(glDeleteVertexArrays, 1, &_vertex_array_id);
  //if(curr_dxt) delete [] curr_dxt;
  //if(prev_dxt) delete [] prev_dxt;
  if(ptr_decode_info) delete ptr_decode_info;
  if(ptr_buffer_struct) delete ptr_buffer_struct;

}