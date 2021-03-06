//
//  ShaderParameter.hpp
//  vulkan-gui-test
//
//  Created by Rafael Sabino on 2/14/19.
//  Copyright © 2019 Rafael Sabino. All rights reserved.
//

#pragma once


#include "glm/gtc/type_ptr.hpp"
#include "glm/glm.hpp"
#include "texture_2d.h"
#include "texture_3d.h"
#include "texture_2d_array.h"
#include "texture_cube.h"
#include "glfw_present_texture.h"
#include "render_texture.h"
#include "depth_texture.h"
#include "EASTL/fixed_string.h"
#include "EASTL/array.h"
#include <map>
#include "ordered_map.h"

/// <summary> Represents a setting for a material that can be used for a shader </summary>

namespace vk
{
    using string_key_type = eastl::fixed_string<char, 30>;

    class shader_parameter
    {
        
    private:
        static constexpr size_t MAX_UNIFORM_BUFFER_SIZE = 512;
        struct values_array
        {
            char* memory[MAX_UNIFORM_BUFFER_SIZE] {};
            size_t num_elements = 0;
        };
        
    public:
        enum class Type
        {
            MAT4,
            VEC4,
            VEC3,
            VEC2,
            FLOAT,
            INT,
            BOOLEAN,
            UINT,
            SAMPLER_2D,
            SAMPLER_3D,
            SAMPLER_2D_ARRAY,
            SAMPLER_CUBE,
            SAMPLER_PRESENT_TEXTURE,
            VEC4_ARRAY,
            NONE
            
        };
        
    private:
        
        union setting_value
        {
            glm::vec4       vector4;
            glm::vec3       vector3;
            glm::vec2       vector2;
            float           float_value;
            int32_t         intValue;
            uint32_t        uintValue;
            texture_2d*     sampler2D;
            texture_3d*     sampler3D;
            texture_2d_array*       sampler_2d_array;
            glfw_present_texture*   sampler_present_tex;
            texture_cube*           sampler_cube;
            
            glm::mat4 mat4;
            bool     boolean;
            
            values_array buffer;
            
            setting_value()
            {
                buffer = {};
            }
            
        };
        
        setting_value value;
        Type type;
        const char* name = nullptr;
        
    public:
        
        inline Type get_type(){return type;}
        inline setting_value* get_stored_value_memory(){ return &value; }
        
        using shader_params_group = ordered_map< string_key_type ,shader_parameter>;
        using KeyValue = eastl::pair<string_key_type, shader_parameter> ;
        
        shader_parameter():value(),type(Type::NONE)
        {}
        
        //the size returned here should be big enough ( safe enough) to store whatever bytes we pass it.
        static size_t aligned_size(size_t alignment, size_t bytes)
        {
            EA_ASSERT_MSG((alignment > 0) && (alignment & ~(alignment -1)), "alignment must be greater than 0 && power of 2");
            return (alignment -1 ) + bytes;
        }
        
        inline size_t get_type_size()
        {
            switch( type )
            {
                case Type::INT:
                    return sizeof(int32_t);
                case Type::FLOAT:
                    return sizeof(float);
                case Type::MAT4:
                    return sizeof(glm::mat4);
                case Type::VEC3:
                    return sizeof(glm::vec3);
                case Type::VEC4:
                    return sizeof(glm::vec4);
                case Type::VEC2:
                    return sizeof(glm::vec2);
                case Type::UINT:
                    return sizeof(uint32_t);
                case Type::BOOLEAN:
                    return sizeof(bool);
                case Type::VEC4_ARRAY:
                    return value.buffer.num_elements * sizeof(glm::vec4);
                default:
                    EA_FAIL();
                    break;
                    
            }
            return 0;
        }
        
        static size_t get_std140_alignment( Type data_type)
        {
            switch( data_type )
            {
                case Type::INT:
                    return sizeof(int32_t);
                case Type::FLOAT:
                    return sizeof(float);
                case Type::MAT4:
                case Type::VEC3:
                case Type::VEC4:
                    return 4 * sizeof(float);
                case Type::VEC2:
                    return 2 * sizeof(float);
                case Type::UINT:
                    return sizeof(uint32_t);
                case Type::BOOLEAN:
                    return sizeof(bool);
                case Type::VEC4_ARRAY:
                    return 4 * sizeof(float);
                default:
                    EA_FAIL();
                    break;
                    
            }
            return 0;
        }
        //note: this function follows std140 alignment rules, in your glsl shader, make sure to specify std140 as your choice
        //for memory layout.
        inline size_t get_std140_alignment()
        {
            return get_std140_alignment(type);
        }

