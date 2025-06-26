#version 330 core
out int FragColor;
uniform int entityID;

void main() {
    FragColor = entityID;
}
