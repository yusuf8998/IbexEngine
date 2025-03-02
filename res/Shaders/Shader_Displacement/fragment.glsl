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
layout(location = 5) in vec3 f_tangentViewPos;
layout(location = 6) in vec3 f_tangentFragPos;
layout(location = 7) in vec3 f_fragNormal;

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
    if (material.normalIndex == -1)
    return f_fragNormal;
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

vec4 calcDiffuse(LightColor color, vec3 normal, vec3 lightDir) {
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * color.diffuse * diff;
    return vec4(diffuse, 1.0);
}

const float kPi = 3.14159265359;

vec4 calcSpecular(LightColor color, vec3 normal, vec3 lightDir, vec3 viewDir) {
    if (material.specularIndex == -1)
    return vec4(vec3(0.0), 0.0);
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float energyConservation = (8.0 + material.shininess) / (8.0 * kPi);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = energyConservation * 0.5 * material.specular * color.specular * spec;
    return vec4(specular, 1.0);
}

vec4 calcDirLight(vec3 normal, vec3 viewDir) {
    vec4 result = vec4(0.0);
    vec4 ambient = vec4(0.0);
    vec4 diffuse = vec4(0.0);
    vec4 specular = vec4(0.0);

    vec3 direction = getLightDir(dirLight.direction);

    ambient = vec4(dirLight.color.ambient, 1.0) * diffuseTexture();
    diffuse = calcDiffuse(dirLight.color, normal, direction) * diffuseTexture();
    specular = calcSpecular(dirLight.color, normal, direction, viewDir) * specularTexture();

    result += ambient + diffuse + specular;

    return result;
}

vec4 calcPointLights(vec3 normal, vec3 viewDir) {
    vec4 result = vec4(0.0);
    for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
        vec4 ambient = vec4(0.0);
        vec4 diffuse = vec4(0.0);
        vec4 specular = vec4(0.0);

        if (pointLights[i].color.ambient == vec3(0.0) && pointLights[i].color.diffuse == vec3(0.0) && pointLights[i].color.specular == vec3(0.0))
        continue;

        vec3 direction = normalize(f_fragPos - pointLights[i].position);
        direction = getLightDir(direction);

        float distance    = length(pointLights[i].position - f_fragPos);
        float attenuation = 1.0 / (pointLights[i].attenuation.constant + pointLights[i].attenuation.linear * distance +
            pointLights[i].attenuation.quadratic * (distance * distance));

        ambient = vec4(pointLights[i].color.ambient, 1.0) * diffuseTexture();
        diffuse = calcDiffuse(pointLights[i].color, normal, direction) * diffuseTexture();
        specular = calcSpecular(pointLights[i].color, normal, direction, viewDir) * specularTexture();

        ambient  *= attenuation;
        diffuse  *= attenuation;
        specular *= attenuation;

        result += ambient + diffuse + specular;
    }
    return result;
}

vec4 calcSpotLights(vec3 normal, vec3 viewDir) {
    vec4 result = vec4(0.0);
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
        vec4 ambient = vec4(0.0);
        vec4 diffuse = vec4(0.0);
        vec4 specular = vec4(0.0);

        if (spotLights[i].color.ambient == vec3(0.0) && spotLights[i].color.diffuse == vec3(0.0) && spotLights[i].color.specular == vec3(0.0))
        continue;

        vec3 direction = getLightDir(spotLights[i].direction);

        vec3 lightDir = normalize(f_fragPos - spotLights[i].position);

        float distance    = length(spotLights[i].position - f_fragPos);
        float attenuation = 1.0 / (spotLights[i].attenuation.constant + spotLights[i].attenuation.linear * distance +
            spotLights[i].attenuation.quadratic * (distance * distance));

        float theta = dot(lightDir, direction);

        ambient = vec4(spotLights[i].color.ambient, 1.0) * diffuseTexture();

        if(theta > spotLights[i].cutOff.inner) {
            diffuse = calcDiffuse(spotLights[i].color, normal, direction) * diffuseTexture();
            specular = calcSpecular(spotLights[i].color, normal, direction, viewDir) * specularTexture();
        }

        ambient  *= attenuation;
        diffuse  *= attenuation;
        specular *= attenuation;

        result += ambient + diffuse + specular;
    }
    return result;
}

void main() {
    vec3 norm = normalMap();
    vec3 viewDir;
    if (material.normalIndex == -1) {
        viewDir = normalize(viewPos - f_fragPos);
    }
    else {
        viewDir = normalize(f_tangentViewPos - f_tangentFragPos);
    }

    FragColor = calcDirLight(norm, viewDir) + calcPointLights(norm, viewDir) + calcSpotLights(norm, viewDir);
}