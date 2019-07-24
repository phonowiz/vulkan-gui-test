#version 450 core
#extension GL_ARB_separate_shader_objects: enable

//Author:  Rafael Sabino
// Date:    02/28/2018


layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 frag_world_position;
layout(location = 1) out vec3 eye_world_position;

//this is bound using the descriptor set, at binding 0 on the vertex side
layout(binding = 0) uniform UBO
{
    mat4 mvp;
    mat4 model;
} ubo;


void main()
{
    gl_Position = ubo.mvp * vec4(pos,1.0f);
    
    frag_world_position = (ubo.model * vec4(pos, 1.0f)).xyz;
}




