

# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
include_directories(${ANDROID_SDK})
include_directories(${CMAKE_SOURCE_DIR}$/glm)

MESSAGE(STATUS "CMAKE_SOURCEE_DIR: " ${CMAKE_SOURCE_DIR}/glm)
set(VK_SRC_DIR ${ANDROID_NDK}/sources/third_party/vulkan/src)

include_directories(${VK_SRC_DIR}/include)
add_library(vkgpu SHARED
            vk_gpu.cpp
            validation_layer.cpp
            vulkan_wrapper.cpp)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror \
                     -DVK_USE_PLATFORM_ANDROID_KHR \
                     -DUSE_DEBUG_EXTENTIONS" )
target_link_libraries(vkgpu app-glue log android)