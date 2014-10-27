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

#include "SDL_mirgl.h"

int Mir_GL_CreateESurface(_THIS)
{
  if (Mir_GL_LoadLibrary(this, NULL) < 0)
      return -1;

  EGLNativeWindowType egl_nwin = (EGLNativeWindowType)
                                 mir_surface_get_egl_native_window(this->hidden->surface);

  this->gl_data->esurface = eglCreateWindowSurface(this->gl_data->edpy,
                                                   this->gl_data->econf,
                                                   egl_nwin, NULL);

  if (this->gl_data->esurface == EGL_NO_SURFACE)
  {
      SDL_SetError("Error Could not create EGL Surface");
      return -1;
  }

  return 0;
}

int Mir_GL_LoadLibrary(_THIS, const char* path)
{
    if (this->gl_config.dll_handle != NULL)
        return 0;

    int major, minor;
    EGLint neglconfigs;

    EGLenum renderable_type = EGL_OPENGL_BIT;
    EGLenum rendering_api   = EGL_OPENGL_API;

    EGLint attribs[] =
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, renderable_type,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_BUFFER_SIZE,  this->gl_config.buffer_size,
        EGL_RED_SIZE,     this->gl_config.red_size,
        EGL_GREEN_SIZE,   this->gl_config.green_size,
        EGL_BLUE_SIZE,    this->gl_config.blue_size,
        EGL_ALPHA_SIZE,   this->gl_config.alpha_size,
        EGL_DEPTH_SIZE,   this->gl_config.depth_size,
        EGL_STENCIL_SIZE, this->gl_config.stencil_size,
        EGL_NONE,
    };

    this->gl_data->edpy = eglGetDisplay(
                            mir_connection_get_egl_native_display(this->hidden->connection));

    if (!eglInitialize(this->gl_data->edpy, &major, &minor))
    {
        SDL_SetError("Failed to initialize EGL");
        return -1;
    }

    eglBindAPI(rendering_api);

    if (!eglChooseConfig(this->gl_data->edpy, attribs,
                         &this->gl_data->econf, 1, &neglconfigs))
    {
        SDL_SetError("Failed to choose econfig");
        return -1;
    }

    if (neglconfigs != 1)
    {
        SDL_SetError("Failed to choose econfig");
        return -1;
    }

    this->gl_config.driver_loaded = 1;

    return 0;
}

void* Mir_GL_GetProcAddress(_THIS, const char* proc)
{
    return eglGetProcAddress(proc);
}

int Mir_GL_GetAttribute(_THIS, SDL_GLattr attrib, int* value)
{
  switch (attrib)
  {
      case SDL_GL_DOUBLEBUFFER:
           *value=this->gl_config.double_buffer;
           break;
      case SDL_GL_STENCIL_SIZE:
           *value=this->gl_config.stencil_size;
           break;
      case SDL_GL_DEPTH_SIZE:
           *value=this->gl_config.depth_size;
           break;
      case SDL_GL_RED_SIZE:
           *value=this->gl_config.red_size;
           break;
      case SDL_GL_GREEN_SIZE:
           *value=this->gl_config.green_size;
           break;
      case SDL_GL_BLUE_SIZE:
           *value=this->gl_config.blue_size;
           break;
      case SDL_GL_ALPHA_SIZE:
           *value=this->gl_config.alpha_size;
           break;
      case SDL_GL_ACCUM_RED_SIZE:
           *value=this->gl_config.accum_red_size;
           break;
      case SDL_GL_ACCUM_GREEN_SIZE:
           *value=this->gl_config.accum_green_size;
           break;
      case SDL_GL_ACCUM_BLUE_SIZE:
           *value=this->gl_config.accum_blue_size;
           break;
      case SDL_GL_ACCUM_ALPHA_SIZE:
           *value=this->gl_config.accum_alpha_size;
           break;
      case SDL_GL_STEREO:
           *value=this->gl_config.stereo;
           break;
      default:
           *value=0;
           return(-1);
  }

  return 0;
}

int Mir_GL_MakeCurrent(_THIS)
{
  EGLSurface* surface = this->gl_data->esurface;

  if (!eglMakeCurrent(this->gl_data->edpy, surface, surface, this->gl_data->context))
  {
      SDL_SetError("Unable to make EGL context current");
      return -1;
  }
 
  return 0;
}

int Mir_GL_CreateContext(_THIS)
{
  int client_version = 2;

  const EGLint context_atrribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, client_version, EGL_NONE
  };

  this->gl_data->context = eglCreateContext(this->gl_data->edpy, this->gl_data->econf,
                                            EGL_NO_CONTEXT, context_atrribs);

  if (this->gl_data->context == EGL_NO_CONTEXT)
  {
      SDL_SetError("Could not create EGL context");
      return -1;
  }

  if (Mir_GL_MakeCurrent(this) < 0)
  {
      return -1;
  }

  return 0;
}

void Mir_GL_DeleteContext(_THIS)
{
    eglDestroyContext(this->gl_data->edpy, this->gl_data->context);
    eglDestroySurface(this->gl_data->edpy, this->gl_data->esurface);

    eglMakeCurrent(this->gl_data->edpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglReleaseThread();

    this->gl_data->context = 0;
    this->gl_data->esurface = NULL;
}

void Mir_GL_UnloadLibrary(_THIS)
{
    if (this->gl_config.driver_loaded)
    {
        eglTerminate(this->gl_data->edpy);

        this->gl_config.dll_handle = NULL;
        this->gl_config.driver_loaded = 0;
    }
}

void Mir_GL_SwapBuffers(_THIS)
{
    eglSwapBuffers(this->gl_data->edpy, this->gl_data->esurface);
}
