#pragma once

#include <GL/freeglut.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <filesystem>
#include <string>
#include <vector>

#include "stb_image.h"

namespace app_render {
namespace fs = std::filesystem;

inline std::string resolveAssetPath(const std::string& rel) {
    std::vector<fs::path> cands;
#ifdef _WIN32
    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) != 0) {
        fs::path exeDir = fs::path(exePath).parent_path();
        cands.push_back(exeDir / rel);
        cands.push_back(exeDir / ".." / rel);
        cands.push_back(exeDir / "../.." / rel);
    }
#endif
    cands.emplace_back(rel);
    cands.emplace_back(fs::path("..") / rel);
    cands.emplace_back(fs::path("../..") / rel);
    for (const auto& p : cands) {
        std::error_code ec;
        if (fs::exists(p, ec)) return fs::absolute(p).string();
    }
    return rel;
}

inline unsigned int uploadTexture(const unsigned char* pixels, int w, int h) {
    unsigned int id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

inline unsigned int createSolidTexture(unsigned char r, unsigned char g, unsigned char b) {
    const unsigned char px[] = {r, g, b, 255};
    return uploadTexture(px, 1, 1);
}

inline unsigned int loadTextureFromAssets(const std::string& rel) {
    int w = 0, h = 0, ch = 0;
    std::string path = resolveAssetPath(rel);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 4);
    if (!data) return 0;
    unsigned int id = uploadTexture(data, w, h);
    stbi_image_free(data);
    return id;
}

inline void bindTexture(unsigned int id) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

inline void drawTexturedCube(float repS = 1.0f, float repT = 1.0f) {
    const float v[8][3] = {
        {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
        { 0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}
    };
    const int f[6][4] = {{0,1,2,3}, {4,5,6,7}, {1,4,7,2}, {5,0,3,6}, {3,2,7,6}, {5,4,1,0}};
    const float n[6][3] = {{0,0,1}, {0,0,-1}, {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}};
    for (int i = 0; i < 6; ++i) {
        glNormal3fv(n[i]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[f[i][0]]);
        glTexCoord2f(repS, 0.0f); glVertex3fv(v[f[i][1]]);
        glTexCoord2f(repS, repT); glVertex3fv(v[f[i][2]]);
        glTexCoord2f(0.0f, repT); glVertex3fv(v[f[i][3]]);
        glEnd();
    }
}

}  // namespace app_render
