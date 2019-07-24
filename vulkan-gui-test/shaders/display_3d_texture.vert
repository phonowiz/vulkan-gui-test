#version 450 core
#extension GL_ARB_separate_shader_objects: enable

//Author:  Rafael Sabino
// Date:    02/28/2018



//uniform mat4 MVP;
//uniform mat4 M;
//layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;
//
//out vec3 fragPosition;
//
//// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
//vec2 scaleAndBias(vec2 p) { return 0.5f * p + vec2(0.5f); }

//
//void main()
//{
//    gl_Position = MVP * vec4(position,1.0f);
//    fragPosition = (M * vec4(position, 1.0f)).xyz;
//}
//

out gl_PerVertex
{
    vec4 gl_Position;
};

//these are vertex attributes in the c++ side.  In the pipeline look for "vertex input" structures
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 inUVCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUVCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragViewVec;
layout(location = 4) out vec3 fragLightVec;

//this is bound using the descriptor set, at binding 0 on the vertex side
layout(binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 lightPosition;
} ubo;


void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(pos, 1.0f);
    vec4 worldPos = ubo.model * vec4(pos, 1.0f);
    
    fragColor = color;
    
    fragUVCoord = inUVCoord;
    fragNormal = mat3(ubo.model) * inNormal;
    fragViewVec = (ubo.view * worldPos).xyz;
    fragLightVec = mat3(ubo.view) * (ubo.lightPosition - vec3(worldPos));
    
}


