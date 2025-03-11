#version 450

#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4

struct LightColor {
    vec3 ambient, diffuse, specular;
};
// 3 x 3 = 9 locations

struct LightAttenuation {
    float constant, linear, quadratic;
};
// 3 locations

struct LightCutOff {
    float inner, outer;
};
// 2 locations

struct DirectionalLight {
    vec3 direction; // 3 locations
    LightColor color; // 9 locations
    sampler2D shadowMap; // 1 locations
    mat4 lightSpaceMatrix; // 4 x 4 = 16 locations
};
// 3 + 9 + 1 + 16 = 29 locations

struct PointLight {
    vec3 position; // 3 locations
    LightColor color; // 9 locations
    LightAttenuation attenuation; // 3 locations
    samplerCube shadowMap; // 1 locations
    mat4 lightSpaceMatrix[6]; // 6 x 4 x 4 = 96 locations
};
// 3 + 9 + 3 + 1 + 96 = 112 locations

struct SpotLight {
    vec3 position; // 3 locations
    vec3 direction; // 3 locations
    LightColor color; // 9 locations
    LightAttenuation attenuation; // 3 locations
    LightCutOff cutOff; // 2 locations
    // sampler2D shadowMap; // 1 locations
    // mat4 lightSpaceMatrix; // 4 x 4 = 16 locations
};
// 3 + 3 + 9 + 3 + 2 + 1 + 16 = 37 locations

uniform DirectionalLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

struct Material {
    sampler2DArray textures; // 1 locations
    int diffuseIndex; // 1 locations
    int specularIndex; // 1 locations
    int normalIndex; // 1 locations
    int displacementIndex; // 1 location
    vec3 diffuse; // 3 locations
    vec3 specular; // 3 locations
    float shininess; // 1 locations
};
// 1 + 1 + 1 + 1 + 1 + 3 + 3 + 1 = 12 locations

uniform Material material;

uniform vec3 viewPos;

layout(location = 0) in vec3 f_fragPos;
layout(location = 1) in vec2 f_texCoords;
layout(location = 2) in mat3 f_TBN; // 2, 3, 4
layout(location = 5) in vec3 f_fragNormal;

layout(binding = 3) uniform LightingUniforms
{
    DirectionalLight dirLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
    vec3 viewPos;
};

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
vec4 calcDiffuseDirectional(vec3 normal, float shadow) {
    float diff = max(dot(normal, getLightDir(dirLight.direction)), 0.0);
    vec3 diffuse = material.diffuse * dirLight.color.diffuse * diff;
    return vec4(diffuse, 1.0) * (1.0 - shadow);
}

vec4 calcDiffusePoint(vec3 normal, float shadow, int i) {
    if (pointLights[i].color.ambient == vec3(0.0) && pointLights[i].color.diffuse == vec3(0.0) && pointLights[i].color.specular == vec3(0.0))
        return vec4(0.0);
    vec3 lightDir = getLightDir(f_fragPos - pointLights[i].position);
    LightColor color = pointLights[i].color;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * color.diffuse * diff;

    float distance    = length(pointLights[i].position - f_fragPos);
    float attenuation = 1.0 / (pointLights[i].attenuation.constant + pointLights[i].attenuation.linear * distance +
        pointLights[i].attenuation.quadratic * (distance * distance));

    return vec4(diffuse, 1.0) * attenuation * (1.0 - shadow);
}

vec4 calcDiffuseSpot(vec3 normal, float shadow, int i) {
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

    return vec4(diffuse, 1.0) * attenuation * (1.0 - shadow);
}
#pragma endregion
#pragma region calcSpecular
const float kPi = 3.14159265359;
vec4 calcSpecularDirectional(vec3 normal, vec3 viewDir, float shadow) {
    vec3 halfwayDir = normalize(getLightDir(dirLight.direction) + viewDir);
    float energyConservation = (8.0 + material.shininess) / (8.0 * kPi);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = energyConservation * 0.5 * material.specular * dirLight.color.specular * spec;
    return vec4(specular, 1.0) * (1.0 - shadow);
}
vec4 calcSpecularPoint(vec3 normal, vec3 viewDir, float shadow, int i)
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

    return vec4(specular, 1.0) * attenuation * (1.0 - shadow);
}
vec4 calcSpecularSpot(vec3 normal, vec3 viewDir, float shadow, int i)
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

    return vec4(specular, 1.0) * attenuation * (1.0 - shadow);
}
#pragma endregion

// float calcDirShadow(vec4 fragPosLightSpace)
// {
//     vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//     projCoords = projCoords * 0.5 + 0.5;
//     float closestDepth = texture(dirLight.shadowMap, projCoords.xy).r;
//     float currentDepth = projCoords.z;
//     float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
//     return shadow;
// }

// float calcPointShadow(vec4 fragPosLightSpace, int i)
// {
//     vec3 fragToLight = fragPosLightSpace.xyz - pointLights[i].position;
//     float currentDepth = length(fragToLight);
//     float closestDepth = texture(pointLights[i].shadowMap, fragToLight).r;
//     float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
//     return shadow;
// }

// float calcSpotShadow(vec4 fragPosLightSpace, int i)
// {
//     vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//     projCoords = projCoords * 0.5 + 0.5;
//     float closestDepth = texture(spotLights[i].shadowMap, projCoords.xy).r;
//     float currentDepth = projCoords.z;
//     float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
//     return shadow;
// }

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

    float dirShadow;
    float pointShadow[MAX_POINT_LIGHTS];
    float spotShadow[MAX_SPOT_LIGHTS];

    dirShadow = 0.0;
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        pointShadow[i] = 0.0;
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        spotShadow[i] = 0.0;

    // dirShadow = calcDirShadow(dirLight.lightSpaceMatrix * vec4(f_fragPos, 1.0));
    // for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    // {
    //     pointShadow[i] = 0.0;
    //     for (int j = 0; j < 6; j++)
    //     {
    //         vec4 fragPosLightSpace = pointLights[i].lightSpaceMatrix[j] * vec4(f_fragPos, 1.0);
    //         pointShadow[i] += calcPointShadow(fragPosLightSpace, i);
    //     }
    // }
    // for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
    // {
    //     vec4 fragPosLightSpace = spotLights[i].lightSpaceMatrix * vec4(f_fragPos, 1.0);
    //     pointShadow[i] = calcSpotShadow(fragPosLightSpace, i);
    // }

    vec4 ambient = calcAmbientLight(dirLight.color);
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        ambient += calcAmbientLight(pointLights[i].color);
    }
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
    {
        ambient += calcAmbientLight(spotLights[i].color);
    }
    
    vec4 diffuse = calcDiffuseDirectional(norm, dirShadow);
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        diffuse += calcDiffusePoint(norm, pointShadow[i], i);
    }
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
    {
        diffuse += calcDiffuseSpot(norm, spotShadow[i], i);
    }

    vec4 specular = vec4(0.0);
    if (material.specularIndex != -1)
    {
        specular = calcSpecularDirectional(norm, viewDir, dirShadow);
        for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        {
            specular += calcSpecularPoint(norm, viewDir, pointShadow[i], i);
        }
        for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        {
            specular += calcSpecularSpot(norm, viewDir, spotShadow[i], i);
        }
    }

    FragColor = ambient * diffuseTexture() + diffuse * diffuseTexture() + specular * specularTexture();
}