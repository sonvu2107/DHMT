#include <GL/freeglut.h>
#include <GL/glu.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <initializer_list>

#define STB_IMAGE_IMPLEMENTATION
#include "app_render_utils.h"

namespace {

using namespace app_render;
using namespace std;

constexpr float kPi = 3.14159265f;
constexpr float kDegToRad = kPi / 180.0f;

struct CraneTextures {
    unsigned int metal = 0;
    unsigned int concrete = 0;
    unsigned int boom = 0;
    unsigned int hook = 0;
    unsigned int cargo = 0;
    unsigned int cab = 0;
};

struct CraneState {
    float baseRotation = 0.0f;
    float boomAngle = 25.0f;
    float cableLength = 4.0f;
    float trolleyPos = 0.6f;
};

struct AppState {
    int windowW = 1280;
    int windowH = 720;
    int lastMouseX = 0;
    int lastMouseY = 0;
    bool leftDown = false;
    float camAzimuth = 35.0f;
    float camElevation = 22.0f;
    float camDistance = 22.0f;
};

class Crane {
public:
    void init() {
        textures_.metal = createSolidTexture(150, 150, 150);
        textures_.concrete = createSolidTexture(130, 130, 130);
        textures_.boom = createSolidTexture(170, 140, 80);
        textures_.hook = createSolidTexture(90, 90, 90);
        textures_.cargo = createSolidTexture(120, 120, 120);
        textures_.cab = createSolidTexture(160, 160, 160);
        tryLoad(textures_.metal, {"assets/textures/roof1.bmp", "assets/textures/bricks.bmp"});
        tryLoad(textures_.concrete, {"assets/textures/bricks.bmp", "assets/textures/roof1.bmp"});
        tryLoad(textures_.boom, {"assets/textures/roof.bmp", "assets/textures/bricks1.bmp"});
        tryLoad(textures_.hook, {"assets/textures/roof1.bmp", "assets/textures/bricks1.bmp"});
        tryLoad(textures_.cargo, {"assets/textures/door.bmp", "assets/textures/window1.bmp"});
        tryLoad(textures_.cab, {"assets/textures/window.bmp", "assets/textures/window1.bmp"});
    }

    void update() {
        state_.boomAngle = clamp(state_.boomAngle, -10.0f, 75.0f);
        state_.cableLength = clamp(state_.cableLength, 1.5f, 8.0f);
        state_.trolleyPos = clamp(state_.trolleyPos, 0.1f, 0.95f);
    }

    void rotateBase(float deg) { state_.baseRotation += deg; }
    void adjustBoom(float deg) { state_.boomAngle += deg; }
    void adjustCable(float d) { state_.cableLength += d; }
    void moveTrolley(float d) { state_.trolleyPos += d; }
    CraneState& state() { return state_; }
    const CraneState& state() const { return state_; }

    void draw() const {
        drawBase();
        drawTower();
        drawCab();
        drawBoom();
        drawCableAndLoad();
    }

private:
    CraneState state_;
    CraneTextures textures_;

    void tryLoad(unsigned int& slot, initializer_list<const char*> paths) {
        for (const char* p : paths) {
            if (unsigned int t = loadTextureFromAssets(p)) {
                slot = t;
                return;
            }
        }
    }

    void drawBox(float w, float h, float d, unsigned int tex, float rep) const {
        bindTexture(tex);
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(0.0f, h * 0.5f, 0.0f);
        glScalef(w, h, d);
        drawTexturedCube(rep, rep);
        glPopMatrix();
    }

    void drawBase() const {
        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        drawBox(2.8f, 0.3f, 2.8f, textures_.concrete, 2.0f);
        glPopMatrix();
    }

    void drawTower() const {
        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        drawBox(0.95f, 4.8f, 0.95f, textures_.metal, 2.5f);
        glPopMatrix();
    }

    void drawCab() const {
        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 4.8f, 0.0f);
        glTranslatef(-0.22f, 0.0f, 0.0f);
        drawBox(1.75f, 0.9f, 1.2f, textures_.cab, 1.0f);
        glPopMatrix();
    }

