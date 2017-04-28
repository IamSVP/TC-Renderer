//
// Created by Srihari Pratapa on 3/30/17.
//


#ifndef __TC_RENDERER_VK_GUARDS_H__
#define __TC_RENDERER_VK_GUARDS_H__

#include "vulkan_wrapper.h"


#include <android/log.h>
#include <cassert>

#ifndef NDEBUG
static inline const char *vkErrMsg(VkResult result) {
  const char *err_msg = NULL;
  switch(result) {
    case VK_NOT_READY:                      err_msg = "VK_NOT_READY A fence or query has not yet completed"; break;
    case VK_TIMEOUT:                        err_msg = "VK_TIMEOUT A wait operation has not completed in the specified time"; break;
    case VK_EVENT_SET:                      err_msg = "VK_EVENT_SET An event is signaled"; break;
    case VK_EVENT_RESET:                    err_msg = "VK_EVENT_RESET An event is unsignaled"; break;
    case VK_INCOMPLETE:                     err_msg = "VK_INCOMPLETE A return array was too small for the result"; break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:       err_msg = "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed."; break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:     err_msg = "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has failed."; break;
    case VK_ERROR_INITIALIZATION_FAILED:    err_msg = "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could not be completed for implementation-specific reasons."; break;
    case VK_ERROR_DEVICE_LOST:              err_msg = "VK_ERROR_DEVICE_LOST The logical or physical device has been lost."; break;
    case VK_ERROR_MEMORY_MAP_FAILED:        err_msg = "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed."; break;
    case VK_ERROR_LAYER_NOT_PRESENT:        err_msg = "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or could not be loaded."; break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:    err_msg = "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not supported."; break;
    case VK_ERROR_FEATURE_NOT_PRESENT:      err_msg = "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported."; break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:      err_msg = "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons"; break;
    case VK_ERROR_TOO_MANY_OBJECTS:         err_msg = "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have already been created"; break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED:     err_msg = "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported on this device"; break;
    case VK_ERROR_FRAGMENTED_POOL:          err_msg = "VK_ERROR_FRAGMENTED_POOL A requested pool allocation has failed due to fragmentation of the pool’s memory."; break;
    default: err_msg = NULL;
  }
  return  err_msg;
}

#define CHECK_VK(fn, ...)                                                      \
do  {                                                                           \
  VkResult result = fn(__VA_ARGS__);                                           \
  if(VK_SUCCESS != result) {                                                    \
    const char* err_msg = vkErrMsg(result);                                     \
    if(NULL != err_msg) {                                                       \
      __android_log_print(ANDROID_LOG_ERROR, " ",                               \
                        "File[%s], line[%d], Vulkan Error: %s ", __FILE__,      \
                        __LINE__, err_msg);                                     \
     } else {                                                                   \
      __android_log_print(ANDROID_LOG_ERROR, "  ",                              \
                        "File[%s], line[%d], Unknown Vulkan Error",  __FILE__, \
                        __LINE__);                                              \
    }                                                                           \
    assert(false);                                                              \
  }                                                                               \
} while(0)
#else
#define CHECK_VK(fn ...) do {(void)(fn(__VA_ARGS__));} while(0)
#endif

#endif //TC_RENDERER_VK_GUARDS_H
