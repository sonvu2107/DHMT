#include "geometry.h"

#include <GL/freeglut.h>

void drawTexturedCube(float repeatS, float repeatT) {
    const float h = 0.5f;

    auto face = [](float nx, float ny, float nz,
                   float x0, float y0, float z0,
                   float x1, float y1, float z1,
                   float x2, float y2, float z2,
                   float x3, float y3, float z3,
                   float s0, float t0, float s1, float t1,
                   float s2, float t2, float s3, float t3) {
        glNormal3f(nx, ny, nz);
        glBegin(GL_QUADS);
        glTexCoord2f(s0, t0); glVertex3f(x0, y0, z0);
        glTexCoord2f(s1, t1); glVertex3f(x1, y1, z1);
        glTexCoord2f(s2, t2); glVertex3f(x2, y2, z2);
        glTexCoord2f(s3, t3); glVertex3f(x3, y3, z3);
        glEnd();
    };

    // Mat truoc (+Z)
    face(0, 0, 1,
         -h, -h, h, h, -h, h, h, h, h, -h, h, h,
         0, 0, repeatS, 0, repeatS, repeatT, 0, repeatT);

    // Mat sau (-Z)
    face(0, 0, -1,
         h, -h, -h, -h, -h, -h, -h, h, -h, h, h, -h,
         0, 0, repeatS, 0, repeatS, repeatT, 0, repeatT);

    // Mat phai (+X)
    face(1, 0, 0,
         h, -h, h, h, -h, -h, h, h, -h, h, h, h,
         0, 0, repeatS, 0, repeatS, repeatT, 0, repeatT);

    // Mat trai (-X)
    face(-1, 0, 0,
         -h, -h, -h, -h, -h, h, -h, h, h, -h, h, -h,
         0, 0, repeatS, 0, repeatS, repeatT, 0, repeatT);

    // Mat tren (+Y)
    face(0, 1, 0,
         -h, h, h, h, h, h, h, h, -h, -h, h, -h,
         0, 0, repeatS, 0, repeatS, repeatT, 0, repeatT);

    // Mat duoi (-Y)
    face(0, -1, 0,
         -h, -h, -h, h, -h, -h, h, -h, h, -h, -h, h,
         0, 0, repeatS, 0, repeatS, repeatT, 0, repeatT);
}
