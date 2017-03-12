#ifndef LAMPTEXTURE_H
#define LAMPTEXTURE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <map>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include "stb_image.h"

class LampTexture {
    int width;
    int height;
    int channels;
    unsigned char* data;
public:
    LampTexture(std::string fileName)
    {
        data = stbi_load(fileName.c_str(), &width, &height, &channels, 0);
    }
    glm::vec3 get_color(float u, float v){
        u = u  / 2.f / M_PI;
        v = v  / M_PI;

        u = u - floor(u);
        v = v - floor(v);
        if (u < 0 || u > 1 || v < 0 || v > 1) {
            return glm::vec3(0, 0, 0);
        }
        int w = int(u * (width-1));
        int h = int(v * (height-1));
        float r = float(data[(h * width + w) * channels]) / 255.f;
        float g = float(data[(h * width + w) * channels + 1]) / 255.f;
        float b = float(data[(h * width + w) * channels + 2]) / 255.f;
        return glm::vec3(r, g, b);
    }
    unsigned char* get_pointer(){
        return data;
    }
    int get_width(){
        return width;
    }
    int get_height(){
        return height;
    }
    int get_channels(){
        return channels;
    }
};

#endif
