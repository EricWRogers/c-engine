#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube SKYBOX;

void main()
{
    FragColor = texture(SKYBOX, texCoords);
}