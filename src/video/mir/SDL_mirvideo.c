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


#include "SDL_video.h"
#include "../SDL_pixels_c.h"
#include "../SDL_sysvideo.h"

#include "SDL_mirbuffer.h"
#include "SDL_mirevents.h"
#include "SDL_mirgl.h"
#include "SDL_mirhw.h"
#include "SDL_mirmouse.h"
#include "SDL_mirvideo.h"

static int Mir_VideoInit(_THIS, SDL_PixelFormat* vformat);
static void Mir_VideoQuit(_THIS);
static SDL_Surface* Mir_SetVideoMode(_THIS, SDL_Surface* current, int width,
                                     int height, int bpp, Uint32 flags);

SDL_Rect** Mir_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
    return this->hidden->modelist;
}

static int Mir_Available(void)
{
    return 1;
}

void Mir_PumpEvents(_THIS)
{
}

static int Mir_ToggleFullScreen(_THIS, int on)
{
    if (!mir_surface_is_valid(this->hidden->surface))
    {
        const char* error = mir_surface_get_error_message(this->hidden->surface);
        SDL_SetError("Failed to created a mir surface: %s", error);
        return 0;
    }

    if (on)
    {
        mir_surface_set_state(this->hidden->surface, mir_surface_state_fullscreen);
    }
    else
    {
        mir_surface_set_state(this->hidden->surface, mir_surface_state_restored);
    }

    return 1;
}

static void Mir_DeleteDevice(SDL_VideoDevice* device)
{
    if (device)
    {
        if (device->hidden)
            SDL_free(device->hidden);

        if (device->gl_data)
            SDL_free(device->gl_data);

        SDL_free(device);
    }
}

static SDL_VideoDevice* Mir_CreateDevice(int devindex)
{
	  SDL_VideoDevice *device = NULL;
    device = (SDL_VideoDevice*)SDL_calloc(1, sizeof(SDL_VideoDevice));

    if (!device)
    {
        SDL_OutOfMemory();
        return 0;
    }

	  device->hidden = (struct SDL_PrivateVideoData*)
                             SDL_calloc(1, (sizeof *device->hidden));
    if (!device->hidden)
    {
        Mir_DeleteDevice(device);
        SDL_OutOfMemory();
        return 0;
    }

#if SDL_VIDEO_OPENGL
	  device->gl_data = (struct SDL_PrivateGLData*)
                              SDL_calloc(1, (sizeof *device->gl_data));

    if (!device->gl_data)
    {
        Mir_DeleteDevice(device);
        SDL_OutOfMemory();
        return 0;
    }
#endif // SDL_VIDEO_OPENGL

    device->hidden->buffer_queue = SDL_calloc(1, sizeof(struct Queue));
    if (!device->hidden->buffer_queue)
    {
        Mir_DeleteDevice(device);
        SDL_OutOfMemory();
        return 0;
    }

    device->hidden->connection = NULL;
    device->hidden->surface = NULL;

    device->handles_any_size = 1;
    device->input_grab = 0;

    device->VideoInit    = Mir_VideoInit;
    device->VideoQuit    = Mir_VideoQuit;
    device->SetVideoMode = Mir_SetVideoMode;
    device->InitOSKeymap = Mir_InitOSKeymap;
    device->ListModes    = Mir_ListModes;
    device->UpdateMouse  = NULL;
    device->free         = Mir_DeleteDevice;

    device->ToggleFullScreen = Mir_ToggleFullScreen;

    device->AllocHWSurface  = Mir_AllocHWSurface;
    device->LockHWSurface   = Mir_LockHWSurface;
    device->UnlockHWSurface = Mir_UnlockHWSurface;
    device->FlipHWSurface   = Mir_FlipHWSurface;
    device->FreeHWSurface   = Mir_FreeHWSurface;

    device->CheckHWBlit   = NULL;
    device->SetHWColorKey = NULL;
    device->UpdateRects   = NULL;
    device->FillHWRect    = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha    = NULL;
    device->GetGammaRamp  = NULL;
    device->SetCaption    = NULL;
    device->GrabInput     = NULL;

    device->FreeWMCursor   = Mir_FreeWMCursor;
    device->CreateWMCursor = Mir_CreateWMCursor;
    device->ShowWMCursor   = Mir_ShowWMCursor;
    device->WarpWMCursor   = Mir_WrapWMCursor;
    device->CheckMouseMode = Mir_CheckMouseMode;
    device->MoveWMCursor   = NULL;

#if SDL_VIDEO_OPENGL
    device->GL_LoadLibrary    = Mir_GL_LoadLibrary;
    device->GL_GetProcAddress = Mir_GL_GetProcAddress;
    device->GL_GetAttribute   = Mir_GL_GetAttribute;
    device->GL_MakeCurrent    = Mir_GL_MakeCurrent;
    device->GL_SwapBuffers    = Mir_GL_SwapBuffers;
#endif // SDL_VIDEO_OPENGL

		device->PumpEvents = Mir_PumpEvents;

    return device;
}

VideoBootStrap Mir_bootstrap = {
    "mir", "Mir Video Driver",
    Mir_Available, Mir_CreateDevice
};

