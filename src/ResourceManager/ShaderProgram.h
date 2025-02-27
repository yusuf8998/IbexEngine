#pragma once

#include <memory>
#include <ResourceManager/ShaderData.h>

class ShaderProgram
{
private:
    std::string name;
    std::shared_ptr<ShaderData> vertex, fragment, geometry;

public:
    ShaderProgram(const std::string &name, const std::shared_ptr<ShaderData> &vertex, const std::shared_ptr<ShaderData> &fragment, const std::shared_ptr<ShaderData> &geometry = nullptr);
    ShaderProgram(const std::string &folder);
    ~ShaderProgram() = default;

    std::string getName() const { return name; };
    std::shared_ptr<ShaderData> getVertex() const { return vertex; };
    std::shared_ptr<ShaderData> getFragment() const { return fragment; };
    std::shared_ptr<ShaderData> getGeometry() const { return geometry; };
};