    void drawBoom() const {
        const float boomLen = 7.5f;
        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 5.25f, 0.0f);
        glRotatef(state_.boomAngle, 0.0f, 0.0f, 1.0f);
        glPushMatrix();
        glTranslatef(-0.18f, 0.0f, 0.0f);
        drawBox(0.78f, 0.45f, 0.75f, textures_.metal, 1.0f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(boomLen * 0.5f, 0.0f, 0.0f);
        drawBox(boomLen, 0.4f, 0.6f, textures_.boom, 4.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(boomLen * state_.trolleyPos, -0.33f, 0.0f);
        drawBox(0.55f, 0.42f, 0.6f, textures_.metal, 1.0f);
        glPopMatrix();
        glPopMatrix();
    }

    void drawCableAndLoad() const {
        const float boomLen = 7.5f;
        const float trolleyYOffset = -0.33f;
        float r = state_.boomAngle * kDegToRad;
        float px = cos(r) * boomLen * state_.trolleyPos;
        float py = 5.25f + sin(r) * boomLen * state_.trolleyPos + trolleyYOffset;

        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(px, py, 0.0f);

        glDisable(GL_TEXTURE_2D);
        glColor3f(0.15f, 0.15f, 0.15f);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, -state_.cableLength, 0.0f);
        glEnd();

        glPushMatrix();
        glTranslatef(0.0f, -state_.cableLength, 0.0f);
        drawBox(0.3f, 0.3f, 0.3f, textures_.hook, 1.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, -state_.cableLength - 0.55f, 0.0f);
        drawBox(0.85f, 0.85f, 0.85f, textures_.cargo, 1.0f);
        glPopMatrix();
        glPopMatrix();
    }
};

Crane gCrane;
AppState gApp;
unsigned int gGroundTex = 0;
unsigned int gSkyTex = 0;

float clampf(float v, float lo, float hi) { return max(lo, min(v, hi)); }

