#pragma once

#include <string>

// Tao texture tu mang pixel RGBA (width x height)
unsigned int createTextureFromPixels(const unsigned char* pixels, int width, int height);

// Tao texture procedural (checkerboard, metal, concrete)
unsigned int createCheckerboardTexture(int size, int tileSize);
unsigned int createMetalTexture(int size);
unsigned int createConcreteTexture(int size);
unsigned int createSkyTexture(int width, int height);

// Load texture tu file PNG/JPG (dung stb_image)
unsigned int loadTextureFromFile(const std::string& path);

// Load tu assets/... tu dong tim theo thu muc exe
unsigned int loadTextureFromAssets(const std::string& relativePath);

// Bind texture va thiet lap tham so lap lai
void bindTexture(unsigned int textureId, float repeatS = 1.0f, float repeatT = 1.0f);
