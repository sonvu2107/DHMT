#include "crane.h"
#include "texture.h"

#include <GL/freeglut.h>
#include <GL/glu.h>

#include <cmath>
#include <cstdio>

namespace {

Crane gCrane;
unsigned int gSkyTexture = 0;
unsigned int gGroundTexture = 0;

float gCamAzimuth = 35.0f;
float gCamElevation = 22.0f;
float gCamDistance = 22.0f;

int gLastMouseX = 0;
int gLastMouseY = 0;
bool gMouseLeftDown = false;
bool gMouseRightDown = false;

int gWindowWidth = 1280;
int gWindowHeight = 720;

int gLastTime = 0;

void drawGround() {
    const float size = 30.0f;
    bindTexture(gGroundTexture, 12.0f, 12.0f);
    glColor3f(0.85f, 0.85f, 0.85f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0f, -size);
    glTexCoord2f(12.0f, 0.0f); glVertex3f(size, 0.0f, -size);
    glTexCoord2f(12.0f, 12.0f); glVertex3f(size, 0.0f, size);
    glTexCoord2f(0.0f, 12.0f); glVertex3f(-size, 0.0f, size);
    glEnd();

    // Luoi ke tren mat dat
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.3f, 0.3f, 0.35f, 0.35f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_LINES);
    for (int i = -30; i <= 30; i += 2) {
        glVertex3f(static_cast<float>(i), 0.01f, -30.0f);
        glVertex3f(static_cast<float>(i), 0.01f, 30.0f);
        glVertex3f(-30.0f, 0.01f, static_cast<float>(i));
        glVertex3f(30.0f, 0.01f, static_cast<float>(i));
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

void drawSkyBackdrop() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(0.45f, 0.72f, 0.95f);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glColor3f(0.75f, 0.88f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
}

void setupCamera() {
    const float az = gCamAzimuth * 3.14159265f / 180.0f;
    const float el = gCamElevation * 3.14159265f / 180.0f;

    const float eyeX = gCamDistance * std::cos(el) * std::sin(az);
    const float eyeY = gCamDistance * std::sin(el) + 3.0f;
    const float eyeZ = gCamDistance * std::cos(el) * std::cos(az);

    gluLookAt(eyeX, eyeY, eyeZ, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat ambient[] = {0.25f, 0.25f, 0.28f, 1.0f};
    GLfloat diffuse[] = {0.85f, 0.85f, 0.88f, 1.0f};
    GLfloat specular[] = {0.6f, 0.6f, 0.65f, 1.0f};
    GLfloat pos0[] = {8.0f, 14.0f, 6.0f, 1.0f};
    GLfloat pos1[] = {-10.0f, 8.0f, -8.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    GLfloat fillDiff[] = {0.35f, 0.38f, 0.45f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fillDiff);
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);

    glEnable(GL_NORMALIZE);
}

void drawHud() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, gWindowWidth, 0, gWindowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    const auto& s = gCrane.state();
    char buffer[512];
    std::snprintf(buffer, sizeof(buffer),
        "Mo phong cau truc (OpenGL + GLUT + Texture)\n"
        "A/D: Xoay cot  |  W/S: Nang/ha can  |  Q/E: Cap len/xuong\n"
        "J/L: Xe con tren can  |  Chuot trai: Xoay camera  |  Chuot phai: Zoom\n"
        "Texture file: %d/5  |  Goc cot: %.1f  |  Goc can: %.1f  |  Cap: %.1f  |  Xe con: %.0f%%",
        gCrane.fileTexturesLoaded(), s.baseRotation, s.boomAngle, s.cableLength,
        s.trolleyPos * 100.0f);

    glRasterPos2i(16, gWindowHeight - 24);
    for (const char* p = buffer; *p; ++p) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
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

    drawSkyBackdrop();

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
    gWindowWidth = w;
    gWindowHeight = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, static_cast<double>(w) / h, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
        case 27:  // ESC
            std::exit(0);
        case 'a':
        case 'A':
            gCrane.rotateBase(3.0f);
            break;
        case 'd':
        case 'D':
            gCrane.rotateBase(-3.0f);
            break;
        case 'w':
        case 'W':
            gCrane.adjustBoom(2.0f);
            break;
        case 's':
        case 'S':
            gCrane.adjustBoom(-2.0f);
            break;
        case 'q':
        case 'Q':
            gCrane.adjustCable(-0.15f);
            break;
        case 'e':
        case 'E':
            gCrane.adjustCable(0.15f);
            break;
        case 'j':
        case 'J':
            gCrane.moveTrolley(-0.03f);
            break;
        case 'l':
        case 'L':
            gCrane.moveTrolley(0.03f);
            break;
        case 'r':
        case 'R':
            gCrane.state() = CraneState{};
            break;
        default:
            break;
    }
    gCrane.update(0.0f);
    glutPostRedisplay();
}

void specialKeys(int key, int /*x*/, int /*y*/) {
    switch (key) {
        case GLUT_KEY_LEFT:
            gCrane.rotateBase(3.0f);
            break;
        case GLUT_KEY_RIGHT:
            gCrane.rotateBase(-3.0f);
            break;
        case GLUT_KEY_UP:
            gCrane.adjustBoom(2.0f);
            break;
        case GLUT_KEY_DOWN:
            gCrane.adjustBoom(-2.0f);
            break;
        default:
            break;
    }
    gCrane.update(0.0f);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        gMouseLeftDown = (state == GLUT_DOWN);
    } else if (button == GLUT_RIGHT_BUTTON) {
        gMouseRightDown = (state == GLUT_DOWN);
    }
    gLastMouseX = x;
    gLastMouseY = y;
}

void motion(int x, int y) {
    const int dx = x - gLastMouseX;
    const int dy = y - gLastMouseY;
    gLastMouseX = x;
    gLastMouseY = y;

    if (gMouseLeftDown) {
        gCamAzimuth += dx * 0.4f;
        gCamElevation += dy * 0.25f;
        if (gCamElevation < 5.0f) gCamElevation = 5.0f;
        if (gCamElevation > 80.0f) gCamElevation = 80.0f;
    }
    if (gMouseRightDown) {
        gCamDistance -= dy * 0.05f;
        if (gCamDistance < 8.0f) gCamDistance = 8.0f;
        if (gCamDistance > 50.0f) gCamDistance = 50.0f;
    }
    glutPostRedisplay();
}

void timer(int /*value*/) {
    const int now = glutGet(GLUT_ELAPSED_TIME);
    const float dt = (gLastTime == 0) ? 0.0f : (now - gLastTime) / 1000.0f;
    gLastTime = now;

    gCrane.update(dt);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void initGL() {
    glClearColor(0.55f, 0.78f, 0.95f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);

    gSkyTexture = createSkyTexture(4, 256);
    gGroundTexture = createCheckerboardTexture(512, 64);

    unsigned int groundFile = loadTextureFromAssets("assets/textures/ground.png");
    if (groundFile != 0) {
        gGroundTexture = groundFile;
    }

    gCrane.init();
}

}  // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(gWindowWidth, gWindowHeight);
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
