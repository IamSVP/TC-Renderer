//
// Created by Srihari Pratapa on 11/13/16.
//

#ifndef TC_RENDERER_GL_GUARDS_H
#define TC_RENDERER_GL_GUARDS_H

#include <GLES3/gl31.h>
#include <cassert>
#include "NDKHelper.h"

#ifndef NDEBUG

static const char* getGLErrString(GLenum err){

    const char *errString = "Unknown error";
    switch(err){
        case GL_INVALID_ENUM: errString = "Invalid enum"; break;
        case GL_INVALID_VALUE: errString = "Invalid value"; break;
        case GL_INVALID_OPERATION: errString = "Invalid operation"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errString = "Invalid Frame Buffer Operation"; break;
        case GL_OUT_OF_MEMORY: errString = "Out of Memory Operation"; break;
    }
    return errString;
}


#define CHECK_GL(fn, ...) fn(__VA_ARGS__);                                           \
  do {                                                                               \
    GLenum glErr = glGetError();                                                     \
    if(glErr != GL_NO_ERROR) {                                                       \
      const char *errString = getGLErrString(glErr);                                 \
      if(NULL != errString){                                                         \
        LOGE("OpenGL call Error (%s : %d): %s\n",__FILE__, __LINE__, errString);    \
      } else {                                                                       \
        LOGE("Unknown OpenGL call Error(%s : %d)\n", __FILE__, __LINE__);                 \
       }                                                                             \
      assert(false);                                                                 \
    }                                                                                \
  } while(0)
#else
#define CHECK_GL(fn, ...) fn(__VA_ARGS__);
#endif
#endif //TC_RENDERER_GL_GUARDS_H
