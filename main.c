#include <kos.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "model.h"

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

int main(int argc, char **argv)
{
    maple_device_t *cont;
    cont_state_t *state;

    glKosInit();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    wv_model model = load("/rd/nde/nde.obj");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    int i = 0;
    int rotation = 0;

    while (1)
    {
        cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        state = (cont_state_t *)maple_dev_status(cont);

        if (!state)
        {
            printf("Error reading controller\n");
            break;
        }

        if (state->buttons & CONT_START) break;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0.0f, -1.0f, -8.0f);
        glRotatef(rotation, 0.0f, 1.0f, 0.0f);

        for (i = 0; i < model.face_count; i++)
        {
            glBindTexture(GL_TEXTURE_2D, model.faces[i].material->texture_id);
            glColor3f(
                model.faces[i].material->diffuse[0],
                model.faces[i].material->diffuse[1],
                model.faces[i].material->diffuse[2]
            );
            glBegin(GL_TRIANGLES);

            int i1 = model.faces[i].indices[0];
            int i2 = model.faces[i].indices[1];
            int i3 = model.faces[i].indices[2];

            int uv1 = model.faces[i].uvs[0];
            int uv2 = model.faces[i].uvs[1];
            int uv3 = model.faces[i].uvs[2];

            glTexCoord2f(model.uvs[uv1 - 1].x, model.uvs[uv1 - 1].y);
            glVertex3f(
                model.vertices[i1 - 1].x,
                model.vertices[i1 - 1].y,
                model.vertices[i1 - 1].z);

            glTexCoord2f(model.uvs[uv2 - 1].x, model.uvs[uv2 - 1].y);
            glVertex3f(
                model.vertices[i2 - 1].x,
                model.vertices[i2 - 1].y,
                model.vertices[i2 - 1].z);

            glTexCoord2f(model.uvs[uv3 - 1].x, model.uvs[uv3 - 1].y);
            glVertex3f(
                model.vertices[i3 - 1].x,
                model.vertices[i3 - 1].y,
                model.vertices[i3 - 1].z);
            glEnd();
        }

        rotation++;
        if (rotation > 360) rotation = 0;

        glutSwapBuffers();
    }

    return 0;
}
