//
//  attachment_set.hpp
//  vulkan-demos
//
//  Created by Rafael Sabino on 3/19/20.
//  Copyright © 2020 Rafael Sabino. All rights reserved.
//

#pragma once

#include "image.h"
#include "glfw_swapchain.h"
#include "depth_texture.h"
#include "render_texture.h"
#include "texture_2d.h"
#include "glfw_present_texture.h"
#include "resource_set.h"

#include <iostream>
#include "EASTL/array.h"
#include "EASTL/fixed_string.h"
#include <type_traits>
#include <glm/glm.hpp>

namespace vk
{


    template< uint32_t NUM_ATTACHMENTS>
    class attachment_group : public object
    {
        
    public:
        
        //note: no copying of groups
        attachment_group & operator=(const attachment_group&) = delete;
        attachment_group(const attachment_group&) = delete;
        attachment_group & operator=(const attachment_group) = delete;
        attachment_group & operator=(attachment_group&) = delete;
        attachment_group(attachment_group&) = delete;
        
        static constexpr uint32_t MAX_NUMBER_OF_ATTACHMENTS = 50;
        
        static_assert(MAX_NUMBER_OF_ATTACHMENTS > (NUM_ATTACHMENTS ));
        
        attachment_group()
        {
            eastl::fill(_multisample.begin(), _multisample.end(), false);
        }
        attachment_group(device* device, glm::vec2 dimensions)
        {
            eastl::fill(_multisample.begin(), _multisample.end(), false);
            _device = device; _dimensions = dimensions;
        };
        
        inline void set_multisampling(bool b)
        {
            eastl::fill(_multisample.begin(), _multisample.end(), b);
        }
        
        inline bool is_multisampling()
        {
            for( int i = 0; i < _multisample.size(); ++i)
            {
                if(_multisample[i])
                    return true;
            }
            
            return false;
                   
        }
        inline void set_dimensions(glm::vec2 v)
        {
            _dimensions = v;
        }
        
        inline int32_t get_depth_set_index()
        {
            int32_t result = -1;
            for( int i = 0; i < _attachments.size(); ++i)
            {
                if( _attachments[i].get_instance_type() == resource_set<depth_texture*>::get_class_type())
                {
                    result = i;
                    break;
                }
            }
            return result;
        }
        
        inline int32_t get_attachment_id(const char* attachment_name)
        {
            int32_t result = -1;
            for( int i = 0; i < _attachments.size(); ++i)
            {
                eastl::fixed_string<char, 50> n = {};
                n = attachment_name;
                if( _attachments[i].get_name() == n)
                {
                    result = i;
                    break;
                }
            }
            
            EA_ASSERT_FORMATTED( result != -1, ("attachment %s not found, did you misspell the name?", attachment_name));
            return result;
        }
        
        inline resource_set<image*>* get_depth_set()
        {
            resource_set<image*>* result = nullptr;
                
            int32_t i = get_depth_set_index();
            EA_ASSERT_MSG(i != -1, "depth attachment was not found");
            result = &(_attachments[i]);
                
            return result;
        }
    
        inline void add_attachment(resource_set<depth_texture>& depth_set,
                                   glm::vec2 defaults, bool clear = true, bool store = true)
        {
            EA_ASSERT_MSG(num_attachments < NUM_ATTACHMENTS, "too many attachments have been added");
            _clear_values[num_attachments] = { defaults.x, defaults.y};
            _clear[num_attachments] = clear;
            _store[num_attachments] = store;
            add_attachment(depth_set);
        }

        template<typename T>
        inline void add_attachment(T& texture_type,
                                   glm::vec2 defaults, bool clear = true, bool store = true)
        {
            EA_ASSERT_MSG(num_attachments < NUM_ATTACHMENTS, "too many attachments have been added");
            _clear_values[num_attachments] = { defaults.x, defaults.y};
            _clear[num_attachments] = clear;
            _store[num_attachments] = store;
            add_attachment(texture_type);
        }
        
        
        bool is_multisample_attachment(int32_t i)
        {
            return _multisample[i];
        }
        
        void set_multisample_attachment(int32_t i, bool multisample)
        {
            _multisample[i] = multisample;
        }
        
        //when render pass is done, should we store attachment
        inline bool should_store(uint32_t i)
        {
            return _store[i];
        }
        
        //when we load this attachment, should it clear?
        inline bool should_clear( uint32_t i )
        {
            return _clear[i];
        }
        
        template< typename R>
        inline void add_attachment(resource_set<R>& textures_set, glm::vec4 clear_color, bool clear = true, bool store = true)
        {
            EA_ASSERT_MSG(num_attachments < NUM_ATTACHMENTS, "too many attachments have been added.");
            
            if(textures_set.get_instance_type() == resource_set<texture_cube>::get_class_type())
            {
                for( int i = 0; i < 6; ++i)
                {
                    _clear_values[num_attachments + i] = { clear_color.x, clear_color.y, clear_color.z, clear_color.w};
                    _clear[num_attachments + i] = clear;
                    _store[num_attachments + i] = store;
                }
            }
            else
            {
                _clear_values[num_attachments] = { clear_color.x, clear_color.y, clear_color.z, clear_color.w};
                _clear[num_attachments] = clear;
                _store[num_attachments] = store;
            }
            add_attachment(textures_set);
        }
        
