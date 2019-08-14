#version 450
#extension GL_ARB_separate_shader_objects: enable


layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUVCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1 ) uniform sampler2D tex;

void main()
{
    outColor = texture(tex, fragUVCoord );
    //outColor = vec4(fragColor,1.0f);
}
