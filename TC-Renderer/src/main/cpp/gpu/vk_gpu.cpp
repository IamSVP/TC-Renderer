//
// Created by Srihari Pratapa on 3/30/17.
//


#include "vk_gpu.h"
#include "vk_guards.h"

#include "vulkan/vulkan.h"
#include "vulkan_wrapper.h"
#include <android/log.h>
#include <android_native_app_glue.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include <set>
#include <vector>
#include <stdexcept>
#include <numeric>
#include <limits>
#include <fstream>
static const char* kTAG = "vk_gpu.cpp";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

//FIXME::put in the proper shaders and stuff

#define VERTEX_SHADER_SPIRV_PATH "shaders/vert."
#define FRAGMENT_SHADER_SPIRV_PATH "shaders/frag."

const int32_t tex_width = 512;
const int32_t tex_height = 512;
//const int32_t tex_channels = 4;


namespace  GPU {


static std::vector<char> ReadFile(const std::string& file_name) {
  std::ifstream file(file_name, std::ios::ate | std::ios::binary);
  if(!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t file_sz = (size_t) file.tellg();
  std::vector<char> buffer(file_sz);
  file.seekg(0);
  file.read(buffer.data(), file_sz);
  file.close();

  return buffer;
}



////////////////////////////////////////////////////////////
//        Physical and Logical Device Creation Functions  //
////////////////////////////////////////////////////////////
struct QueueFamilyIndices {

  int32_t graphics_family = -1;
  int32_t present_family = -1;
  int32_t transfer_family = -1;
  int32_t compute_family = - 1;

  bool IsComplete() {
    return graphics_family >= 0 && present_family >= 0 && compute_family >= 0
     && transfer_family >= 0;
  }

};


QueueFamilyIndices vIndices;

QueueFamilyIndices VKGPUContext::FindQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
  int32_t i = 0;
  for(const auto& queue_family : queue_families) {
    if(queue_family.queueCount > 0 && (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        && (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      indices.graphics_family = i;
    }
    VkBool32 presentation_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentation_support);
    if(queue_family.queueCount > 0 && presentation_support)
      indices.present_family = i;


    if(indices.graphics_family >= 0 && indices.present_family >=0) {
      break;
    }
    i++;
  }


  //Find Transfer Family
  i = 0;
  for(const auto &queue_family : queue_families) {
    if(queue_family.queueCount > 0 &&
       (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
       ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
      indices.transfer_family = i;
    }
    if(indices.transfer_family > 0)
      break;
    i++;
  }

  if(indices.transfer_family < 0) {
   for(const auto &queue_family : queue_families) {
    if(queue_family.queueCount > 0 &&
       (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
      indices.transfer_family = i;
    }
    if(indices.transfer_family > 0)
      break;
    i++;
   }
  }

  //Find Compute Family
  i = 0;
  for(const auto &queue_family : queue_families) {
    if(queue_family.queueCount > 0 &&
       (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
       ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
      indices.compute_family = i;
    }
    if(indices.compute_family > 0)
      break;
    i++;
  }

  if(indices.compute_family < 0) {
   for(const auto &queue_family : queue_families) {
    if(queue_family.queueCount > 0 &&
       (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      indices.compute_family = i;
    }
    if(indices.compute_family > 0)
      break;
    i++;
   }
  }

  return indices;
}


bool VKGPUContext::IsDeviceSuitable(VkPhysicalDevice device) {

  //All the devices are same on android and support everything
  return true;
}

void VKGPUContext::PickPhysicalDevice() {

  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);
  if(device_count == 0) {
    assert(!"Physical device not found!!");
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());
  for(const auto& device : devices) {
    if(IsDeviceSuitable(device)) {
      _physical_device = device;
      break;
    }
  }
  if(_physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

}
void VKGPUContext::CreateLogicalDevice() {
  QueueFamilyIndices indices = FindQueueFamilies(_physical_device);
  //Have a global variable and use the same always
  vIndices = indices;
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<int> unique_queue_families = {indices.graphics_family,
                                         indices.present_family,
                                         indices.transfer_family};
  float queue_priority = 1.0f;
  for (int queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  //Enumerate all device layers and extensions
  _layer_ext.InitDevLayersAndExt(_physical_device);

  //Get device extensions and device extension names
  auto device_ext_count = _layer_ext.DevExtCount();
  auto device_exts = _layer_ext.DevExtNames();
  auto device_layer_count = _layer_ext.DevLayerCount();
  auto device_layers = _layer_ext.DevLayerNames();
  VkPhysicalDeviceFeatures device_features = {};
  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount = device_ext_count;
  create_info.ppEnabledExtensionNames = static_cast<const char *const *>(device_exts);
  if (enable_validation_layers) {
    create_info.enabledLayerCount = device_layer_count;
    create_info.ppEnabledLayerNames = static_cast<const char *const *>(device_layers);
  }
  else {
    create_info.enabledLayerCount = 0;
  }
  CHECK_VK(vkCreateDevice,_physical_device, &create_info, nullptr, _logical_device.replace());
  vkGetDeviceQueue(_logical_device, indices.graphics_family, 0, &_graphics_queue);
  vkGetDeviceQueue(_logical_device, indices.compute_family, 0, &_comp_queue);
  vkGetDeviceQueue(_logical_device, indices.present_family, 0, &_present_queue);
  vkGetDeviceQueue(_logical_device, indices.transfer_family, 0, &_transfer_queue);
}

//////////////////////////////////////////////////////////////
//**** End Physical and Logical Device Creation Functions***//
//////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//             Surface & Swap Chain Creation Functions //
////////////////////////////////////////////////////////

struct SwapChainSupportDetails {
VkSurfaceCapabilitiesKHR capabilities;
std::vector<VkSurfaceFormatKHR> formats;
std::vector<VkPresentModeKHR> present_modes;
};

void VKGPUContext::CreateSurface(android_app *app) {

  VkAndroidSurfaceCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
  create_info.pNext = nullptr;
  create_info.window = app->window;
  create_info.flags = 0;

  CHECK_VK(vkCreateAndroidSurfaceKHR, _instance, &create_info, nullptr, _surface.replace());

}

SwapChainSupportDetails VKGPUContext::QuerySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, nullptr);
  if(format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, details.formats.data());
  }
  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, nullptr);
  if(present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, details.present_modes.data());
  }
  return details;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////   Swap chain properties //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats) {
  if(available_formats.size() == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED) {
    return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  }
  for(const auto &available_format : available_formats) {
    if(available_format.format == VK_FORMAT_B8G8R8A8_UNORM && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }
  return available_formats[0];
}
VkPresentModeKHR ChooseSwapSurfacePresentMode(const std::vector<VkPresentModeKHR> available_present_modes) {
  for(const auto &available_present_mode : available_present_modes) {
    if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
  if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  else{
    VkExtent2D err;
    err.height = -1;
    err.width = -1;
    return err;
  }

}
////////////////////////////////////////////////////////////////////////////////////////////////
////// *************************End of Swap Chain Properties********************************///
//////////////////////////////////////////////////////////////////////////////////////////////


void VKGPUContext::CreateSwapChains(){
  SwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(_physical_device);
  VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(swap_chain_support.formats);
  VkPresentModeKHR present_mode = ChooseSwapSurfacePresentMode(swap_chain_support.present_modes);
  VkExtent2D extent = ChooseSwapExtent(swap_chain_support.capabilities);
  _swap_chain_image_format = surface_format.format;
  _swap_chain_extent = extent;
  uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
  if(swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = _surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  QueueFamilyIndices indices = FindQueueFamilies(_physical_device);
  uint32_t queue_family_indices [] = {static_cast<uint32_t>(indices.graphics_family),
                                      static_cast<uint32_t>(indices.present_family)};
  if(indices.graphics_family != indices.present_family) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
  }
  create_info.preTransform = swap_chain_support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;

  //Whenver called it recreates the a new swap chain
  VkSwapchainKHR old_swap_chain = _swap_chain;
  create_info.oldSwapchain = old_swap_chain;
  VkSwapchainKHR new_swap_chain;

  CHECK_VK(vkCreateSwapchainKHR, _logical_device, &create_info, nullptr, &new_swap_chain);
  _swap_chain = new_swap_chain;


  // Get the swap chain Images
  vkGetSwapchainImagesKHR(_logical_device, _swap_chain, &image_count, nullptr);
  _swap_chain_images.resize(image_count);
  vkGetSwapchainImagesKHR(_logical_device, _swap_chain, &image_count, _swap_chain_images.data());

}

void VKGPUContext::CreateImageView(VkImage image, VkFormat format,
                                               VDeleter<VkImageView> &image_view) {
  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;
  CHECK_VK(vkCreateImageView, _logical_device, &view_info, nullptr, image_view.replace());
  return;
}
void VKGPUContext::CreateImageViews() {

  _swapchain_image_views.resize(_swap_chain_images.size(), VDeleter<VkImageView>{_logical_device, vkDestroyImageView});

  for(uint32_t i = 0; i < _swapchain_image_views.size(); i++) {
    CreateImageView(_swap_chain_images[i], _swap_chain_image_format, _swapchain_image_views[i]);
  }
}

/////////////////////////////////////////////////////////
//********End Swap Chain Creation Functions************//
////////////////////////////////////////////////////////






////////////////////////////////////////////////////////
//////// Render Pass and Graphics Pipeline /////////////
///////////////////////////////////////////////////////

void VKGPUContext::CreateRenderPass() {


  VkAttachmentDescription color_attachment = {};
  color_attachment.format = _swap_chain_image_format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  CHECK_VK(vkCreateRenderPass, _logical_device, &render_pass_info, nullptr, _render_pass.replace());
}

void VKGPUContext::CreateShaderModule(const std::vector<char> &code,
                                                  VDeleter<VkShaderModule>& shader_module){

  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<uint32_t*>(const_cast<char*>(code.data()));
  CHECK_VK(vkCreateShaderModule,_logical_device, &create_info, nullptr, shader_module.replace());
}
void VKGPUContext::CreateGraphicsPipeline() {

  auto vert_shader_code = ReadFile(VERTEX_SHADER_SPIRV_PATH);
  auto frag_shader_code = ReadFile(FRAGMENT_SHADER_SPIRV_PATH);
  VDeleter<VkShaderModule> vert_shader_module{_logical_device, vkDestroyShaderModule};
  VDeleter<VkShaderModule> frag_shader_module{_logical_device, vkDestroyShaderModule};
  CreateShaderModule(vert_shader_code, vert_shader_module);
  CreateShaderModule(frag_shader_code, frag_shader_module);
  VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
  vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shader_module;
  vert_shader_stage_info.pName = "main";
  VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
  frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_module;
  frag_shader_stage_info.pName = "main";
  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};
  // 1.Vertex input stage
  auto binding_description = Vertex::GetBindingDescription();
  auto attribute_description = Vertex::GetAttributeDescriptions();
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions = &binding_description;
  vertex_input_info.vertexAttributeDescriptionCount = attribute_description.size();
  vertex_input_info.pVertexAttributeDescriptions = attribute_description.data();
  // 2. input assembly
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;
  // 3. Viewport and scissors
  VkViewport view_port = {};
  view_port.x = 0.0f;
  view_port.y = 0.0f;
  view_port.width = static_cast<float>(_swap_chain_extent.width);
  view_port.height = static_cast<float>(_swap_chain_extent.height);
  view_port.minDepth = 0.0f;
  view_port.maxDepth = 1.0f;
  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = _swap_chain_extent;
  VkPipelineViewportStateCreateInfo view_port_state = {};
  view_port_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  view_port_state.viewportCount = 1;
  view_port_state.pViewports = &view_port;
  view_port_state.scissorCount = 1;
  view_port_state.pScissors = &scissor;
  // 4. Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthClampEnable = VK_FALSE;
  //rasterizer.depthBiasConstantFactor = 0.0f;
  //rasterizer.depthBiasClamp = 0.0f;
  //rasterizer.depthBiasSlopeFactor = 0.0f;
  // 5. Multisampling
  VkPipelineMultisampleStateCreateInfo multi_sampling = {};
  multi_sampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multi_sampling.sampleShadingEnable = VK_FALSE;
  multi_sampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multi_sampling.minSampleShading = 1.0f;
  multi_sampling.pSampleMask = nullptr;
  multi_sampling.alphaToCoverageEnable = VK_FALSE;
  multi_sampling.alphaToOneEnable = VK_FALSE;
  // 6. Color Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment = {};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                          | VK_COLOR_COMPONENT_G_BIT
                                          | VK_COLOR_COMPONENT_B_BIT
                                          | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blending = {};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f;
  color_blending.blendConstants[1] = 0.0f;
  color_blending.blendConstants[2] = 0.0f;
  color_blending.blendConstants[3] = 0.0f;
  // 7. Dynamic state
  VkDynamicState dynamic_states[] = {
  VK_DYNAMIC_STATE_VIEWPORT,
  VK_DYNAMIC_STATE_LINE_WIDTH
  };
  VkPipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = 2;
  dynamic_state.pDynamicStates = dynamic_states;

  // 8. Pipeline Layout
  VkDescriptorSetLayout set_layouts[] = {_descriptor_set_layout};
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = set_layouts;
  pipeline_layout_info.pushConstantRangeCount = 0;
  pipeline_layout_info.pPushConstantRanges = 0;
  CHECK_VK(vkCreatePipelineLayout, _logical_device, &pipeline_layout_info, nullptr, _pipeline_layout.replace());

  // Final step: setup all together
  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &view_port_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multi_sampling;
  pipeline_info.pDepthStencilState = nullptr;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = nullptr;
  pipeline_info.layout = _pipeline_layout;
  pipeline_info.renderPass = _render_pass;
  pipeline_info.subpass = 0;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;
  CHECK_VK(vkCreateGraphicsPipelines, _logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, _graphics_pipeline.replace());

}



////////////////////////////////////////////////////////
//****** End Render Pass and Graphics Pipeline *******//
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//****** Frame Buffers Creation Functions*************** //
//////////////////////////////////////////////////////////
void VKGPUContext::CreateFramebuffers() {
  _swapchain_framebuffers.resize(_swapchain_image_views.size(),
                                VDeleter<VkFramebuffer>{_logical_device, vkDestroyFramebuffer});
  for(size_t i = 0; i < _swapchain_framebuffers.size(); i++) {

    VkImageView attachments[] = {_swapchain_image_views[i]};
    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = _render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = _swap_chain_extent.width;
    framebuffer_info.height = _swap_chain_extent.height;
    framebuffer_info.layers = 1;
    CHECK_VK(vkCreateFramebuffer,_logical_device, &framebuffer_info, nullptr,
                           _swapchain_framebuffers[i].replace());
  }

}



/////////////////////////////////////////////////////////
//*** End of Frame Buffer Creating Functions ******** //
////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//            Descriptor sets and layouts Functions           //
///////////////////////////////////////////////////////////////
// 1. You create a DescriptorSetLayout to describe the type of buffer, count, stages in which the descriptor set is gonna be used and then create the layout for the descriptor sets
// 2. You cannot create a Descriptor set directly you need Descriptor pool first
// 3. Now Create the Descriptor Set give the pool, set the layouts
// 4. Now Link the buffers(uniform or other kind) using DescriptorBufferInfo
// 5. vkUpdateDescriptorSets is the function through which you need to update the descriptor sets with the buffer data. It takes vkWriteDescriptorSet as input parameter in which everything is tied together

void VKGPUContext::CreateDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding ubo_layout_binding = {};
  //note the binding! is this the same as binding in the shader??
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_layout_binding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutBinding sampler_layout_binding = {};
  sampler_layout_binding.binding = 1;
  sampler_layout_binding.descriptorCount = 1;
  sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.pImmutableSamplers = nullptr;
  sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {{ubo_layout_binding,
                                                           sampler_layout_binding}};
  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = bindings.size();
  layout_info.pBindings = bindings.data();
  CHECK_VK(vkCreateDescriptorSetLayout, _logical_device, &layout_info,
                                 nullptr, _descriptor_set_layout.replace());
}

void VKGPUContext::CreateDescriptorPool() {
  std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_sizes[0].descriptorCount = 1;
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = 1;

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = pool_sizes.size();
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.maxSets = 1;

  CHECK_VK(vkCreateDescriptorPool, _logical_device, &pool_info, nullptr, _descriptor_pool.replace());

}

void VKGPUContext::CreateDescriptorSets() {

  VkDescriptorSetLayout layouts[] = {_descriptor_set_layout};
  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = _descriptor_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = layouts;

  CHECK_VK(vkAllocateDescriptorSets, _logical_device, &alloc_info, &_descriptor_set);

  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = _uniform_buffer;
  buffer_info.offset = 0;
  buffer_info.range = sizeof(UniformBufferObject);

  VkDescriptorImageInfo image_info = {};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView = _texture_image_view;
  image_info.sampler = _texture_sampler;

  std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};
  descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_writes[0].dstSet = _descriptor_set;
  //note this binding too
  descriptor_writes[0].dstBinding = 0;
  descriptor_writes[0].dstArrayElement = 0;
  descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_writes[0].descriptorCount = 1;
  descriptor_writes[0].pBufferInfo = &buffer_info;
  descriptor_writes[0].pImageInfo = nullptr;
  descriptor_writes[0].pTexelBufferView = nullptr;


  descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_writes[1].dstSet = _descriptor_set;
  //note this binding too
  descriptor_writes[1].dstBinding = 1;
  descriptor_writes[1].dstArrayElement = 0;
  descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor_writes[1].descriptorCount = 1;
  descriptor_writes[1].pBufferInfo = nullptr;
  descriptor_writes[1].pImageInfo = &image_info;
  descriptor_writes[1].pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(_logical_device, descriptor_writes.size(),
                         descriptor_writes.data(), 0, nullptr);

}
///////////////////////////////////////////////////////////
// *********End of descriptor set layouts and stuff**** //
/////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////  Command Buffer Functions ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void VKGPUContext::CreateCommandPool() {

