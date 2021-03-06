//
//  node.hpp
//  vulkan-demos
//
//  Created by Rafael Sabino on 1/18/20.
//  Copyright © 2020 Rafael Sabino. All rights reserved.
//

#pragma once

#include "EASTL/fixed_vector.h"

#include "command_recorder.h"
#include "camera.h"
#include "texture_registry.h"
#include "material_store.h"
#include "EASTL/fixed_string.h"
#include "texture_cube.h"
#include <iostream>

namespace  vk
{
    class command_recorder;
    
    template<uint32_t NUM_CHILDREN>
    class node : public object
    {
    public:
        
        using node_type = vk::node<NUM_CHILDREN>;
        using tex_registry_type = texture_registry<NUM_CHILDREN>;
        using material_store_type = material_store;
        
        node(){}
        
        node(device* device)
        {
            _device = device;
        }
        
        static constexpr size_t MAX_COMMANDS = 20;
        
        inline void set_device(device* device)
        {
            _device = device;
        }
        node & operator=(const node&) = delete;
        node(const node&) = delete;
        node & operator=(node&) = delete;
        node(node&) = delete;
    
        void set_stores(texture_registry<NUM_CHILDREN>& tex_registry , material_store& mat_store)
        {
            _material_store = &mat_store;
            _texture_registry = &tex_registry;
        }
        
        
        inline void set_active(bool b)
        {
            _active = b;
        }

        virtual void init()
        {
            EA_ASSERT( _device != nullptr);
            
            if(!_visited)
            {
                _visited = true;
                //debug_print("initting...");
                
                for( eastl_size_t i = 0; i < _children.size(); ++i)
                {
                    _children[i]->set_stores(*_texture_registry, *_material_store);
                    _children[i]->init();
                }
                
                init_node();
                create_gpu_resources();
            }
        }
        
        
        void destroy_all()
        {
            for( eastl_size_t i = 0; i < _children.size(); ++i)
            {
                _children[i]->destroy_all();
            }
            
            destroy();
        }
        //how to validate?
        //virtual void validate() = 0;
        
        virtual void update(vk::camera& camera, uint32_t image_id)
        {
            if(!_visited)
            {
                _visited = true;
                for( eastl_size_t i = 0; i < node_type::_children.size(); ++i)
                {
                    node_type::_children[i]->update(camera,  image_id);
                }
            }
            
            //debug_print("updating...");
            update_node(camera, image_id);
        }
        
        //update shader parameters here every frame
        virtual void update_node(vk::camera& camera, uint32_t image_id) = 0;
        
        virtual void init_node() = 0;
        virtual bool record_node_commands(command_recorder& buffer, uint32_t image_id) = 0;
        
        virtual VkPipelineStageFlagBits get_producer_stage() = 0;
        virtual VkPipelineStageFlagBits get_consumer_stage() = 0;
        
        
        inline void set_enable(bool b){ _enable = b; }
        
        virtual void add_child( node_type& child )
        {
            child.set_device(_device);
            _children.push_back(&child);
            EA_ASSERT(_children.size() != 0 );
        }
        
        inline bool is_leaf(){
            return _children.empty();
        }
        
        node_type* get_child(uint32_t i)
        {
            EA_ASSERT(i < NUM_CHILDREN);
            return node_type::_children[i];
        }
        
        
        virtual bool record(command_recorder& buffer, uint32_t image_id)
        {
            bool result = true;
            
            if(!_visited)
            {
                _visited = true;
                for( int i = 0; i < _children.size(); ++i)
                {
                    result = result && node_type::_children[i]->record(buffer, image_id);
                }
                
                //TODO: always record transitions.  typically all nodes in a graph would be executed, but in debug mode this may not happen.
                record_transitions(buffer, image_id);
                if( result && _active)
                {
                    //debug_print("recording...");
                    result = record_node_commands(buffer, image_id);
                }
            }
            
            return result;
        }
        
        inline void set_name(const char* name)
        {
            _name = name;
        }
        
        inline const char* const get_name()
        {
            return _name.c_str();
        }
        
        void debug_print(const char* message)
        {
            for( int i = 0; i <= _level; ++i)
            {
                std::cout << "\t";
            }
            std::cout << _name.c_str() << ": " << message << std::endl;
            
        }
        
    protected:
        
        virtual void reset_node( uint32_t i, vk::device* device )
        {
            _visited = false;
            _device = device;
            _level = i;

            for( int i = 0; i < _children.size(); ++i)
            {
                node_type::_children[i]->reset_node(_level + 1, device);
            }
        }
        
