[OPENGL VERSION]

in vec3 fragmentNormal;
in vec2 fragmentUV;

uniform sampler2D mySampler;
uniform bool useTexture;
uniform vec4 baseColor;

out vec4 color;

void main()
{
    vec3 n = normalize(fragmentNormal);
    vec3 lightDir = normalize(vec3(-0.4, 1.0, 0.25));
    float diffuse = max(dot(n, lightDir), 0.0);
    float lighting = 0.25 + diffuse * 0.75;

    vec4 tex = useTexture ? texture(mySampler, fragmentUV) : vec4(1.0);
    color = vec4(baseColor.rgb * tex.rgb * lighting, baseColor.a * tex.a);
}