        static size_t get_std140_aligned_size_in_bytes( Type data_type)
        {
            size_t result = 0;
            switch( data_type )
            {
                case Type::INT:
                    result = aligned_size(get_std140_alignment(data_type), sizeof(int32_t));
                    break;
                case Type::FLOAT:
                    result =  aligned_size(get_std140_alignment(data_type), sizeof(float));
                    break;
                case Type::BOOLEAN:
                    result =  aligned_size(get_std140_alignment(data_type), sizeof( bool));
                    break;
                case Type::UINT:
                    result = aligned_size(get_std140_alignment(data_type), sizeof(uint32_t));
                    break;
                case Type::MAT4:
                    result = aligned_size(get_std140_alignment(data_type), sizeof(glm::mat4));
                    break;
                case Type::VEC2:
                    result = aligned_size(get_std140_alignment(data_type), sizeof( glm::vec2));
                    break;
                case Type::VEC3:
                    result = aligned_size(get_std140_alignment(data_type), sizeof( glm::vec3));
                    break;
                case Type::VEC4:
                    result =  aligned_size(get_std140_alignment(data_type), sizeof(glm::vec4));
                    break;
                default:
                {
                    EA_FAIL_MSG("this case should never happen");
                    result = 0;
                }
            };
            
            return result;
        }
        inline size_t get_max_std140_aligned_size_in_bytes()
        {
            size_t result;
            switch(type)
            {
                case Type::VEC4_ARRAY:
                {
                    size_t vec4_size = aligned_size(get_std140_alignment(), sizeof(glm::vec4));
                    result = value.buffer.num_elements * vec4_size;
                    break;
                }
                default:
                {
                    result = get_std140_aligned_size_in_bytes( type );
                }
            }
            
            return result;
        }
        
        void* write_to_buffer(void* p, size_t& mem_size)
        {
            char* ptr = nullptr;
            if(type == Type::VEC4_ARRAY)
            {
                glm::vec4* vecs = reinterpret_cast<glm::vec4*>(value.buffer.memory);
                for(size_t i = 0; i < value.buffer.num_elements; ++i)
                {
                    void* result = std::align( get_std140_alignment(), sizeof(glm::vec4), p, mem_size);
                    EA_ASSERT(result);
                    std::memcpy(p, &vecs[i], sizeof(glm::vec4));
                    mem_size -= sizeof(glm::vec4);
                    ptr = static_cast<char*>(p);
                    ptr+= sizeof(glm::vec4);
                    p = reinterpret_cast<void*>(ptr);
                }
            }
            else
            {
                EA_ASSERT(p != nullptr);
                void* result = std::align( get_std140_alignment(),get_type_size(), p, mem_size);
                EA_ASSERT(result);
                EA_ASSERT(mem_size >= get_type_size());
                mem_size -= get_type_size();
                std::memcpy(p, get_stored_value_memory(), get_type_size());
                ptr = static_cast<char*>(p);
                ptr+= get_type_size();
            }

            
            return reinterpret_cast<void*>(ptr);
        }
        
        inline glfw_present_texture* get_present_texture()
        {
            EA_ASSERT(type == Type::SAMPLER_PRESENT_TEXTURE);
            return value.sampler_present_tex;
        }
        inline texture_2d* get_texture_2d()
        {
            EA_ASSERT(type == Type::SAMPLER_2D);
            return value.sampler2D;
        }
        
        inline texture_3d* get_texture_3d()
        {
            EA_ASSERT(type == Type::SAMPLER_3D);
            return value.sampler3D;
        }
        inline texture_2d_array* get_texture_2d_array()
        {
            EA_ASSERT(type == Type::SAMPLER_2D_ARRAY);
            return value.sampler_2d_array;
        }
        
        inline texture_cube* get_texture_cube()
        {
            EA_ASSERT(type == Type::SAMPLER_CUBE);
            return value.sampler_cube;
        }
        