        virtual void create_gpu_resources() = 0;
        
        
        VkAccessFlagBits get_dst_access_maks(VkPipelineStageFlags flag)
        {
            VkAccessFlagBits result {};
            if( flag & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
            {
                result = static_cast<VkAccessFlagBits>(result | VK_ACCESS_SHADER_READ_BIT);
            }
            if( flag & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
            {
                result = static_cast<VkAccessFlagBits>(result | VK_ACCESS_SHADER_READ_BIT);
            }
            
            EA_ASSERT_MSG(result != 0, "Unrecognized stage flag to determine src access mask");
            return result;
        }
        
        VkAccessFlagBits get_src_access_mask(VkPipelineStageFlags flag)
        {
            VkAccessFlagBits result {};
            if( flag & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
            {
                result = static_cast<VkAccessFlagBits>(result | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
            }
            if( flag & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
            {
                result = static_cast<VkAccessFlagBits>(result | VK_ACCESS_SHADER_WRITE_BIT);
            }
            
            EA_ASSERT_MSG(result != 0, "Unrecognized stage flag to determine src access mask");
            return result;
        }
        
        const char* layout_string( image::image_layouts layout)
        {
            switch(layout)
            {
                case image::image_layouts::PREINITIALIZED:
                    return "PREINITIALIZED";
                case image::image_layouts::UNDEFINED :
                    return "UNDEFINED";
                case image::image_layouts::GENERAL:
                    return "GENERAL";
                case image::image_layouts::DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                    return "DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
                case image::image_layouts::DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                    return "DEPTH_STENCIL_READ_ONLY_OPTIMAL";
                case image::image_layouts::SHADER_READ_ONLY_OPTIMAL:
                    return "SHADER_READ_ONLY_OPTIMAL";
                case image::image_layouts::TRANSFER_SOURCE_OPTIMAL:
                    return "TRANSFER_SOURCE_OPTIMAL";
                case image::image_layouts::TRANSFER_DESTINATION_OPTIMAL:
                    return "TRANSFER_DESTINATION_OPTIMAL";
                case image::image_layouts::COLOR_ATTACHMENT_OPTIMAL:
                    return "COLOR_ATTACHMENT_OPTIMAL";
                case image::image_layouts::PRESENT_KHR:
                    return "PRESENT_KHR";
                default:
                    EA_FAIL_MSG("unrecognized layout");
            }
            return "";
        }
        
        void create_barrier(command_recorder& buffer, vk::image* p_image, node_type* node,
                            uint32_t image_id, vk::usage_transition transition)
        {
            node_type* dependee_node = node;
            
            EA_ASSERT_MSG( _device->_queue_family_indices.graphics_family.value() ==
                   _device->_queue_family_indices.compute_family.value(), "If this assert fails, we will need to transfer dependent "
                                                                            "resources from compute to graphics queues and vice versa");
            
            {

                //TODO: this data could be received from vk::usage_transition struct by making  producer node and consumer node specify who is the producer stage
                //TODO: and who is the consumer stage
                VkPipelineStageFlagBits producer = dependee_node->get_producer_stage();
                VkPipelineStageFlagBits consumer = this->get_consumer_stage();
                
                VkImageMemoryBarrier barrier {};
                
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.pNext = nullptr;

                barrier.srcAccessMask = get_src_access_mask(producer);
                barrier.dstAccessMask = get_dst_access_maks(consumer);

                barrier.oldLayout = static_cast<VkImageLayout>(transition.previous);
                barrier.newLayout = static_cast<VkImageLayout>(transition.current);
                
                barrier.image = p_image->get_image();
                barrier.subresourceRange = { p_image->get_aspect_flag() , 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
                
                //we are not transferring ownership
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

//                eastl::fixed_string<char, 100> msg {};
//                msg.sprintf("between %s and %s, transitioning %s => %s", this->get_name(), dependee_node->get_name(),
//                            layout_string(transition.previous), layout_string(transition.current));
//                
//                debug_print(msg.c_str());
                
                vkCmdPipelineBarrier(
                                     buffer.get_raw_graphics_command(image_id),
                                     producer,
                                     consumer,
                                     0,
                                     0, nullptr,
                                     0, nullptr,
                                     1, &barrier);
            }
        }
        
        void record_transitions(command_recorder& buffer,  uint32_t image_id)
        {
            EA_ASSERT_MSG( _device->_queue_family_indices.graphics_family.value() ==
                   _device->_queue_family_indices.compute_family.value(), "If this assert fails, we will need to transfer dependent "
                                                                            "resources from compute to graphics queues and vice versa");
            
            using tex_registry_type = texture_registry<NUM_CHILDREN>;

            //note: here we are only grabbing those image resources this node depends on
            //in the graph, there may be more resources that are written to below this node, it doesn't mean this node depends on them
            typename tex_registry_type::node_dependees& dependees = _texture_registry->get_dependees(this);

            typename tex_registry_type::node_dependees::iterator begin = dependees.begin();
            typename tex_registry_type::node_dependees::iterator end = dependees.end();

            //TODO: In theory we could collect all the barriers and have one vkCmdPipelineBarrier
            for(typename tex_registry_type::node_dependees::iterator b = begin ; b != end ; ++b)
            {
                
                eastl::shared_ptr<vk::object> res = eastl::static_pointer_cast<vk::object>((*b).data.resource);
                
                if(res->get_instance_type() == texture_2d::get_class_type() ||
                   res->get_instance_type() == texture_3d::get_class_type() ||
                   res->get_instance_type() == texture_cube::get_class_type() ||
                   res->get_instance_type() == render_texture::get_class_type())
                {
                    eastl::shared_ptr<vk::image> p_image = eastl::static_pointer_cast<vk::image>(res);
                    vk::usage_transition trans {};
                    trans.previous = p_image->get_original_layout();
                    trans.current = (*b).layout;
                    create_barrier(buffer, p_image.get(),  (*b).data.node, image_id,trans);
                }
                else
                {
                    //this is a resource set...
                    //TODO: maybe we can use templates here...
                    if(res->get_instance_type()  == resource_set<vk::texture_2d>::get_class_type())
                    {
                        eastl::shared_ptr< resource_set<vk::texture_2d> > set = eastl::static_pointer_cast< resource_set<vk::texture_2d>>(res);
                        
//                        eastl::fixed_string<char, 100> msg {};
//                        msg.sprintf("transitioning texture %s", set->get_name().c_str());
//                        debug_print(msg.c_str());
                        
                        vk::texture_2d* tex = &((*set)[image_id]);
                        create_barrier(buffer, tex,  (*b).data.node, image_id, (*set).get_current_transition());
                        (*set).pop_transition();
                    }
                    else if(res->get_instance_type()  == resource_set<vk::texture_3d>::get_class_type())
                    {
                        eastl::shared_ptr< resource_set<vk::texture_3d> > set = eastl::static_pointer_cast< resource_set<vk::texture_3d>>(res);
                        
//                        eastl::fixed_string<char, 100> msg {};
//                        msg.sprintf("transitioning texture %s", set->get_name().c_str());
//                        debug_print(msg.c_str());
                        
                        vk::image* tex = &((*set)[image_id]);
                        create_barrier(buffer, tex,  (*b).data.node, image_id, (*set).get_current_transition());
                        (*set).pop_transition();
                    }
                    else if(res->get_instance_type()  == resource_set<vk::depth_texture>::get_class_type())
                    {
                        eastl::shared_ptr< resource_set<vk::depth_texture> > set = eastl::static_pointer_cast< resource_set<vk::depth_texture>>(res);
                        
//                        eastl::fixed_string<char, 100> msg {};
//                        msg.sprintf("transitioning texture %s", set->get_name().c_str());
//                        debug_print(msg.c_str());
                        
                        vk::image* tex = &((*set)[image_id]);
                        create_barrier(buffer, tex,  (*b).data.node, image_id, (*set).get_current_transition());
                        (*set).pop_transition();
                    }
                    else if(res->get_instance_type()  == resource_set<vk::render_texture>::get_class_type())
                    {
                        eastl::shared_ptr< resource_set<vk::render_texture> > set = eastl::static_pointer_cast< resource_set<vk::render_texture>>(res);
                        
//                        eastl::fixed_string<char, 100> msg {};
//                        msg.sprintf("transitioning texture %s", set->get_name().c_str());
//                        debug_print(msg.c_str());
                        
                        vk::image* tex = &((*set)[image_id]);
                        create_barrier(buffer, tex,  (*b).data.node, image_id, (*set).get_current_transition());
                        (*set).pop_transition();
                    }
                    else if(res->get_instance_type()  == resource_set<vk::texture_cube>::get_class_type())
                    {
                        eastl::shared_ptr< resource_set<vk::texture_cube> > set = eastl::static_pointer_cast< resource_set<vk::texture_cube>>(res);
                        
//                        eastl::fixed_string<char, 100> msg {};
//                        msg.sprintf("transitioning texture %s", set->get_name().c_str());
//                        debug_print(msg.c_str());
                        
                        vk::image* tex = &((*set)[image_id]);
                        create_barrier(buffer, tex,  (*b).data.node, image_id, (*set).get_current_transition());
                        (*set).pop_transition();
                    }
                    else
                    {
                        EA_FAIL_MSG("unrecognized resource_set");
                    }
                }
            }
        }
        
        
    protected:
        device* _device = nullptr;
        eastl::fixed_vector<node_type*, NUM_CHILDREN, true> _children{};
        
        material_store_type*     _material_store = nullptr;
        tex_registry_type*  _texture_registry = nullptr;
        
        eastl::fixed_string<char, 100> _name = "default";
        
        bool _active = true;
        bool _visited = false;
        bool _enable = true;
        
        uint32_t _level = 0;
        
    };
}
