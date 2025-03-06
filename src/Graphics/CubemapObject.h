#pragma once

#include <GLAD/glad.h>
#include <string>
#include <array>

class CubemapObject
{
private:
    GLuint textureID;
    std::string filePaths[6];

    void loadCubemap();

    void generateCubemap(unsigned char *data[6], int width, int height, int channels);

public:
    CubemapObject(const std::array<std::string, 6> &filePaths);
    CubemapObject(const std::string &cubemapDir, const std::string &extension = "png");
    ~CubemapObject();

    // Method to bind the texture array
    void bind() const;

    // Getter for the texture array ID
    GLuint getID() const { return textureID; }

    const std::string &getFilePath(size_t index) const { return filePaths[index]; }
};