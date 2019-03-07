//
//  texture.hpp
//  vulkan-gui-test
//
//  Created by Rafael Sabino on 2/10/19.
//  Copyright © 2019 Rafael Sabino. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "resource.h"
#include "image.h"

#include "stb_image.h"
namespace vk
{
    class Texture : public Image
    {
    public:
    
        Texture(){};
        Texture(PhysicalDevice* device, uint32_t width, uint32_t height);
        Texture(PhysicalDevice* device,const char* path = nullptr);
        
        enum class Formats
        {
            RGBA = VK_FORMAT_R8G8B8A8_UNORM
        };

        virtual void create( uint32_t width, uint32_t height) ;
        virtual void destroy() override;
        
        stbi_uc * getRaw()
        {
            return _ppixels;
        }
        
        VkSampler getSampler()
        {
            return _sampler;
        }
        
        int getChannels()
        {
            assert(_channels == 4);
            return _channels;
        }
        
        int getSizeInBytes()
        {
            return getWidth() * getHeight() * getChannels();
        }
        
        void load( );
        
    private:
        
        bool _loaded = false;
        bool _uploaded = false;
        
        stbi_uc *_ppixels = nullptr;
        VkSampler _sampler = VK_NULL_HANDLE;
        const char* _path = nullptr;
        
        //TODO: we only support 4 channels at the moment
        uint32_t _channels = 4;
        //TODO: we only support RGBA
        Texture::Formats _format = Texture::Formats::RGBA;
    };
}

