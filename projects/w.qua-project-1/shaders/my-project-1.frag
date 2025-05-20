#version 460 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

// Output
out vec4 FragColor;

// Material properties with std140 layout
layout(std140) uniform Material {
    vec3 ambientColor;
    float ambientIntensity;
    vec3 diffuseColor;
    float diffuseIntensity;
    vec3 specularColor;
    float specularIntensity;
    float shininess;
    vec3 padding;
} material;

// Directional light properties with std140 layout
layout(std140) uniform DirectionalLight {
    vec4 direction;
    vec4 color;
} light;

// For compatibility with existing code
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Uniforms
uniform vec3 viewPos;

// Function prototypes
vec3 CalcDirLight(vec3 normal, vec3 viewDir);

void main()
{
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Directional lighting
    vec3 result = CalcDirLight(norm, viewDir);
    
    // Final color
    FragColor = vec4(result, 1.0);
}

// Calculates the color contribution from a directional light source
vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    // Make sure we use the vec3 from direction correctly, ignoring the w component
    vec3 lightDir = normalize(-light.direction.xyz);
    
    // Debug: if lightDir is (0,0,0), use a default direction
    if(length(lightDir) < 0.001) {
        lightDir = normalize(vec3(0.0, -1.0, 0.0));
    }
    
    // Ambient component
    vec3 ambient = light.color.rgb * material.ambientColor * material.ambientIntensity * Color;
    
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color.rgb * diff * material.diffuseColor * material.diffuseIntensity * Color;
    
    // Specular component (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color.rgb * spec * material.specularColor * material.specularIntensity;
    
    return (ambient + diffuse + specular);
} 