void setupCamera() {
    float az = gApp.camAzimuth * kPi / 180.0f, el = gApp.camElevation * kPi / 180.0f;
    float ex = gApp.camDistance * cos(el) * sin(az);
    float ey = gApp.camDistance * sin(el) + 3.0f;
    float ez = gApp.camDistance * cos(el) * cos(az);
    gluLookAt(ex, ey, ez, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void drawGround() {
    bindTexture(gGroundTex);
    glColor3f(1.0f, 1.0f, 1.0f);
    const float s = 30.0f;
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, 0.0f, -s);
    glTexCoord2f(12.0f, 0.0f); glVertex3f(s, 0.0f, -s);
    glTexCoord2f(12.0f, 12.0f); glVertex3f(s, 0.0f, s);
    glTexCoord2f(0.0f, 12.0f); glVertex3f(-s, 0.0f, s);
    glEnd();
}

void drawSky() {
    if (gSkyTex == 0) return;
    const float s = 80.0f;
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    bindTexture(gSkyTex);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-s,-s,-s); glTexCoord2f(1,0); glVertex3f( s,-s,-s); glTexCoord2f(1,1); glVertex3f( s, s,-s); glTexCoord2f(0,1); glVertex3f(-s, s,-s);
    glTexCoord2f(0,0); glVertex3f( s,-s, s); glTexCoord2f(1,0); glVertex3f(-s,-s, s); glTexCoord2f(1,1); glVertex3f(-s, s, s); glTexCoord2f(0,1); glVertex3f( s, s, s);
    glTexCoord2f(0,0); glVertex3f(-s,-s, s); glTexCoord2f(1,0); glVertex3f(-s,-s,-s); glTexCoord2f(1,1); glVertex3f(-s, s,-s); glTexCoord2f(0,1); glVertex3f(-s, s, s);
    glTexCoord2f(0,0); glVertex3f( s,-s,-s); glTexCoord2f(1,0); glVertex3f( s,-s, s); glTexCoord2f(1,1); glVertex3f( s, s, s); glTexCoord2f(0,1); glVertex3f( s, s,-s);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-s, s,-s); glTexCoord2f(1,0); glVertex3f(s, s,-s); glTexCoord2f(1,1); glVertex3f(s, s, s); glTexCoord2f(0,1); glVertex3f(-s, s, s);
    glEnd();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void drawHud() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, gApp.windowW, 0, gApp.windowH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    const char* lines[] = {
        "A/D: XOAY COT  |  W/S: NANG/HA CAN  |  Q/E: CAP LEN/XUONG",
        "J/L: XE CON  |  CHUOT TRAI: XOAY CAMERA  |  CON LAN: ZOOM",
        "R: RESET  |  ESC: THOAT"
    };

    int y = gApp.windowH - 24;
    glColor3f(1.0f, 1.0f, 1.0f);
    for (const char* line : lines) {
        glRasterPos2i(14, y);
        for (const char* p = line; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        y -= 24;
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSky();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    setupCamera();
    drawGround();
    gCrane.draw();
    drawHud();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    gApp.windowW = w;
    gApp.windowH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, static_cast<double>(w) / h, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void applyControl(unsigned char key) {
    switch (key) {
        case 'a': gCrane.rotateBase(3.0f); break;
        case 'd': gCrane.rotateBase(-3.0f); break;
        case 'w': gCrane.adjustBoom(2.0f); break;
        case 's': gCrane.adjustBoom(-2.0f); break;
        case 'q': gCrane.adjustCable(-0.15f); break;
        case 'e': gCrane.adjustCable(0.15f); break;
        case 'j': gCrane.moveTrolley(-0.03f); break;
        case 'l': gCrane.moveTrolley(0.03f); break;
        default: break;
    }
}

void keyboard(unsigned char key, int, int) {
    if (key == 27) exit(0);
    unsigned char k = static_cast<unsigned char>(tolower(key));
    if (k == 'r') gCrane.state() = CraneState{};
    else applyControl(k);
    gCrane.update();
    glutPostRedisplay();
}

void specialKeys(int key, int, int) {
    switch (key) {
        case GLUT_KEY_LEFT: applyControl('a'); break;
        case GLUT_KEY_RIGHT: applyControl('d'); break;
        case GLUT_KEY_UP: applyControl('w'); break;
        case GLUT_KEY_DOWN: applyControl('s'); break;
        default: break;
    }
    gCrane.update();
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN && button == 3) {  // wheel up
        gApp.camDistance = clampf(gApp.camDistance - 1.0f, 8.0f, 50.0f);
        glutPostRedisplay();
        return;
    }
    if (state == GLUT_DOWN && button == 4) {  // wheel down
        gApp.camDistance = clampf(gApp.camDistance + 1.0f, 8.0f, 50.0f);
        glutPostRedisplay();
        return;
    }
    if (button == GLUT_LEFT_BUTTON) gApp.leftDown = (state == GLUT_DOWN);
    gApp.lastMouseX = x;
    gApp.lastMouseY = y;
}

void motion(int x, int y) {
    int dx = x - gApp.lastMouseX, dy = y - gApp.lastMouseY;
    gApp.lastMouseX = x;
    gApp.lastMouseY = y;
    if (gApp.leftDown) {
        gApp.camAzimuth += dx * 0.4f;
        gApp.camElevation = clampf(gApp.camElevation + dy * 0.25f, 5.0f, 80.0f);
    }
    glutPostRedisplay();
}

void initGL() {
    glClearColor(0.55f, 0.78f, 0.95f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    gGroundTex = loadTextureFromAssets("assets/textures/roof1.bmp");
    if (gGroundTex == 0) gGroundTex = loadTextureFromAssets("assets/textures/roof.bmp");
    if (gGroundTex == 0) gGroundTex = createSolidTexture(90, 90, 90);
    gSkyTex = loadTextureFromAssets("assets/textures/sky.bmp");
    gCrane.init();
}

}  // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(gApp.windowW, gApp.windowH);
    glutCreateWindow("Mo phong cau truc - OpenGL/GLUT");
    initGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}
