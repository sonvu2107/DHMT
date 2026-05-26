#include <GL/freeglut.h>
#include <GL/glu.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fs = std::filesystem;

namespace {

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
    int lastTime = 0;
    bool leftDown = false;
    bool rightDown = false;
    float camAzimuth = 35.0f;
    float camElevation = 22.0f;
    float camDistance = 22.0f;
};

std::string resolveAssetPath(const std::string& rel) {
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

unsigned int uploadTexture(const unsigned char* pixels, int w, int h) {
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

unsigned int createSolidTexture(unsigned char r, unsigned char g, unsigned char b) {
    const unsigned char px[] = {r, g, b, 255};
    return uploadTexture(px, 1, 1);
}

unsigned int loadTextureFromAssets(const std::string& rel) {
    int w = 0, h = 0, ch = 0;
    std::string path = resolveAssetPath(rel);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 4);
    if (!data) return 0;
    unsigned int id = uploadTexture(data, w, h);
    stbi_image_free(data);
    return id;
}

void bindTexture(unsigned int id) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void drawTexturedCube(float repS = 1.0f, float repT = 1.0f) {
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

class Crane {
public:
    void init() {
        fileTexturesLoaded_ = 0;
        textures_.metal = createSolidTexture(150, 150, 150);
        textures_.concrete = createSolidTexture(130, 130, 130);
        textures_.boom = createSolidTexture(170, 140, 80);
        textures_.hook = createSolidTexture(90, 90, 90);
        textures_.cargo = createSolidTexture(120, 120, 120);
        textures_.cab = createSolidTexture(160, 160, 160);
        tryLoad(textures_.metal, {"assets/textures/roof1.bmp", "assets/textures/b.bmp"});
        tryLoad(textures_.concrete, {"assets/textures/bricks.bmp", "assets/textures/c.bmp"});
        tryLoad(textures_.boom, {"assets/textures/roof.bmp", "assets/textures/bricks1.bmp"});
        tryLoad(textures_.hook, {"assets/textures/b.bmp", "assets/textures/roof1.bmp"});
        tryLoad(textures_.cargo, {"assets/textures/door.bmp", "assets/textures/window1.bmp"});
        tryLoad(textures_.cab, {"assets/textures/window.bmp", "assets/textures/window1.bmp"});
    }

    void update() {
        state_.boomAngle = std::clamp(state_.boomAngle, -10.0f, 75.0f);
        state_.cableLength = std::clamp(state_.cableLength, 1.5f, 8.0f);
        state_.trolleyPos = std::clamp(state_.trolleyPos, 0.1f, 0.95f);
    }

    void rotateBase(float deg) { state_.baseRotation += deg; }
    void adjustBoom(float deg) { state_.boomAngle += deg; }
    void adjustCable(float d) { state_.cableLength += d; }
    void moveTrolley(float d) { state_.trolleyPos += d; }
    CraneState& state() { return state_; }
    const CraneState& state() const { return state_; }
    int fileTexturesLoaded() const { return fileTexturesLoaded_; }

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
    int fileTexturesLoaded_ = 0;

    void tryLoad(unsigned int& slot, std::initializer_list<const char*> paths) {
        for (const char* p : paths) {
            if (unsigned int t = loadTextureFromAssets(p)) {
                slot = t;
                ++fileTexturesLoaded_;
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
        glTranslatef(0.0f, 0.3f, 0.0f);
        drawBox(1.35f, 0.35f, 1.35f, textures_.metal, 1.0f);
        glPopMatrix();
    }

    void drawTower() const {
        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        drawBox(0.95f, 4.8f, 0.95f, textures_.metal, 2.5f);
        glTranslatef(0.0f, 4.8f, 0.0f);
        drawBox(1.55f, 0.45f, 1.2f, textures_.metal, 1.0f);
        glPopMatrix();
    }

    void drawCab() const {
        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 4.8f, 0.0f);
        drawBox(1.3f, 0.9f, 1.2f, textures_.cab, 1.0f);
        glPopMatrix();
    }

    void drawBoom() const {
        const float boomLen = 7.5f;
        glPushMatrix();
        glRotatef(state_.baseRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 5.25f, 0.0f);
        glRotatef(state_.boomAngle, 0.0f, 0.0f, 1.0f);
        drawBox(0.45f, 0.45f, 0.75f, textures_.metal, 1.0f);
        glPushMatrix();
        glTranslatef(boomLen * 0.5f, 0.0f, 0.0f);
        drawBox(boomLen, 0.4f, 0.6f, textures_.boom, 4.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(boomLen * state_.trolleyPos, -0.33f, 0.0f);
        drawBox(0.55f, 0.42f, 0.6f, textures_.metal, 1.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-1.0f, 0.08f, 0.0f);
        drawBox(1.15f, 0.7f, 0.75f, textures_.concrete, 1.0f);
        glPopMatrix();
        glPopMatrix();
    }

    void drawCableAndLoad() const {
        const float boomLen = 7.5f;
        const float trolleyYOffset = -0.33f;
        float r = state_.boomAngle * kDegToRad;
        float px = std::cos(r) * boomLen * state_.trolleyPos;
        float py = 5.25f + std::sin(r) * boomLen * state_.trolleyPos + trolleyYOffset;

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
        glPushMatrix();
        glTranslatef(0.0f, -0.28f, -0.18f);
        drawBox(0.1f, 0.35f, 0.1f, textures_.hook, 1.0f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.0f, -0.28f, 0.18f);
        drawBox(0.1f, 0.35f, 0.1f, textures_.hook, 1.0f);
        glPopMatrix();
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
unsigned int gSkyTopTex = 0;

float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    GLfloat amb[] = {0.25f, 0.25f, 0.28f, 1.0f}, dif[] = {0.85f, 0.85f, 0.88f, 1.0f};
    GLfloat spec[] = {0.6f, 0.6f, 0.65f, 1.0f}, p0[] = {8.0f, 14.0f, 6.0f, 1.0f}, p1[] = {-10.0f, 8.0f, -8.0f, 1.0f};
    GLfloat fill[] = {0.35f, 0.38f, 0.45f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb); glLightfv(GL_LIGHT0, GL_DIFFUSE, dif); glLightfv(GL_LIGHT0, GL_SPECULAR, spec); glLightfv(GL_LIGHT0, GL_POSITION, p0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fill); glLightfv(GL_LIGHT1, GL_POSITION, p1);
    glEnable(GL_NORMALIZE);
}

void setupCamera() {
    float az = gApp.camAzimuth * kPi / 180.0f, el = gApp.camElevation * kPi / 180.0f;
    float ex = gApp.camDistance * std::cos(el) * std::sin(az);
    float ey = gApp.camDistance * std::sin(el) + 3.0f;
    float ez = gApp.camDistance * std::cos(el) * std::cos(az);
    gluLookAt(ex, ey, ez, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void drawGround() {
    bindTexture(gGroundTex);
    glColor3f(1.0f, 1.0f, 1.0f);
    const float s = 30.0f;
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
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

    bindTexture(gSkyTopTex != 0 ? gSkyTopTex : gSkyTex);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-s, s,-s); glTexCoord2f(1,0); glVertex3f(s, s,-s); glTexCoord2f(1,1); glVertex3f(s, s, s); glTexCoord2f(0,1); glVertex3f(-s, s, s);
    glEnd();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
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

    // Nen HUD dam de tang tuong phan
    const int hudH = 104;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2i(0, gApp.windowH);
    glVertex2i(gApp.windowW, gApp.windowH);
    glVertex2i(gApp.windowW, gApp.windowH - hudH);
    glVertex2i(0, gApp.windowH - hudH);
    glEnd();
    glDisable(GL_BLEND);

    const CraneState& s = gCrane.state();
    char line4[256];
    std::snprintf(line4, sizeof(line4),
        "TEXTURE: %d/6  |  GOC COT: %.1f  |  GOC CAN: %.1f  |  CAP: %.1f  |  XE CON: %.0f%%",
        gCrane.fileTexturesLoaded(), s.baseRotation, s.boomAngle, s.cableLength, s.trolleyPos * 100.0f);

    const char* lines[] = {
        "MO PHONG CAU TRUC (OPENGL + GLUT + TEXTURE)",
        "A/D: XOAY COT  |  W/S: NANG/HA CAN  |  Q/E: CAP LEN/XUONG",
        "J/L: XE CON  |  CHUOT TRAI: XOAY CAMERA  |  CHUOT PHAI: ZOOM",
        line4
    };

    auto drawLine = [](int x, int y, const char* text) {
        // Bong den
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2i(x + 1, y - 1);
        for (const char* p = text; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        // Chu vang sang + ve lai lech 1px de day net hon
        glColor3f(1.0f, 0.95f, 0.15f);
        glRasterPos2i(x, y);
        for (const char* p = text; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        glRasterPos2i(x + 1, y);
        for (const char* p = text; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    };

    int y = gApp.windowH - 24;
    for (const char* line : lines) {
        drawLine(14, y, line);
        y -= 24;
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSky();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    setupCamera();
    setupLighting();
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
    if (key == 27) std::exit(0);
    unsigned char k = static_cast<unsigned char>(std::tolower(key));
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
    if (button == GLUT_LEFT_BUTTON) gApp.leftDown = (state == GLUT_DOWN);
    if (button == GLUT_RIGHT_BUTTON) gApp.rightDown = (state == GLUT_DOWN);
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
    if (gApp.rightDown) gApp.camDistance = clampf(gApp.camDistance - dy * 0.05f, 8.0f, 50.0f);
    glutPostRedisplay();
}

void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (gApp.lastTime == 0) gApp.lastTime = now;
    gApp.lastTime = now;
    gCrane.update();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void initGL() {
    glClearColor(0.55f, 0.78f, 0.95f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    gGroundTex = loadTextureFromAssets("assets/textures/roof1.bmp");
    if (gGroundTex == 0) gGroundTex = loadTextureFromAssets("assets/textures/roof.bmp");
    if (gGroundTex == 0) gGroundTex = createSolidTexture(90, 90, 90);
    gSkyTex = loadTextureFromAssets("assets/textures/sky.bmp");
    gSkyTopTex = loadTextureFromAssets("assets/textures/skytop.bmp");
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
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}
