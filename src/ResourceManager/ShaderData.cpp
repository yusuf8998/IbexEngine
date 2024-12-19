#include "ShaderData.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::string ShaderData::readFile(const std::string &filePath) const
{
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

ShaderData::ShaderData(const std::string &path)
    : filename(path)
{
    source = readFile(path);
}

ShaderData::~ShaderData()
{
}
