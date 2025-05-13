#version 460 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

// Output
out vec4 FragColor;

// Material properties
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// Directional light properties
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Uniforms
uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    
    // Final color
    FragColor = vec4(result, 1.0);
}

// Calculates the color contribution from a directional light source
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Ambient component
    vec3 ambient = light.ambient * material.ambient * Color;
    
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse * Color);
    
    // Specular component (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    return (ambient + diffuse + specular);
} 