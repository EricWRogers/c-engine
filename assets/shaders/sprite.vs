[OPENGL VERSION]

#ifdef GL_ES
    precision mediump float;
#endif

in vec3 vertexPosition;
in vec4 vertexColor;
in vec2 vertexUV;

out vec3 fragmentPosition;
out vec4 fragmentColor;
out vec2 fragmentUV;

uniform mat4 P;
uniform float TIME;

void main()
{
    gl_Position.xyz = (P * vec4(vertexPosition, 1.0)).xyz;
    gl_Position.w = 1.0;
    
    fragmentPosition = vertexPosition;
    fragmentColor = vertexColor;
    fragmentUV = vec2(vertexUV.x, 1 - vertexUV.y);
}

/*
glyphs[0]->topLeft.uv = Vector2(1.0f,0.0f);
            vertices[cv++] = glyphs[0]->topLeft;
            glyphs[0]->bottomLeft.uv = Vector2(0.0f,0.0f);
            vertices[cv++] = glyphs[0]->bottomLeft;
            glyphs[0]->bottomRight.uv = Vector2(0.0f,1.0f);
            vertices[cv++] = glyphs[0]->bottomRight;
            glyphs[0]->topLeft.uv = Vector2(0.0f,1.0f);
            vertices[cv++] = glyphs[0]->topLeft;
            glyphs[0]->bottomRight.uv = Vector2(1.0f,0.0f);
            vertices[cv++] = glyphs[0]->bottomRight;
            glyphs[0]->topRight.uv = Vector2(1.0f,1.0f);
            vertices[cv++] = glyphs[0]->topRight;
*/