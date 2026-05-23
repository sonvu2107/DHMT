#pragma once

#include <GL/freeglut.h>

struct CraneTextures {
    unsigned int metal = 0;
    unsigned int concrete = 0;
    unsigned int checker = 0;
    unsigned int hook = 0;
    unsigned int warning = 0;
    unsigned int cargo = 0;
};

struct CraneState {
    float baseRotation = 0.0f;   // Do quay cot (Y axis)
    float boomAngle = 25.0f;     // Goc can cau (do)
    float cableLength = 4.0f;    // Chieu dai cap
    float trolleyPos = 0.6f;     // Vi tri xe con tren can (0..1)
};

class Crane {
public:
    void init();
    void update(float deltaTime);
    void draw() const;

    CraneState& state() { return state_; }
    const CraneState& state() const { return state_; }

    void rotateBase(float degrees);
    void adjustBoom(float degrees);
    void adjustCable(float delta);
    void moveTrolley(float delta);

    int fileTexturesLoaded() const { return fileTexturesLoaded_; }

private:
    CraneState state_;
    CraneTextures textures_;
    int fileTexturesLoaded_ = 0;

    void drawBase() const;
    void drawTower() const;
    void drawBoom() const;
    void drawCableAndHook() const;
    void drawCab() const;

    void drawTexturedBox(float w, float h, float d, unsigned int texId, float repeat) const;
    void drawTexturedCylinder(float radius, float height, int slices, unsigned int texId) const;
};
