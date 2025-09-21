/*
 * GLXcube - GLX and X11 sanity check
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

GLfloat position[3] = {0.0f, 0.0f, -5.0f};
GLfloat velocity[3] = {0.01f, 0.02f, 0.0f};
GLfloat rotation = 0.0f;
GLfloat rotation_speed = 0.5f;

Display *display;
Window window;
GLXContext glc;

void init_gl() {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void resize_gl(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void draw_cube() {
    GLfloat vertices[8][3] = {
        {-0.5, -0.5,  0.5},
        { 0.5, -0.5,  0.5},
        { 0.5,  0.5,  0.5},
        {-0.5,  0.5,  0.5},
        {-0.5, -0.5, -0.5},
        { 0.5, -0.5, -0.5},
        { 0.5,  0.5, -0.5},
        {-0.5,  0.5, -0.5}
    };
    GLint faces[6][4] = {
        {0, 1, 2, 3},
        {4, 7, 6, 5},
        {0, 4, 5, 1},
        {2, 6, 7, 3},
        {1, 5, 6, 2},
        {4, 0, 3, 7}
    };

    GLfloat face_colors[6][3] = {
        {1.0f, 0.0f, 0.0f}, 
        {0.0f, 1.0f, 0.0f}, 
        {0.0f, 0.0f, 1.0f}, 
        {1.0f, 1.0f, 0.0f}, 
        {0.0f, 1.0f, 1.0f}, 
        {1.0f, 0.0f, 1.0f}  
    };

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
        glColor3fv(face_colors[i]);
        for (int j = 0; j < 4; j++) {
            glVertex3fv(vertices[faces[i][j]]);
        }
    }
    glEnd();
}

void draw_scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(position[0], position[1], position[2]);
    glRotatef(rotation, 1.0f, 1.0f, 0.0f);
    draw_cube();
    glXSwapBuffers(display, window);
}

void update_logic() {
    position[0] += velocity[0];
    position[1] += velocity[1];

    if (position[0] > 1.5f || position[0] < -1.5f) {
        velocity[0] *= -1.0f;
    }

    if (position[1] > 1.2f || position[1] < -1.2f) {
        velocity[1] *= -1.0f;
    }

    rotation += rotation_speed;
    if (rotation > 360.0f) {
        rotation -= 360.0f;
    }
}

int main() {
    XEvent event;
    int screen_id;
    int visual_attributes[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };
    XVisualInfo *visual_info;
    Colormap colormap;
    XSetWindowAttributes window_attributes;
    Atom wm_delete_window;
    srand(time(NULL));

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Could not open screen.\n");
        return 1;
    }
    screen_id = DefaultScreen(display);
    visual_info = glXChooseVisual(display, screen_id, visual_attributes);
    if (!visual_info) {
        fprintf(stderr, "No appropriate visual found.\n");
        XCloseDisplay(display);
        return 1;
    }
    glc = glXCreateContext(display, visual_info, NULL, GL_TRUE);
    if (!glc) {
        fprintf(stderr, "Failed to create context.\n");
        XCloseDisplay(display);
        return 1;
    }

    colormap = XCreateColormap(display, RootWindow(display, screen_id), visual_info->visual, AllocNone);
    window_attributes.colormap = colormap;
    window_attributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
    window = XCreateWindow(display, RootWindow(display, screen_id), 0, 0, 800, 600, 0, visual_info->depth, InputOutput, visual_info->visual, CWColormap | CWEventMask, &window_attributes);
    XMapWindow(display, window);
    XStoreName(display, window, "GLXcube");

    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    glXMakeCurrent(display, window, glc);
    init_gl();

    while (1) {
        while (XPending(display)) {
            XNextEvent(display, &event);
            switch (event.type) {
                case Expose:
                    draw_scene();
                    break;
                case ConfigureNotify:
                    resize_gl(event.xconfigure.width, event.xconfigure.height);
                    draw_scene();
                    break;
                case KeyPress:
                    if (XLookupKeysym(&event.xkey, 0) == XK_Escape || XLookupKeysym(&event.xkey, 0) == XK_q) {
                        goto cleanup;
                    }
                    break;
                case ClientMessage:
                    if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
                        goto cleanup;
                    }
                    break;
            }
        }
        update_logic();
        draw_scene();
        usleep(10000);
    }

cleanup:
    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, glc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
