//
//  depth_image.cpp
//  vulkan-gui-test
//
//  Created by Rafael Sabino on 2/17/19.
//  Copyright © 2019 Rafael Sabino. All rights reserved.
//

#include "depth_texture.h"

using namespace vk;

void depth_texture::create(uint32_t width, uint32_t height)
{
    if(!_created)
    {
        VkFormat depth_format = _device->find_depth_format( );
        _format = static_cast<vk::image::formats>(depth_format);
        
        _width = width;
        _height = height;
        //TODO: look into using function is_stencil_format
        _aspect_flag = static_cast< image::formats>(VK_FORMAT_D32_SFLOAT) != _format ?  (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT;

        
        VkImageUsageFlagBits usage_flags = _write_to_texture ? static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  | VK_IMAGE_USAGE_SAMPLED_BIT) :
                                static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        
        create_image( depth_format,
                    VK_IMAGE_TILING_OPTIMAL, usage_flags,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
        
        create_image_view( _image, depth_format, _image_view);
        
        _image_layout = image_layouts::UNDEFINED;
        
        _created = true;
        _initialized = true;
    }
}
void depth_texture::create_sampler()
{
    VkSamplerCreateInfo sampler_create_info = {};
    
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.maxAnisotropy = 1.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 1.0f;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    
    VkResult result = vkCreateSampler(_device->_logical_device, &sampler_create_info, nullptr, &_sampler);
    ASSERT_VULKAN(result);
}

void depth_texture::init()
{
    EA_ASSERT(_device != nullptr);
    create_sampler();
    EA_ASSERT_MSG(_width !=0, "spec says this can't be zero");
    create(_width, _height);
    _initialized = true;
}
void depth_texture::set_format(vk::image::formats f)
{
    EA_ASSERT_MSG(f == formats::DEPTH_32_FLOAT || f == formats::DEPTH_32_STENCIL_8 ||
           f == formats::DEPTH_24_STENCIL_8, "invalid depth format" );
    
    set_channels(1);
    _aspect_flag = VK_IMAGE_ASPECT_DEPTH_BIT;
    if(formats::DEPTH_32_FLOAT != f)
     _aspect_flag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
}

void depth_texture::destroy()
{
    if(_created)
    {
        vkDestroyImageView(_device->_logical_device, _image_view, nullptr);
        vkDestroyImage(_device->_logical_device, _image, nullptr);
        vkFreeMemory(_device->_logical_device, _image_memory, nullptr);
        vkDestroySampler(_device->_logical_device, _sampler, nullptr);
        _created = false;
        _image = VK_NULL_HANDLE;
        _image_memory = VK_NULL_HANDLE;
        _image_view = VK_NULL_HANDLE;
        _sampler = VK_NULL_HANDLE;
    }
}

VkAttachmentDescription depth_texture::get_depth_attachment()
{
    VkAttachmentDescription depth_attachment = {};
    
    depth_attachment.flags = 0;
    depth_attachment.format =_device->find_depth_format();
    depth_attachment.samples = _multisampling ? _device->get_max_usable_sample_count(): VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = _write_to_texture ?  VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    return depth_attachment;
}

