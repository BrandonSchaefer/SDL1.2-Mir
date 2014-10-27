/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 2013 Canonical Ltd

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Brandon Schaefer
    brandon.schaefer@canonical.com
*/

#include "SDL_config.h"

#ifndef _SDL_mirgl_h
#define _SDL_mirgl_h

#if SDL_VIDEO_OPENGL
#include <EGL/egl.h>
#include "SDL_mirvideo.h"
#endif // SDL_VIDEO_OPENGL

struct SDL_PrivateGLData
{
#if SDL_VIDEO_OPENGL
    EGLDisplay edpy;
    EGLContext context;
    EGLConfig econf;
    EGLSurface esurface;
#endif // SDL_VIDEO_OPENGL
};

extern int Mir_GL_CreateESurface(_THIS);
extern int Mir_GL_CreateContext(_THIS);
extern void Mir_GL_DeleteContext(_THIS);
extern void Mir_GL_UnloadLibrary(_THIS);
#if SDL_VIDEO_OPENGL
extern int Mir_GL_LoadLibrary(_THIS, const char* path);
extern void *Mir_GL_GetProcAddress(_THIS, const char* proc);
extern int Mir_GL_GetAttribute(_THIS, SDL_GLattr attrib, int* value);
extern int Mir_GL_MakeCurrent(_THIS);
extern void Mir_GL_SwapBuffers(_THIS);
#endif // SDL_VIDEO_OPENGL

#endif 
