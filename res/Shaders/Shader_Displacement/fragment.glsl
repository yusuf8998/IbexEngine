#version 450

#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4

struct LightColor {
    vec3 ambient, diffuse, specular;
};

struct LightAttenuation {
    float constant, linear, quadratic;
};

struct LightCutOff {
    float inner, outer;
};

struct DirectionalLight {
    vec3 direction;
    LightColor color;
};

struct PointLight {
    vec3 position;
    LightColor color;
    LightAttenuation attenuation;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    LightColor color;
    LightAttenuation attenuation;
    LightCutOff cutOff;
};

uniform DirectionalLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

struct Material {
    sampler2DArray textures;
    int diffuseIndex;
    int specularIndex;
    int normalIndex;
    int displacementIndex;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

uniform vec3 viewPos;

layout(location = 0) in vec3 f_fragPos;
layout(location = 1) in vec2 f_texCoords;
layout(location = 2) in mat3 f_TBN; // 2, 3, 4
layout(location = 5) in vec3 f_fragNormal;

layout(location = 0) out vec4 FragColor;  // Output color

vec4 diffuseTexture() {
    if (material.diffuseIndex == -1)
    return vec4(material.diffuse, 1.0);
    return texture(material.textures, vec3(f_texCoords, float(material.diffuseIndex)));
}
vec4 specularTexture() {
    if (material.specularIndex == -1)
    return vec4(material.specular, 1.0);
    return texture(material.textures, vec3(f_texCoords, float(material.specularIndex)));
}
vec3 normalMap() {
    return normalize(texture(material.textures, vec3(f_texCoords, float(material.normalIndex))).rgb * 2.0 - 1.0);
}

vec3 getLightDir(vec3 direction) {
    if (material.normalIndex == -1) {
        return normalize(-direction);
    }
    else {
        return normalize(f_TBN * -direction);
    }
}

vec4 calcAmbientLight(LightColor color) {
    return vec4(color.ambient, 1.0);
}

#pragma region calcDiffuse
vec4 calcDiffuseDirectional(vec3 normal) {
    float diff = max(dot(normal, getLightDir(dirLight.direction)), 0.0);
    vec3 diffuse = material.diffuse * dirLight.color.diffuse * diff;
    return vec4(diffuse, 1.0);
}

vec4 calcDiffusePoint(vec3 normal, int i) {
    if (pointLights[i].color.ambient == vec3(0.0) && pointLights[i].color.diffuse == vec3(0.0) && pointLights[i].color.specular == vec3(0.0))
        return vec4(0.0);
    vec3 lightDir = getLightDir(f_fragPos - pointLights[i].position);
    LightColor color = pointLights[i].color;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * color.diffuse * diff;

    float distance    = length(pointLights[i].position - f_fragPos);
    float attenuation = 1.0 / (pointLights[i].attenuation.constant + pointLights[i].attenuation.linear * distance +
        pointLights[i].attenuation.quadratic * (distance * distance));

    return vec4(diffuse, 1.0) * attenuation;
}

vec4 calcDiffuseSpot(vec3 normal, int i) {
    if (dot(f_fragPos - spotLights[i].position, spotLights[i].direction) < cos(spotLights[i].cutOff.outer))
        return vec4(0.0);
    if (spotLights[i].color.ambient == vec3(0.0) && spotLights[i].color.diffuse == vec3(0.0) && spotLights[i].color.specular == vec3(0.0))
        return vec4(0.0);
    vec3 lightDir = getLightDir(f_fragPos - spotLights[i].position);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * spotLights[i].color.diffuse * diff;

    float distance    = length(spotLights[i].position - f_fragPos);
    float attenuation = 1.0 / (spotLights[i].attenuation.constant + spotLights[i].attenuation.linear * distance +
        spotLights[i].attenuation.quadratic * (distance * distance));

    return vec4(diffuse, 1.0) * attenuation;
}
#pragma endregion
#pragma region calcSpecular
const float kPi = 3.14159265359;
vec4 calcSpecularDirectional(vec3 normal, vec3 viewDir) {
    vec3 halfwayDir = normalize(getLightDir(dirLight.direction) + viewDir);
    float energyConservation = (8.0 + material.shininess) / (8.0 * kPi);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = energyConservation * 0.5 * material.specular * dirLight.color.specular * spec;
    return vec4(specular, 1.0);
}
vec4 calcSpecularPoint(vec3 normal, vec3 viewDir, int i)
{
    if (pointLights[i].color.ambient == vec3(0.0) && pointLights[i].color.diffuse == vec3(0.0) && pointLights[i].color.specular == vec3(0.0))
        return vec4(0.0);
    vec3 lightDir = getLightDir(f_fragPos - pointLights[i].position);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float energyConservation = (8.0 + material.shininess) / (8.0 * kPi);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = energyConservation * 0.5 * material.specular * pointLights[i].color.specular * spec;

    float distance    = length(pointLights[i].position - f_fragPos);
    float attenuation = 1.0 / (pointLights[i].attenuation.constant + pointLights[i].attenuation.linear * distance +
        pointLights[i].attenuation.quadratic * (distance * distance));

    return vec4(specular, 1.0) * attenuation;
}
vec4 calcSpecularSpot(vec3 normal, vec3 viewDir, int i)
{
    if (dot(f_fragPos - spotLights[i].position, spotLights[i].direction) < cos(spotLights[i].cutOff.outer))
        return vec4(0.0);
    if (spotLights[i].color.ambient == vec3(0.0) && spotLights[i].color.diffuse == vec3(0.0) && spotLights[i].color.specular == vec3(0.0))
        return vec4(0.0);
    vec3 lightDir = getLightDir(f_fragPos - spotLights[i].position);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float energyConservation = (8.0 + material.shininess) / (8.0 * kPi);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = energyConservation * 0.5 * material.specular * spotLights[i].color.specular * spec;

    float distance    = length(spotLights[i].position - f_fragPos);
    float attenuation = 1.0 / (spotLights[i].attenuation.constant + spotLights[i].attenuation.linear * distance +
        spotLights[i].attenuation.quadratic * (distance * distance));

    return vec4(specular, 1.0) * attenuation;
}
#pragma endregion

void main() {
    vec3 norm = vec3(0.0);
    vec3 viewDir = normalize(viewPos - f_fragPos);
    if (material.normalIndex == -1) {
        norm = f_fragNormal;
    }
    else {
        norm = normalMap();
        viewDir = normalize(f_TBN * viewDir);
    }

    vec4 ambient = calcAmbientLight(dirLight.color);
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        ambient += calcAmbientLight(pointLights[i].color);
    }
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
    {
        ambient += calcAmbientLight(spotLights[i].color);
    }
    
    vec4 diffuse = calcDiffuseDirectional(norm);
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        diffuse += calcDiffusePoint(norm, i);
    }
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
    {
        diffuse += calcDiffuseSpot(norm, i);
    }

    vec4 specular = vec4(0.0);
    if (material.specularIndex != -1)
    {
        specular = calcSpecularDirectional(norm, viewDir);
        for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        {
            specular += calcSpecularPoint(norm, viewDir, i);
        }
        for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        {
            specular += calcSpecularSpot(norm, viewDir, i);
        }
    }

    FragColor = ambient * diffuseTexture() + diffuse * diffuseTexture() + specular * specularTexture();
}