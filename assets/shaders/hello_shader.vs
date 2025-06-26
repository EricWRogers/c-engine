#version 330 core

in vec3 aPosition;
in vec3 aNormal;
in vec2 aUV;

out vec2 fragmentUV;
out vec3 fragmentPos;
out vec3 fragmentNormal;

uniform mat4 TRANSFORM;
uniform mat4 VIEW;
uniform mat4 PROJECTION;
uniform float TIME;
uniform bool WIND;
uniform float WINDEFFECT;

void main()
{
    float offset = 0.0;

    if (WIND)
        offset = sin(TIME) * (aPosition.y + 0.5) * WINDEFFECT;

    fragmentPos = vec3(TRANSFORM * vec4(aPosition + vec3(offset, 0.0, offset), 1.0));
    fragmentNormal = aNormal;
    fragmentUV = vec2(aUV.x, -aUV.y);
    gl_Position = PROJECTION * VIEW * vec4(fragmentPos, 1.0);
}