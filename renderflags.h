
// renderflags.h

#ifndef RENDERFLAGS_H
#define RENDERFLAGS_H

#ifdef _WIN32
#include <windows.h>
//#include <gl\gl.h>
#endif

#ifdef __APPLE__
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include "cvars.h"
#endif

#include <SDL_opengl.h>


#include <SDL_opengl.h>

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
