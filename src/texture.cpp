#include "texture.h"
#include "asset_path.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/freeglut.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace {

unsigned int uploadTexture(const unsigned char* pixels, int width, int height) {
    unsigned int id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

void setPixel(std::vector<unsigned char>& buf, int width, int x, int y,
              unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    int idx = (y * width + x) * 4;
    buf[idx + 0] = r;
    buf[idx + 1] = g;
    buf[idx + 2] = b;
    buf[idx + 3] = a;
}

}  // namespace

unsigned int createTextureFromPixels(const unsigned char* pixels, int width, int height) {
    return uploadTexture(pixels, width, height);
}

unsigned int createCheckerboardTexture(int size, int tileSize) {
    std::vector<unsigned char> pixels(size * size * 4);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool dark = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            unsigned char c = dark ? 90 : 180;
            setPixel(pixels, size, x, y, c, c, c);
        }
    }
    return uploadTexture(pixels.data(), size, size);
}

unsigned int createMetalTexture(int size) {
    std::vector<unsigned char> pixels(size * size * 4);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float nx = static_cast<float>(x) / size;
            float ny = static_cast<float>(y) / size;
            float stripe = 0.5f + 0.5f * std::sin(nx * 40.0f + ny * 8.0f);
            float scratch = 0.5f + 0.5f * std::sin(ny * 120.0f);
            float v = 0.35f + 0.25f * stripe + 0.1f * scratch;
            unsigned char c = static_cast<unsigned char>(std::clamp(v * 255.0f, 0.0f, 255.0f));
            setPixel(pixels, size, x, y, c, static_cast<unsigned char>(c * 0.95f),
                     static_cast<unsigned char>(c * 0.85f));
        }
    }
    return uploadTexture(pixels.data(), size, size);
}

unsigned int createConcreteTexture(int size) {
    std::vector<unsigned char> pixels(size * size * 4);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int noise = (x * 13 + y * 7 + (x ^ y) * 3) % 40;
            unsigned char c = static_cast<unsigned char>(140 + noise);
            setPixel(pixels, size, x, y, c, c - 5, c - 10);
        }
    }
    return uploadTexture(pixels.data(), size, size);
}

unsigned int createSkyTexture(int width, int height) {
    std::vector<unsigned char> pixels(width * height * 4);
    for (int y = 0; y < height; ++y) {
        float t = static_cast<float>(y) / height;
        unsigned char r = static_cast<unsigned char>(120 + t * 60);
        unsigned char g = static_cast<unsigned char>(170 + t * 40);
        unsigned char b = static_cast<unsigned char>(220 + t * 20);
        for (int x = 0; x < width; ++x) {
            setPixel(pixels, width, x, y, r, g, b);
        }
    }
    return uploadTexture(pixels.data(), width, height);
}

unsigned int loadTextureFromFile(const std::string& path) {
    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        return 0;
    }
    unsigned int id = uploadTexture(data, width, height);
    stbi_image_free(data);
    return id;
}

unsigned int loadTextureFromAssets(const std::string& relativePath) {
    return loadTextureFromFile(resolveAssetPath(relativePath));
}

void bindTexture(unsigned int textureId, float repeatS, float repeatT) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
