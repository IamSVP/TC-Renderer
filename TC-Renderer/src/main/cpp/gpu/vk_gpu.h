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
#include <map>

#define GLM_FORCE_RADIANS
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"

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
uint32_t FindMemoryType(VkPhysicalDevice &physical_device,
                        uint32_t type_filter, VkMemoryPropertyFlags properties);


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



struct Vertex {

glm::vec2 pos;
glm::vec3 color;
glm::vec2 tex_coord;

static VkVertexInputBindingDescription GetBindingDescription() {
  VkVertexInputBindingDescription binding_description = {};
  binding_description.binding = 0;
  binding_description.stride = sizeof(Vertex);
  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return binding_description;
}

static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
  std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions = {};

  attribute_descriptions[0].binding = 0;
  attribute_descriptions[0].location = 0;
  attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_descriptions[0].offset = offsetof(Vertex, pos);

  attribute_descriptions[1].binding = 0;
  attribute_descriptions[1].location = 1;
  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descriptions[1].offset = offsetof(Vertex, color);

  attribute_descriptions[2].binding = 0;
  attribute_descriptions[2].location = 2;
  attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

  return attribute_descriptions;
}

};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};


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
  void CreateDescriptorSetLayout();

  void CreateSemaphores();
  void CreateBuffer(VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VDeleter<VkBuffer> & buffer,
                    VDeleter<VkDeviceMemory>& buffer_memory,
                    bool is_transfer
  );
  void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
  void CreateTextureSampler();

  // Method to add pre-compiled shaders to the code
  void AddShader(std::string &name, std::vector<char> &code);

  // Methods to add geometry
  void SetVertices(std::vector<Vertex> &vertices);
  void SetIndices(std::vector<uint16_t> &indices);
  void SetMVP(UniformBufferObject ubo);



  //////////////////////////////////////////
  //**** Compute related variables  *****//
  ////////////////////////////////////////

  VDeleter<VkCommandPool> _comp_command_pool{_logical_device, vkDestroyCommandPool};
  VkQueue _comp_queue;
  std::vector<VDeleter<VkCommandBuffer> > _comp_command_buffers;
  std::vector<VDeleter<VkFence> > _comp_fences;
  std::vector<VDeleter<VkDescriptorSetLayout> > _comp_descriptor_set_layouts;
  std::vector<VDeleter<VkDescriptorPool> > _comp_descriptor_pools;
  std::vector<VkDescriptorSet> _comp_descriptor_sets;
  std::vector<VDeleter<VkPipelineLayout> > _comp_pipelines;

  ////////////////////////////////////////
  //*** End of compute realted variables//
  //////////////////////////////////////


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


  std::map<std::string, std::vector<char> > _shaders;


  //Geomtery should be simple
  std::vector<Vertex> _vertices;
  std::vector<uint16_t> _indices;
  UniformBufferObject _mvp_ubo;


};

} // Namespace GPU

#endif //TC_RENDERER_VK_GPU_H_H