SDL_Surface* Mir_SetVideoMode(_THIS, SDL_Surface* current,
				int width, int height, int bpp, Uint32 flags)
{
    if (this->hidden->surface && mir_surface_is_valid(this->hidden->surface))
    {
         mir_surface_release_sync(this->hidden->surface);
         this->hidden->surface = NULL;
    }

    MirDisplayInfo dinfo;
    mir_connection_get_display_info(this->hidden->connection, &dinfo);
    if (width > dinfo.width || height > dinfo.height)
    {
        SDL_SetError("Error Screen size to large\n");
        return NULL;
    }

    // FIXME Why does only XRGB work for the pixel format?
    MirSurfaceParameters surfaceparm =
    {
        .name   = "MirSurface",
        .width  = width,
        .height = height,
        .pixel_format = mir_pixel_format_xrgb_8888,//mir_pixel_format_invalid,
        //.pixel_format = mir_pixel_format_xbgr_8888,//mir_pixel_format_invalid,
        .buffer_usage = mir_buffer_usage_software
    };

    if ((flags & SDL_OPENGL) == SDL_OPENGL)
      surfaceparm.buffer_usage = mir_buffer_usage_hardware;

    this->hidden->surface = mir_connection_create_surface_sync(this->hidden->connection,
                                                               &surfaceparm);

    if (!mir_surface_is_valid(this->hidden->surface))
    {
        const char* error = mir_surface_get_error_message(this->hidden->surface);
        SDL_SetError("Failed to created a mir surface: %s", error);
        return NULL;
    }

    MirEventDelegate delegate = {
        Mir_HandleSurfaceEvent,
        NULL
    };

    mir_surface_set_event_handler(this->hidden->surface, &delegate);

    if ((flags & SDL_OPENGL) == SDL_OPENGL)
    {
        current->flags |= SDL_OPENGL;

        if (Mir_GL_CreateESurface(this) < 0)
        {
            SDL_SetError("Could not Create EGL Surface");
            return NULL;
        }

        if (Mir_GL_CreateContext(this) < 0)
        {
            SDL_SetError("Could not Create GL Context");
            return NULL;
        }
    }
    else
    {
        if ((current->w != width || current->h != height))
        {
            current->pixels = NULL;
            current->w      = width;
            current->h      = height;
            current->pitch  = SDL_CalculatePitch(current);

            current->pixels = SDL_calloc(1, current->h * current->pitch);
            if (!current->pixels)
            {
                  SDL_OutOfMemory();
                  return NULL;
            }
            this->UpdateRects = Mir_UpdateRects;
        }
    }

    return current;
}

int Mir_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
    this->hidden->connection = mir_connect_sync(NULL, __PRETTY_FUNCTION__);

    if (!mir_connection_is_valid(this->hidden->connection))
    {
        SDL_SetError("Failed to connect to the Mir Server");
        return -1;
    }

    // FIXME Get the real value, though it is going to be 32 unless we use RGB_888
    vformat->BitsPerPixel = 32;

    Mir_InitQueue(this->hidden->buffer_queue);

    MirDisplayConfiguration* display_config =
            mir_connection_create_display_config(this->hidden->connection);

    this->hidden->modelist = (SDL_Rect**)SDL_calloc(1, (display_config->num_outputs + 1) * sizeof(SDL_Rect*));

    int d;
    int valid_outputs = 0;
    for (d = 0; d < display_config->num_outputs; d++)
    {
        MirDisplayOutput const* out = display_config->outputs + d;
        if (out->used && out->connected && out->num_modes > 0)
        {
            int num_modes;
            for (num_modes = 0; num_modes < out->num_modes; num_modes++)
            {
                this->hidden->modelist[valid_outputs] = (SDL_Rect*)SDL_calloc(1, sizeof(SDL_Rect));
                this->hidden->modelist[valid_outputs]->x = out->position_x;
                this->hidden->modelist[valid_outputs]->y = out->position_y;
                this->hidden->modelist[valid_outputs]->w = out->modes[num_modes].horizontal_resolution;
                this->hidden->modelist[valid_outputs]->h = out->modes[num_modes].vertical_resolution;

                valid_outputs++;
            }
        }
    }

    this->hidden->modelist[valid_outputs] = NULL;

    mir_display_config_destroy(display_config);

    this->info.wm_available = 1;

    return 0;
}

// FIXME Make sure we clean everything up, Also:
// we get a crash (call of virtual function) when releasing the connection :(
void Mir_VideoQuit(_THIS)
{
    Mir_DeleteQueue(this->hidden->buffer_queue);

		if (this->hidden->buffer_queue)
    {
        SDL_free(this->hidden->buffer_queue);
    }

    if (mir_surface_is_valid(this->hidden->surface))
    {
        mir_surface_release_sync(this->hidden->surface);
        this->hidden->surface = NULL;
    }

#if SDL_VIDEO_OPENGL
    if (this->gl_config.driver_loaded != 0)
    {
        Mir_GL_DeleteContext(this);
        Mir_GL_UnloadLibrary(this);
    }
#endif // SDL_VIDEO_OPENGL

    if (mir_connection_is_valid(this->hidden->connection))
    {
        mir_connection_release(this->hidden->connection);
        this->hidden->connection = NULL;
    }

    int i = 0;
    if (this->hidden->modelist)
    {
        while (this->hidden->modelist[i] != NULL)
        {
          SDL_free(this->hidden->modelist[i]);
          i++;
        }

        SDL_free(this->hidden->modelist);
    }
}
