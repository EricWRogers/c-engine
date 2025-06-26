#version 330 core
layout (location = 0) in vec3 aPosition;

out vec3 texCoords;

uniform mat4 PROJECTION;
uniform mat4 VIEW;

void main()
{
    texCoords = aPosition;
    vec4 pos = PROJECTION * VIEW * vec4(aPosition, 1.0);
    gl_Position = pos.xyww;
}