  QueueFamilyIndices queue_family_indices = FindQueueFamilies(_physical_device);

  VkCommandPoolCreateInfo command_pool_info = {};
  command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  command_pool_info.queueFamilyIndex = queue_family_indices.graphics_family;
  command_pool_info.flags = 0;

  CHECK_VK(vkCreateCommandPool, _logical_device, &command_pool_info, nullptr, _command_pool.replace());


  //Command pool for transfering
  VkCommandPoolCreateInfo transfer_command_pool_info = {};
  transfer_command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  transfer_command_pool_info.queueFamilyIndex = queue_family_indices.transfer_family;
  transfer_command_pool_info.flags = 0;

  CHECK_VK(vkCreateCommandPool, _logical_device, &transfer_command_pool_info, nullptr,
                         _transfer_command_pool.replace());

}

void VKGPUContext::CreateCommandBuffers() {

  if(_command_buffers.size() > 0)
    vkFreeCommandBuffers(_logical_device, _command_pool, _command_buffers.size(), _command_buffers.data());

  _command_buffers.resize(_swapchain_framebuffers.size());

  VkCommandBufferAllocateInfo command_buffer_alloc_info = {};
  command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_alloc_info.commandPool = _command_pool;
  command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_alloc_info.commandBufferCount = static_cast<uint32_t>(_command_buffers.size());

  CHECK_VK(vkAllocateCommandBuffers, _logical_device, &command_buffer_alloc_info, _command_buffers.data());


  for(size_t i = 0; i < _command_buffers.size(); i++) {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    begin_info.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(_command_buffers[i], &begin_info);

    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = _render_pass;
    render_pass_begin_info.framebuffer = _swapchain_framebuffers[i];
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = _swap_chain_extent;

    VkClearValue color_value;
    color_value.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &color_value;

    //Command Recordings
    vkCmdBeginRenderPass(_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);

    VkBuffer vertex_buffers[] = {_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(_command_buffers[i], 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(_command_buffers[i], _index_buffer, 0, VK_INDEX_TYPE_UINT16);

    //Record the commands to bind the descriptors having uniform buffers
    vkCmdBindDescriptorSets(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _pipeline_layout, 0, 1, &_descriptor_set, 0, nullptr);

    vkCmdDrawIndexed(_command_buffers[i], _indices.size(), 1, 0, 0, 0);

    vkCmdEndRenderPass(_command_buffers[i]);
    CHECK_VK(vkEndCommandBuffer, _command_buffers[i]);

  }

}




/////////////////////////////////////////////////////////////////////////////////////////
// ********************************End of Command Buffer Functions*********************//
///////////////////////////////////////////////////////////////////////////////////////




//Constructor to which the INFO is passed
VKGPUContext::VKGPUContext(VkApplicationInfo info, bool &init){

  if (!InitVulkan()) {
    LOGE("Vulkan is unavailable, install vulkan and re-start");
    init = false;
    return;
  }
  _app_info = info;
}


//////////////////////////////////////
//    DEBUG DEBUG DEGUB /////////////
//     I'm just using the nice class LayerAndExtensions given
//     in Android-vulkan tutorials
//////////////////////////////////////
void VKGPUContext::SetupDebugCallback() {

_layer_ext.AddInstanceExt(_layer_ext.GetDbgExtName());

}

///////////////////////////////////
//******** DEBUG DEBUG DEBUG *****//
//////////////////////////////////

void VKGPUContext::CreateInstance() {


  // Setup Debug callbacks
  SetupDebugCallback();

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &_app_info;

  auto extension_count = _layer_ext.InstExtCount();
  auto extensions = _layer_ext.InstExtNames();

  auto layer_count = _layer_ext.InstLayerCount();
  auto layers = _layer_ext.InstLayerNames();

  create_info.enabledExtensionCount = extension_count;
  create_info.ppEnabledExtensionNames = static_cast<const char* const*>(extensions);
  if(enable_validation_layers) {
    create_info.enabledLayerCount = layer_count;
    create_info.ppEnabledLayerNames = static_cast<const char *const *>(layers);
  }
  else {
     create_info.enabledLayerCount = 0;
  }

  CHECK_VK(vkCreateInstance, &create_info, nullptr, _instance.replace());

}
/////////////////////////////////////////////////////////////////
//                                                            //
//         Helper functions to record command buffers         //
//                                                            //
///////////////////////////////////////////////////////////////

VkCommandBuffer VKGPUContext::BeginSingleTimeCommands() {

  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = _command_pool;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(_logical_device, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

void VKGPUContext::EndSingleTimeCommands(VkCommandBuffer command_buffer,
                                                     VkQueue queue) {

  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);

  vkFreeCommandBuffers(_logical_device, _command_pool, 1, &command_buffer);

}



/////////////////////////////////////////////////////////////////
//                                                            //
//           Texture image creation Functions                 //
//                                                            //
///////////////////////////////////////////////////////////////

void VKGPUContext::CreateTextureImage() {



/*  CreateImage(tex_width, tex_height, VK_FORMAT_R8G8B8A8_UNORM,*/
  //VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
  //VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //staging_image, staging_image_memory);

  //void *data;
  //vkMapMemory(logical_device, staging_image_memory, 0, image_sz, 0, &data);

  //// Cannot direct do a memcpy because the image buffer might have padded some more pixels for
  //// better memory transfer operations

  //VkImageSubresource sub_resource = {};
  //sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  //sub_resource.mipLevel = 0;
  //sub_resource.arrayLayer = 0;


  //VkSubresourceLayout staging_image_layout;
  //vkGetImageSubresourceLayout(logical_device, staging_image, &sub_resource, &staging_image_layout);

  //if(staging_image_layout.rowPitch == tex_width * 4) {
  //memcpy(data, pixels, (size_t)image_sz);
  //}
  //else {
  //uint8_t *data_bytes = reinterpret_cast<uint8_t*>(data);
  //for(int32_t y = 0; y < tex_height; y++) {
  //memcpy(
  //&data_bytes[y * staging_image_layout.rowPitch],
  //&pixels[y * tex_width * 4],
  //tex_width * 4
  //);
  //}
  //}

  /*vkUnmapMemory(logical_device, staging_image_memory);*/

  CreateImage(tex_width, tex_height,
              VK_FORMAT_R8G8B8A8_UNORM,
              VK_IMAGE_TILING_LINEAR,
              VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
              VK_IMAGE_LAYOUT_UNDEFINED,
              _texture_image,
              _texture_image_memory);

}

void VKGPUContext::UpdateTextureImage() {
  // 1. Trasition the image layout to general
  TransitionImageLayout(_texture_image, VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  VkDeviceSize image_sz = tex_width * tex_height * 4;
  vkMapMemory(_logical_device, _texture_image_memory, 0, image_sz, 0, &image_data);

  int32_t width, height, channels;

  tex_img_count = (tex_img_count + 1) % 22 + 1;
  char tex_path[256];
  sprintf(tex_path, "Kodak_Cropped%05d.png", tex_img_count);

  stbi_uc* pixels = stbi_load(tex_path, &width, &height, &channels,
                              STBI_rgb_alpha);

  if(!pixels) {
    printf("%s\n", tex_path);
    throw std::runtime_error("failed to load texture image!");
  }



  VkImageSubresource sub_resource = {};
  sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  sub_resource.mipLevel = 0;
  sub_resource.arrayLayer = 0;


  VkSubresourceLayout texture_image_layout;
  vkGetImageSubresourceLayout(_logical_device, _texture_image, &sub_resource, &texture_image_layout);

  if(texture_image_layout.rowPitch == tex_width * 4) {
    memcpy(image_data, pixels, (size_t)image_sz);
  }
  else {
    uint8_t *data_bytes = reinterpret_cast<uint8_t*>(image_data);
    for(int32_t y = 0; y < tex_height; y++) {
      memcpy(
      &data_bytes[y * texture_image_layout.rowPitch],
      &pixels[y * tex_width * 4],
      tex_width * 4
      );
    }
  }
  vkUnmapMemory(_logical_device, _texture_image_memory);
}


uint32_t FindMemoryType(VkPhysicalDevice &physical_device,
                        uint32_t type_filter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for(uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if(type_filter & (1 << i) &&
       (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory!");
}


void VKGPUContext::CreateImage(uint32_t width, uint32_t height,
                                           VkFormat format,
                                           VkImageTiling tiling,
                                           VkImageUsageFlags usage,
                                           VkMemoryPropertyFlags properties,
                                           VkImageLayout initial_layout,
                                           VDeleter<VkImage> &image,
                                           VDeleter<VkDeviceMemory> &image_memory)
{

  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.format = format;
  image_info.tiling = tiling;
  image_info.initialLayout = initial_layout;
  image_info.usage = usage;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;

  CHECK_VK(vkCreateImage, _logical_device, &image_info, nullptr, image.replace());

  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(_logical_device, image, &mem_requirements);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(_physical_device, mem_requirements.memoryTypeBits,
                                              properties);

  CHECK_VK(vkAllocateMemory, _logical_device, &alloc_info, nullptr, image_memory.replace());

  vkBindImageMemory(_logical_device, image, image_memory, 0);

}


void VKGPUContext::TransitionImageLayout(VkImage image, VkFormat format,
                                                     VkImageLayout old_layout,
                                                     VkImageLayout new_layout) {

  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = 0;

  if(old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
     new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {

    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else if(old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
          new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  }
  else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
          new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
          new_layout == VK_IMAGE_LAYOUT_GENERAL) {

    barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
  }
  else if(old_layout == VK_IMAGE_LAYOUT_GENERAL &&
          new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }


  else {
    throw std::invalid_argument("unsupported layout transition");
  }


  vkCmdPipelineBarrier(
  command_buffer,
  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  0,
  0, nullptr,
  0, nullptr,
  1, &barrier);

  EndSingleTimeCommands(command_buffer, _graphics_queue);
}

void VKGPUContext::CopyImage(VkImage src_image, VkImage dst_image, uint32_t widht, uint32_t height) {

  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkImageSubresourceLayers sub_resource = {};
  sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  sub_resource.baseArrayLayer = 0;
  sub_resource.mipLevel = 0;
  sub_resource.layerCount = 1;

  VkImageCopy region = {};
  region.srcSubresource = sub_resource;
  region.dstSubresource = sub_resource;
  region.srcOffset = {0, 0, 0};
  region.dstOffset = {0, 0, 0};
  region.extent.width = widht;
  region.extent.height = height;
  region.extent.depth = 1;

  vkCmdCopyImage(
  command_buffer,
  src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
  dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  1, &region);


  EndSingleTimeCommands(command_buffer, _graphics_queue);
}




void VKGPUContext::CreateTextureImageView() {
  CreateImageView(_texture_image, VK_FORMAT_R8G8B8A8_UNORM, _texture_image_view);
}

void VKGPUContext::CreateTextureSampler() {

  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  sampler_info.anisotropyEnable = VK_TRUE;
  sampler_info.maxAnisotropy = 16;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = 0.0f;

  CHECK_VK(vkCreateSampler, _logical_device, &sampler_info, nullptr, _texture_sampler.replace());
  return;

}

////////////////////////////////////////////////////////////////
//                                                           //
//********* End of Texture image creating functions**********//
//                                                           //
//////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//     Buffer and device memory creation       //
////////////////////////////////////////////////

void VKGPUContext::CreateBuffer(VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VDeleter<VkBuffer> & buffer,
                    VDeleter<VkDeviceMemory>& buffer_memory,
                    bool is_transfer) {

  uint32_t pIndices[2] = {static_cast<uint32_t>(vIndices.graphics_family),
                          static_cast<uint32_t>(vIndices.transfer_family)};

  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;

  if(is_transfer) {
    buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    buffer_info.queueFamilyIndexCount = 2;
    buffer_info.pQueueFamilyIndices = pIndices;
  }
  else {
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  CHECK_VK(vkCreateBuffer, _logical_device, &buffer_info, nullptr, buffer.replace());

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(_logical_device, buffer, &mem_requirements);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(_physical_device, mem_requirements.memoryTypeBits,
                                              properties);

  CHECK_VK(vkAllocateMemory, _logical_device, &alloc_info, nullptr, buffer_memory.replace());

  vkBindBufferMemory(_logical_device, buffer, buffer_memory, 0);


}

void VKGPUContext::CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {

  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkBufferCopy copy_region = {};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = size;

  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

  EndSingleTimeCommands(command_buffer, _graphics_queue);
  return;

}

/////////////////////////////////////////////////
//    End of Buffer and device memory creation //
////////////////////////////////////////////////





//void HelloWorldCompute::CreateStorageBuffers() {
//
//  VkDeviceSize buffer_size = sizeof(uint32_t) * 2048;
//
//  in_values.resize(2048);
//  for(uint i = 1; i <= 2048; i++) {
//    in_values[i] = i;
//  }
//
//  CreateBuffer(buffer_size, physical_device, logical_device,
//               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
//               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//               ssb_in, ssb_in_memory, false);
//
//  void *data;
//  vkMapMemory(logical_device, ssb_in_memory, 0, buffer_size, 0, &data);
//  memcpy(data, in_values.data(), (size_t)buffer_size);
//  vkUnmapMemory(logical_device, ssb_in_memory);
//
//  CreateBuffer(buffer_size, physical_device, logical_device,
//               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
//               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
//               ssb_out, ssb_out_memory, false);
//}



void VKGPUContext::CreateSemaphores() {

  VkSemaphoreCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  CHECK_VK(vkCreateSemaphore, _logical_device, &create_info, nullptr, _image_available_semaphore.replace());
  CHECK_VK(vkCreateSemaphore, _logical_device, &create_info, nullptr, _rendering_finished_semaphore.replace());

}



/////////////////////////////////////////////////
////       Methods to set geometry            //
///////////////////////////////////////////////
void VKGPUContext::CreateVertexBuffers() {
  VkDeviceSize buffer_size = sizeof(_vertices[0]) * _vertices.size();
  VDeleter<VkBuffer> staging_buffer{_logical_device, vkDestroyBuffer};
  VDeleter<VkDeviceMemory> staging_buffer_memory{_logical_device, vkFreeMemory};
  CreateBuffer(buffer_size,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               staging_buffer, staging_buffer_memory, true);
  void *data;
  vkMapMemory(_logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
  memcpy(data, _vertices.data(), (size_t) buffer_size);
  vkUnmapMemory(_logical_device, staging_buffer_memory);
  CreateBuffer(buffer_size,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertex_buffer, _vertex_buffer_memory, true);
  CopyBuffer(staging_buffer, _vertex_buffer, buffer_size);
  return;
}

void VKGPUContext::CreateIndexBuffers(){

  VkDeviceSize buffer_size = sizeof(_indices[0]) * _indices.size();

  VDeleter<VkBuffer> staging_buffer{_logical_device, vkDestroyBuffer};
  VDeleter<VkDeviceMemory> staging_buffer_memory{_logical_device, vkFreeMemory};

  CreateBuffer(buffer_size,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               staging_buffer, staging_buffer_memory, true);

  void *data;
  vkMapMemory(_logical_device, staging_buffer_memory, 0, buffer_size, 0 , &data);
  memcpy(data, _indices.data(), (size_t)buffer_size);
  vkUnmapMemory(_logical_device, staging_buffer_memory);

  CreateBuffer(buffer_size,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _index_buffer, _index_buffer_memory, true);

  CopyBuffer(staging_buffer, _index_buffer, buffer_size);

  return;


}

void VKGPUContext::CreateUniformbuffers() {

  VkDeviceSize buffer_size = sizeof(UniformBufferObject);

  CreateBuffer(buffer_size,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               _uniform_staging_buffer, _uniform_staging_buffer_memory, true);

  CreateBuffer(buffer_size,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
               _uniform_buffer, _uniform_buffer_memory, true);

}


/////////////////////////////////////////////////
////******End Methods to set geometry*********//
///////////////////////////////////////////////


}
