//
//  material.cpp
//  vulkan-gui-test
//
//  Created by Rafael Sabino on 2/10/19.
//  Copyright © 2019 Rafael Sabino. All rights reserved.
//

#include "visual_material.h"
#include "shader_parameter.h"
#include <unordered_map>
#include <array>
#include <fstream>
#include <iostream>

using namespace vk;

visual_material::visual_material( const char* name, shader_shared_ptr vertex_shader, shader_shared_ptr fragment_shader, device* device):
material_base(device, name)
{
    _vertex_shader = vertex_shader;
    _fragment_shader = fragment_shader;
}

shader_parameter::shader_params_group& visual_material::get_uniform_parameters(parameter_stage stage, uint32_t binding)
{
    assert( stage != parameter_stage::COMPUTE && "compute stage is not usable from visual material class, choose compute material instead" );
    
    buffer_info& mem = _uniform_buffers[stage];
    mem.binding = binding;
    mem.usage_type = usage_type::UNIFORM_BUFFER;
    
    return _uniform_parameters[stage];
    
}

void visual_material::destroy()
{
    vkDestroyDescriptorSetLayout(_device->_logical_device, _descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(_device->_logical_device, _descriptor_pool, nullptr);
    _descriptor_set_layout = VK_NULL_HANDLE;
    _descriptor_pool = VK_NULL_HANDLE;
    deallocate_parameters();
    
    material_base::destroy();
}
visual_material::~visual_material()
{
}