        inline void init()
        {
            EA_ASSERT_MSG(num_attachments ==NUM_ATTACHMENTS, "you haven't populated all attachments");
        }
        
        
        inline void set_device(device* device)
        {
            _device = device;
        }
        
        
        VkClearValue* get_clear_values() { return _clear_values.data(); }
        
        uint32_t size() { return num_attachments; }
        
        inline resource_set<vk::image*>& operator[](int i) { return _attachments[i]; }
        
        virtual void destroy() override
        {
            for( int i = 0; i < _attachments.size(); ++i)
            {
                for( int j = 0; j < _attachments[i].size(); ++i)
                {
                    _attachments[i][j]->destroy();
                }
            }
        }
        
    private:
        
        
        template< typename R>
        inline void add_attachment(R& textures_set)
        {
            EA_ASSERT_MSG(textures_set.get_instance_type() != resource_set<texture_2d>::get_class_type(), "texture 2d's are pre initialized since they are loaded from hard drive, "
                   "did you mean to use a render_texture instead?");

            EA_ASSERT_MSG(num_attachments < NUM_ATTACHMENTS, "there is no more space left for attachments" );

            if(textures_set.get_instance_type() == texture_cube::get_class_type())
            {
                for( int k = 0; k < 6; ++k)
                {
                    eastl::fixed_string<char, 100> name {};
                    EA_ASSERT_MSG(!textures_set.get_name().empty(), "texture cubes need a name for the faces to be referenced as render targets");
                    name.sprintf("%s_%i", textures_set.get_name().c_str(), k);
                    _attachments[num_attachments].set_name( name.c_str());
                    textures_set.set_dimensions(_dimensions.x, _dimensions.y);
                    _attachments[num_attachments] = textures_set;
                    _attachments[num_attachments].set_dimensions(_dimensions.x, _dimensions.y);

                    ++num_attachments;
                    EA_ASSERT_FORMATTED(num_attachments <= NUM_ATTACHMENTS, ("there is no more space left for attachments. We can have up to %i, but %i have been added",NUM_ATTACHMENTS,num_attachments));
                }
            }
            else
            {
                _attachments[num_attachments].set_name( textures_set.get_name().c_str());
                _attachments[num_attachments] = textures_set;
                _attachments[num_attachments].set_dimensions(_dimensions.x, _dimensions.y);

                ++num_attachments;
                EA_ASSERT_MSG(num_attachments <= NUM_ATTACHMENTS, "there is no more space left for attachments" );
            }
        }
        
        template< typename R>
        inline void add_attachment(resource_set<R>& textures_set)
        {
            EA_ASSERT_MSG(textures_set.get_instance_type() != resource_set<texture_2d>::get_class_type(), "texture 2d's are pre initialized since they are loaded from hard drive, "
                   "did you mean to use a render_texture instead?");

            EA_ASSERT_MSG(num_attachments < NUM_ATTACHMENTS, "there is no more space left for attachments" );

            if(textures_set.get_instance_type() == resource_set<texture_cube>::get_class_type())
            {
                for( int k = 0; k < 6; ++k)
                {
                    eastl::fixed_string<char, 100> name {};
                    name.sprintf("%s_%i", textures_set.get_name().c_str(), k);
                    _attachments[num_attachments].set_name( name.c_str());
                    textures_set.set_dimensions(_dimensions.x, _dimensions.y);
                    _attachments[num_attachments] = textures_set;
                    _attachments[num_attachments].set_dimensions(_dimensions.x, _dimensions.y);

                    ++num_attachments;
                }
            }
            else
            {
                _attachments[num_attachments].set_name( textures_set.get_name().c_str());
                _attachments[num_attachments] = textures_set;
                _attachments[num_attachments].set_dimensions(_dimensions.x, _dimensions.y);

                ++num_attachments;
                EA_ASSERT_MSG(num_attachments <= NUM_ATTACHMENTS, "you've added too many attachments" );
            }
        }
        
        eastl::array<VkClearValue,NUM_ATTACHMENTS> _clear_values {};
        
        //note: we add 1 to accomodate for the depth attachment
        device* _device = nullptr;
        
        
        glm::vec2 _dimensions = glm::vec2(0.0f, 0.0f);
        eastl::array<resource_set<vk::image*>, NUM_ATTACHMENTS> _attachments {};
        eastl::array<bool, NUM_ATTACHMENTS> _multisample {};
        eastl::array<bool, NUM_ATTACHMENTS> _clear {};
        eastl::array<bool, NUM_ATTACHMENTS> _store {};
        uint32_t num_attachments = 0;
    };
}
