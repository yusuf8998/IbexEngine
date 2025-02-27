#include <ResourceManager/ShaderProgram.h>
#include <sys/stat.h>
#include <stdexcept>

ShaderProgram::ShaderProgram(const std::string &name, const std::shared_ptr<ShaderData> &vertex, const std::shared_ptr<ShaderData> &fragment, const std::shared_ptr<ShaderData> &geometry)
    : name(name), vertex(vertex), fragment(fragment), geometry(geometry)
{
}

ShaderProgram::ShaderProgram(const std::string &folder)
{
    name = folder;

    struct stat buffer;
    int vertex_file = stat((folder + "/vertex.glsl").c_str(), &buffer);
    int fragment_file = stat((folder + "/fragment.glsl").c_str(), &buffer);
    int geometry_file = stat((folder + "/geometry.glsl").c_str(), &buffer);

    if (vertex_file == 0)
        vertex = std::make_shared<ShaderData>(folder + "/vertex.glsl");
    else
        throw std::runtime_error("Vertex shader not found in " + folder);

    if (fragment_file == 0)
        fragment = std::make_shared<ShaderData>(folder + "/fragment.glsl");
    else
        throw std::runtime_error("Fragment shader not found in " + folder);
    
    if (geometry_file == 0)
        geometry = std::make_shared<ShaderData>(folder + "/geometry.glsl");
}
