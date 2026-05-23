#include "crane.h"
#include "geometry.h"
#include "texture.h"

#include <GL/glu.h>

#include <algorithm>
#include <cmath>

namespace {

constexpr float kPi = 3.14159265f;
constexpr float kDegToRad = kPi / 180.0f;

void drawTexQuad(float w, float h, float repeatS, float repeatT) {
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, -h);
    glTexCoord2f(repeatS, 0.0f); glVertex3f(w, 0.0f, -h);
    glTexCoord2f(repeatS, repeatT); glVertex3f(w, 0.0f, h);
    glTexCoord2f(0.0f, repeatT); glVertex3f(-w, 0.0f, h);
    glEnd();
}

}  // namespace

void Crane::init() {
    fileTexturesLoaded_ = 0;
    textures_.metal = createMetalTexture(256);
    textures_.concrete = createConcreteTexture(256);
    textures_.checker = createCheckerboardTexture(256, 32);
    textures_.hook = createMetalTexture(128);

    auto tryLoad = [this](unsigned int& slot, const char* path) {
        if (unsigned int t = loadTextureFromAssets(path)) {
            slot = t;
            ++fileTexturesLoaded_;
        }
    };

    tryLoad(textures_.metal, "assets/textures/metal.png");
    textures_.hook = textures_.metal;
    tryLoad(textures_.concrete, "assets/textures/concrete.png");
    tryLoad(textures_.checker, "assets/textures/ground.png");
    tryLoad(textures_.warning, "assets/textures/warning.png");
    if (textures_.warning == 0) {
        textures_.warning = createCheckerboardTexture(256, 16);
    }
    tryLoad(textures_.cargo, "assets/textures/cargo.png");
    if (textures_.cargo == 0) {
        textures_.cargo = textures_.concrete;
    }
}

void Crane::update(float /*deltaTime*/) {
    state_.boomAngle = std::clamp(state_.boomAngle, -10.0f, 75.0f);
    state_.cableLength = std::clamp(state_.cableLength, 1.5f, 8.0f);
    state_.trolleyPos = std::clamp(state_.trolleyPos, 0.1f, 0.95f);
}

void Crane::rotateBase(float degrees) {
    state_.baseRotation += degrees;
}

void Crane::adjustBoom(float degrees) {
    state_.boomAngle += degrees;
}

void Crane::adjustCable(float delta) {
    state_.cableLength += delta;
}

void Crane::moveTrolley(float delta) {
    state_.trolleyPos += delta;
}

void Crane::drawTexturedBox(float w, float h, float d, unsigned int texId, float repeat) const {
    bindTexture(texId, repeat, repeat);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(0.0f, h * 0.5f, 0.0f);
    glScalef(w, h, d);
    drawTexturedCube(repeat, repeat);
    glPopMatrix();
}

void Crane::drawTexturedCylinder(float radius, float height, int slices, unsigned int texId) const {
    bindTexture(texId, 4.0f, 2.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluQuadricNormals(quad, GLU_SMOOTH);

    glPushMatrix();
    glTranslatef(0.0f, height * 0.5f, 0.0f);
    gluCylinder(quad, radius, radius, height, slices, 4);
    glPopMatrix();

    gluDeleteQuadric(quad);
}

void Crane::drawBase() const {
    glPushMatrix();
    glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);

    // Nen be quay
    drawTexturedBox(2.2f, 0.3f, 2.2f, textures_.concrete, 2.0f);

    // Vong tru
    drawTexturedCylinder(1.0f, 0.25f, 32, textures_.metal);
    glPopMatrix();
}

void Crane::drawTower() const {
    glPushMatrix();
    glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);

    drawTexturedBox(0.9f, 5.0f, 0.9f, textures_.metal, 3.0f);

    // Gia cuong
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.55f);
    drawTexturedBox(0.15f, 4.5f, 0.15f, textures_.metal, 1.0f);
    glPopMatrix();

    glPopMatrix();
}

void Crane::drawCab() const {
    glPushMatrix();
    glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 5.2f, 0.0f);

    drawTexturedBox(1.4f, 1.2f, 1.4f, textures_.metal, 2.0f);

    // Cua kinh
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.4f, 0.7f, 0.95f, 0.7f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.71f);
    glScalef(0.8f, 0.6f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
}

void Crane::drawBoom() const {
    const float boomLength = 8.0f;

    glPushMatrix();
    glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 5.5f, 0.0f);
    glRotatef(-state_.boomAngle, 0.0f, 0.0f, 1.0f);

    bindTexture(textures_.warning, 6.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(boomLength * 0.5f, 0.0f, 0.0f);
    drawTexturedBox(boomLength, 0.35f, 0.5f, textures_.warning, 6.0f);
    glPopMatrix();

    // Counterweight
    glPushMatrix();
    glTranslatef(-1.2f, 0.1f, 0.0f);
    drawTexturedBox(1.5f, 0.8f, 0.8f, textures_.concrete, 1.5f);
    glPopMatrix();

    glPopMatrix();
}

void Crane::drawCableAndHook() const {
    const float boomLength = 8.0f;
    const float rad = state_.boomAngle * kDegToRad;

    const float pivotX = std::cos(rad) * boomLength * state_.trolleyPos;
    const float pivotY = 5.5f - std::sin(rad) * boomLength * state_.trolleyPos;

    glPushMatrix();
    glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
    glTranslatef(pivotX, pivotY, 0.0f);

    // Cap
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.15f, 0.15f, 0.15f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, -state_.cableLength, 0.0f);
    glEnd();

    // Moc
    glPushMatrix();
    glTranslatef(0.0f, -state_.cableLength, 0.0f);
    bindTexture(textures_.hook, 1.0f, 1.0f);
    glColor3f(0.8f, 0.2f, 0.15f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.08f, 0.25f, 12, 24);
    glPopMatrix();

    // Vat nang
    glPushMatrix();
    glTranslatef(0.0f, -state_.cableLength - 0.5f, 0.0f);
    drawTexturedBox(0.8f, 0.8f, 0.8f, textures_.cargo, 1.0f);
    glPopMatrix();

    glPopMatrix();
}

void Crane::draw() const {
    drawBase();
    drawTower();
    drawCab();
    drawBoom();
    drawCableAndHook();
}