        inline image* get_image()
        {
            EA_ASSERT(type == Type::SAMPLER_2D || type == Type::SAMPLER_3D || type == Type::SAMPLER_2D_ARRAY
                   || type == Type::SAMPLER_PRESENT_TEXTURE || type == Type::SAMPLER_CUBE);
            
            if(type == Type::SAMPLER_2D)
            {
                return static_cast<image*>(value.sampler2D);
            }
            
            if(type == Type::SAMPLER_2D_ARRAY)
            {
                return static_cast<image*>(value.sampler_2d_array);
            }
            if(type == Type::SAMPLER_PRESENT_TEXTURE)
            {
                return static_cast<image*>(value.sampler_present_tex);
            }
            if(type == Type::SAMPLER_CUBE)
            {
                return static_cast<image*>(value.sampler_cube);
            }
            return static_cast<image*>(value.sampler3D);
        }
        
        inline void set_vectors_array(const glm::vec4* vecs, size_t num_vectors)
        {
            EA_ASSERT( type == Type::NONE || type == Type::VEC4_ARRAY);
            type = Type::VEC4_ARRAY;
            value.buffer.num_elements = num_vectors;
            void* data = reinterpret_cast<void*>(value.buffer.memory);
            EA_ASSERT((value.buffer.num_elements * sizeof(glm::vec4)) < MAX_UNIFORM_BUFFER_SIZE);
            std::memcpy(data, &vecs[0], num_vectors * sizeof(glm::vec4));
        }
        
        template<int MAX_SIZE>
        inline shader_parameter& operator=(const eastl::array<int32_t, MAX_SIZE>& arr)
        {
            //note: as  you can see here int arrays are actually vec4 arrays due to the layout we've chosen for parameters to shaders (std140).
            //If you can avoid int arrays as arguments to shaders, due so.  There is lots of memory that doesn't get used
            EA_ASSERT( type == Type::NONE || type == Type::VEC4_ARRAY);
            type = Type::VEC4_ARRAY;
            value.buffer.num_elements = arr.size();
            void* data = reinterpret_cast<void*>(value.buffer.memory);
            EA_ASSERT((value.buffer.num_elements * sizeof(glm::vec4)) < MAX_UNIFORM_BUFFER_SIZE);
            
            char* ptr = reinterpret_cast<char*>(data);
            for(int i = 0; i < MAX_SIZE; ++i)
            {
                ptr += sizeof(glm::vec4);
                std::memcpy(ptr, &arr[i], sizeof(int32_t));
            }
            
            return *this;
        }
        
        template<int MAX_SIZE>
        inline shader_parameter& operator=(const eastl::array<glm::vec4, MAX_SIZE>& arr)
        {
            set_vectors_array(static_cast<const glm::vec4*>(arr.data()), arr.size());
            return *this;
        }

        inline shader_parameter& operator=(const glm::mat4 &value)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::MAT4, "shader argument type mismatch");
            type = Type::MAT4;
            this->value.mat4 = value;

