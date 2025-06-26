#version 330 core
//The fragment shader operates on each pixel in a given polygon
out vec4 FragColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

in vec2 fragmentUV;
in vec3 fragmentPos;
in vec3 fragmentNormal;

uniform vec3 COLOR;
uniform Material MATERIAL;
uniform DirectionalLight DIRECTIONALLIGHT;
uniform PointLight POINTLIGHTS[4];
uniform int NUMBEROFPOINTLIGHTS;
uniform float TIME;

uniform vec3 VIEWPOS;

vec3 CalculateDirectionalLight(DirectionalLight _directionalLight);
vec3 CalculatePointLight(PointLight _pointLight);

void main() {
	// base color
	vec4 color = texture(MATERIAL.diffuse, fragmentUV) * vec4(COLOR, 1.0);

    if (color.a <= 0.0)
    {
        discard;
    }

	vec3 result = CalculateDirectionalLight(DIRECTIONALLIGHT);

	for(int i = 0; i < NUMBEROFPOINTLIGHTS; i++)
		result += CalculatePointLight(POINTLIGHTS[i]);

	FragColor = color * vec4(result, 1.0);
}

vec3 CalculateDirectionalLight(DirectionalLight _directionalLight)
{
    // ambient
    vec3 ambient = _directionalLight.ambient;
  	
    // diffuse 
    vec3 norm = normalize(fragmentNormal);
    vec3 lightDir = normalize(-_directionalLight.direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = _directionalLight.diffuse * diff;  
    
    // specular
    vec3 viewDir = normalize(VIEWPOS - fragmentPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), MATERIAL.shininess);
    vec3 specular = _directionalLight.specular * spec * texture(MATERIAL.specular, fragmentUV).rgb;  
        
    return ambient + diffuse + specular;
}

vec3 CalculatePointLight(PointLight _pointLight)
{
	// ambient
    vec3 ambient = _pointLight.ambient;
  	
    // diffuse 
    vec3 norm = normalize(fragmentNormal);
    vec3 lightDir = normalize(_pointLight.position - fragmentPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = _pointLight.diffuse * diff;  
    
    // specular
    vec3 viewDir = normalize(VIEWPOS - fragmentPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), MATERIAL.shininess);
    vec3 specular = _pointLight.specular * spec * texture(MATERIAL.specular, fragmentUV).rgb;  
    
    // attenuation
    float distance    = length(_pointLight.position - fragmentPos);
    float attenuation = 1.0 / (_pointLight.constant + _pointLight.linear * distance + _pointLight.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    return ambient + diffuse + specular;
}