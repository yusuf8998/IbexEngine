#pragma once
#include <string>

class ShaderData
{
private:
    std::string source;
    std::string filename;

    std::string readFile(const std::string &filePath) const;
public:
    ShaderData(const std::string &path);
    ~ShaderData();

    std::string getSource() const { return source; }
};