            return *this;
        }
        
        inline shader_parameter& operator=(const float &value)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::FLOAT, "shader argument type mismatch");
            type = Type::FLOAT;
            this->value.float_value = value;
            
            return *this;
        }
        
        inline shader_parameter& operator=(const glm::vec4 &value)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::VEC4, "shader argument type mismatch");
            type = Type::VEC4;
            this->value.vector4 = value;
            
            return *this;
        }
        
        inline shader_parameter& operator=(const glm::vec3 &value)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::VEC3, "shader argument type mismatch");
            type = Type::VEC3;
            this->value.vector3 = value;
            
            return *this;
        }
        
        inline shader_parameter& operator=(const glm::vec2 &value)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::VEC2, "shader argument type mismatch");
            type = Type::VEC2;
            this->value.vector2 = value;
            
            return *this;
        }
        
        inline shader_parameter& operator=(const int32_t &value)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::INT, "shader argument type mismatch");
            type = Type::INT;
            this->value.intValue = value;
            
            return *this;
        }
        
        inline shader_parameter& operator=(const uint32_t &value)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::UINT, "shader argument type mismatch");
            type = Type::UINT;
            this->value.intValue = value;
            
            return *this;
        }
        
        inline shader_parameter& operator=(texture_cube* sampler)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::SAMPLER_CUBE, "shader argument type mismatch");
            type = Type::SAMPLER_CUBE;
            this->value.sampler_cube = sampler;
            
            return *this;
        }
        
        inline shader_parameter& operator=(texture_2d_array* sampler)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::SAMPLER_2D_ARRAY, "shader argument type mismatch");
            type = Type::SAMPLER_2D_ARRAY;
            this->value.sampler_2d_array = sampler;
            
            return *this;
        }
        
        inline shader_parameter& operator=(texture_2d* sampler)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::SAMPLER_2D, "shader argument type mismatch");
            type = Type::SAMPLER_2D;
            this->value.sampler2D = sampler;
    
            return *this;
        }
        
        inline shader_parameter& operator=(texture_3d* sampler)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::SAMPLER_3D, "shader argument type mismatch");
            type = Type::SAMPLER_3D;
            this->value.sampler3D = sampler;
            
            return *this;
        }
        
        inline shader_parameter& operator=(glfw_present_texture* sampler)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::SAMPLER_PRESENT_TEXTURE, "shader argument type mismatch");
            type = Type::SAMPLER_PRESENT_TEXTURE;
            this->value.sampler_present_tex = sampler;
            
            return *this;
        }
        
        
        inline shader_parameter& operator=(image* sampler)
        {
            EA_ASSERT_MSG( type == Type::NONE || type == Type::SAMPLER_3D || type == Type::SAMPLER_2D || type == Type::SAMPLER_PRESENT_TEXTURE, "shader argument type mismatch");
            if(sampler->get_instance_type() == texture_3d::get_class_type())
            {
                type = Type::SAMPLER_3D;
                value.sampler3D = static_cast<texture_3d*>( sampler );
            }
            else if( sampler->get_instance_type() == texture_2d::get_class_type() ||
                    sampler->get_instance_type() == glfw_present_texture::get_class_type() ||
                    sampler->get_instance_type() == render_texture::get_class_type() ||
                    sampler->get_instance_type() == depth_texture::get_class_type())
            {
                type = Type::SAMPLER_2D;
                value.sampler2D = static_cast<texture_2d*>( sampler );
            }
            else if( sampler->get_instance_type() == texture_cube::get_class_type())
            {
                type = Type::SAMPLER_CUBE;
                value.sampler_cube = static_cast<texture_cube*>(sampler);
            }
            else
            {
                EA_FAIL_MSG("image type not recognized");
            }
            
            return *this;
        }
        
        inline shader_parameter& operator=(const bool value)
        {
            type = Type::BOOLEAN;
            this->value.boolean = value;
            return *this;
        }
        
        shader_parameter(glm::mat4 _value)
        {
            value.mat4 = _value;
            type = Type::MAT4;
        }
        
        shader_parameter(glm::vec4 _value)
        {
            value.vector4 = _value;
            type = Type::VEC4;
        }
        
        shader_parameter(glm::vec3 _value)
        {
            value.vector3 = _value;
            type = Type::VEC3;
        }
        
        shader_parameter(glm::vec2 _value)
        {
            value.vector2 = _value;
            type = Type::VEC2;
        }
        
        shader_parameter(int32_t _value)
        {
            value.intValue = _value;
            type = Type::INT;
        }
        shader_parameter(uint32_t _value)
        {
            value.uintValue = _value;
            type = Type::UINT;
        }
        
        shader_parameter(float _value)
        {
            value.float_value= _value;
            type = Type::FLOAT;
        }
        
        shader_parameter(texture_2d* _value)
        {
            value.sampler2D = _value;
            type = Type::SAMPLER_2D;
        }
    
        shader_parameter(texture_3d* _value)
        {
            value.sampler3D = _value;
            type = Type::SAMPLER_3D;
        }
        
        shader_parameter(texture_2d_array* _value)
        {
            value.sampler_2d_array = _value;
            type = Type::SAMPLER_2D_ARRAY;
        }

        shader_parameter(texture_cube* _value)
        {
            value.sampler_cube = _value;
            type = Type::SAMPLER_CUBE;
        }
        
        shader_parameter(glfw_present_texture* _value)
        {
            value.sampler_present_tex = _value;
            type = Type::SAMPLER_PRESENT_TEXTURE;
        }
        
    private:
        
    };

}
