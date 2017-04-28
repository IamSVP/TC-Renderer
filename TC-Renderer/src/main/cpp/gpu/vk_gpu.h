//
// Created by Srihari Pratapa on 3/30/17.
//

#ifndef __TC_RENDERER_VK_GPU_H__
#define __TC_RENDERER_VK_GPU_H__
//TODO:Do not forget to set the DVK_USE_PLATFORM_ANDROID_KHR flag in the cake for window
//TODO:Make Sure the proper instance extensions and device extensions are called
#include "vulkan_wrapper.h"
#include "validation_layer.hpp"
#include <functional>
#include <vector>


//Forward Declarations of various structures
struct android_app;

namespace GPU {

#ifndef NDEBUG
static const bool enable_validation_layers = true;
#else
static const bool enable_validation_layers = false;
#endif

struct QueueFamilyIndices;
struct SwapChainSupportDetails;
template<typename T>
class VDeleter {
public:
VDeleter() : VDeleter([](T, VkAllocationCallbacks *) { }) { }
VDeleter(std::function<void(T, VkAllocationCallbacks *)> deletef) {
  this->deleter = [=](T obj) { deletef(obj, nullptr); };
}
VDeleter(const VDeleter<VkInstance> &instance,
         std::function<void(VkInstance, T, VkAllocationCallbacks *)> deletef) {
  this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
}
VDeleter(const VDeleter<VkDevice> &device,
         std::function<void(VkDevice, T, VkAllocationCallbacks *)> deletef) {
  this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
}
~VDeleter() {
  cleanup();
}
const T *operator&() const {
  return &object;
}
T *replace() {
  cleanup();
  return &object;
}
operator T() const {
  return object;
}
void operator=(T rhs) {
  if (rhs != object) {
    cleanup();
    object = rhs;
  }
}
template<typename V>
bool operator==(V rhs) {
  return object == T(rhs);
}
private:
T object{VK_NULL_HANDLE};
std::function<void(T)> deleter;
void cleanup() {
  if (object != VK_NULL_HANDLE) {
    deleter(object);
  }
  object = VK_NULL_HANDLE;
}
};

///////////////////////////////////
//        Debug Report Callbacks //
//////////////////////////////////

VkResult CreateDebugReportCallback(VkInstance instance,
                                   const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator,
                                   VkDebugReportCallbackEXT *pCallback) {
  auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance,
                                                                         "vkCreateDebugReportCallbackEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  }
  else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}
void DestroyDebugReportCallbackEXT(VkInstance instance,
                                   VkDebugReportCallbackEXT callback,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance,
                                                                          "vkDestroyDebugReportCallbackEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}

////////////////////////////////////
//**** End of Debug Report ******//
//////////////////////////////////





class VKGPUContext {
public:
VKGPUContext(bool &init);
VKGPUContext(VkApplicationInfo info, bool &init);

//Methods
public:
void CreateInstance();
void SetupDebugCallback();
void CreateSurface(android_app* app);
void PickPhysicalDevice();
void CreateLogicalDevice();


void CreateSwapChains();

void CreateImageViews();
void CreateRenderPass();

void CreateGraphicsPipeline();
void CreateShaderModule(const std::vector<char> &code, VDeleter<VkShaderModule>& shader_module);
void CreateDescriptorSetLayout();
void CreateCommandPool();

void CreateFramebuffers();

void CreateCommandBuffers();


//Texture and image creation functions
void CreateTextureImage();
void UpdateTextureImage();
void CreateImage(uint32_t width, uint32_t height,
                  VkFormat format,
                  VkImageTiling tiling,
                  VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkImageLayout initial_layout,
                  VDeleter<VkImage> &image,
                  VDeleter<VkDeviceMemory> &image_memory);
void CreateTextureImageView();
void CreateIndexBuffers();
void CreateVertexBuffers();
void CreateUniformbuffers();
void CreateDescriptorPool();
void CreateDescriptorSets();
void CreateSemaphores();
void CreateTextureSampler();




private:
//    Private helper functions to make decisions about kind of
//    settings we want in different object and stages of vulkan
bool IsDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
void CreateImageView(VkImage image, VkFormat format, VDeleter<VkImageView> &image_view);

//Helper functions to start and end command buffer recordings
VkCommandBuffer BeginSingleTimeCommands();
void EndSingleTimeCommands(VkCommandBuffer command_buffer,
                           VkQueue queue);
void CopyImage(VkImage src_image, VkImage dst_image, uint32_t width, uint32_t height);

// Function to change the layout of the image for transition
void TransitionImageLayout(VkImage image, VkFormat format,
                           VkImageLayout old_layout, VkImageLayout new_layout);
//Image data mapped pointer
void *image_data;
int32_t tex_img_count = 0;

private:

//application info
VkApplicationInfo _app_info;
//instance

VDeleter<VkInstance> _instance{vkDestroyInstance};
// Debug Report callback
LayerAndExtensions _layer_ext;
VDeleter<VkDebugReportCallbackEXT> _debug_callback{_instance, DestroyDebugReportCallbackEXT};
// Surface KHR
VDeleter<VkSurfaceKHR> _surface{_instance, vkDestroySurfaceKHR};
//physical device
VkPhysicalDevice _physical_device;
// logical device
VDeleter<VkDevice> _logical_device{vkDestroyDevice};

//VkQueues physical, present and
VkQueue _graphics_queue;
VkQueue _compute_queue;
VkQueue _present_queue;
VkQueue _transfer_queue;

//VkSwapChain KHR
VDeleter<VkSwapchainKHR> _swap_chain{_logical_device, vkDestroySwapchainKHR};
std::vector<VkImage> _swap_chain_images;
VkFormat _swap_chain_image_format;
VkExtent2D _swap_chain_extent;
std::vector<VDeleter<VkImageView> > _swapchain_image_views;
VDeleter<VkRenderPass> _render_pass{_logical_device, vkDestroyRenderPass};
VDeleter<VkDescriptorSetLayout> _descriptor_set_layout{_logical_device,
                                                      vkDestroyDescriptorSetLayout};
VDeleter<VkPipelineLayout> _pipeline_layout{_logical_device, vkDestroyPipelineLayout};
VDeleter<VkPipeline> _graphics_pipeline{_logical_device, vkDestroyPipeline};
std::vector< VDeleter<VkFramebuffer> > _swapchain_framebuffers;
VDeleter<VkCommandPool> _command_pool{_logical_device, vkDestroyCommandPool};
VDeleter<VkCommandPool> _transfer_command_pool{_logical_device, vkDestroyCommandPool};


//Texture Images and textures
VDeleter<VkImage> _staging_image{_logical_device, vkDestroyImage};
VDeleter<VkDeviceMemory> _staging_image_memory{_logical_device, vkFreeMemory};
VDeleter<VkImage> _texture_image{_logical_device, vkDestroyImage};
VDeleter<VkDeviceMemory> _texture_image_memory{_logical_device, vkFreeMemory};
VDeleter<VkImageView> _texture_image_view{_logical_device, vkDestroyImageView};
VDeleter<VkSampler> _texture_sampler{_logical_device, vkDestroySampler};


//Buffers and Memory
VDeleter<VkBuffer> _vertex_buffer{_logical_device, vkDestroyBuffer};
VDeleter<VkDeviceMemory> _vertex_buffer_memory{_logical_device, vkFreeMemory};
VDeleter<VkBuffer> _index_buffer{_logical_device, vkDestroyBuffer};
VDeleter<VkDeviceMemory> _index_buffer_memory{_logical_device, vkFreeMemory};
VDeleter<VkBuffer> _uniform_staging_buffer{_logical_device, vkDestroyBuffer};
VDeleter<VkDeviceMemory> _uniform_staging_buffer_memory{_logical_device, vkFreeMemory};
VDeleter<VkBuffer> _uniform_buffer{_logical_device, vkDestroyBuffer};
VDeleter<VkDeviceMemory> _uniform_buffer_memory{_logical_device, vkFreeMemory};
//Command Buffers and Pools

//Descriptor layouts, sets, and pools
VDeleter<VkDescriptorPool> _descriptor_pool{_logical_device, vkDestroyDescriptorPool};
VkDescriptorSet _descriptor_set;
std::vector<VkCommandBuffer> _command_buffers;


//Semaphores for draw method!
VDeleter<VkSemaphore> _image_available_semaphore{_logical_device, vkDestroySemaphore};
VDeleter<VkSemaphore> _rendering_finished_semaphore{_logical_device, vkDestroySemaphore};

//

};

} // Namespace GPU

#endif //TC_RENDERER_VK_GPU_H_H
