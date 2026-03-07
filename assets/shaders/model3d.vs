[OPENGL VERSION]

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexUV;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 fragmentNormal;
out vec2 fragmentUV;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(M)));
    fragmentNormal = normalize(normalMatrix * vertexNormal);
    fragmentUV = vec2(vertexUV.x, vertexUV.y);

    gl_Position = P * V * M * vec4(vertexPosition, 1.0);
}
