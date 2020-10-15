
// renderflags.h

#ifndef RENDERFLAGS_H
#define RENDERFLAGS_H

#ifdef _WIN32
#include <windows.h>
//#include <gl\gl.h>
#endif

#if defined __APPLE__ || defined __linux__
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include "cvars.h"
#endif

#include <SDL_opengl.h>
#ifdef GLES
#include <SDL_opengles.h>
#define GL_LINE                                     0x1B01
#define GL_FILL                                     0x1B02
#endif

extern const int render_blend;
extern const int render_texture;
extern const int render_depthtest;
extern const int render_lighting;
extern const int render_wireframe;

void renderenable(const int&);
void renderdisable(const int&);
void renderset(const int&, const BOOL&);
void renderinit(void);

/* void twBindTexture(const GLenum&, const GLuint&); */

#endif
