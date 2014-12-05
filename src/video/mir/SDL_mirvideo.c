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
        mir_surface_release_sync(this->hidden->surface);
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

    Uint32 output_id = mir_display_output_id_invalid;

    if (flags & SDL_FULLSCREEN)
    {
        MirDisplayConfiguration* display_config =
                mir_connection_create_display_config(this->hidden->connection);

        Uint32 fallback_output_id = mir_display_output_id_invalid;
        Uint32 d;
        Uint32 m;

        this->hidden->mode_changed = SDL_FALSE;

        for (d = 0; d < display_config->num_outputs; ++d)
        {
            MirDisplayOutput const* out = display_config->outputs + d;
            if (out->used && out->connected)
            {
                if (out->modes[out->current_mode].horizontal_resolution == width &&
                    out->modes[out->current_mode].vertical_resolution == height)
                {
                    output_id = out->output_id;
                    break;
                }

                if (fallback_output_id == mir_display_output_id_invalid &&
                    out->modes[out->current_mode].horizontal_resolution >= width &&
                    out->modes[out->current_mode].vertical_resolution >= height)
                {
                    fallback_output_id = out->output_id;
                }
            }
        }

        if (output_id == mir_display_output_id_invalid)
        {
            for (d = 0; d < display_config->num_outputs; ++d)
            {
                MirDisplayOutput* out = display_config->outputs + d;
                if (out->used && out->connected)
                {
                    for (m = 0; m < out->num_modes; ++m)
                    {
                        if (out->modes[m].horizontal_resolution == width &&
                            out->modes[m].vertical_resolution == height)
                        {
                            this->hidden->mode_changed = SDL_TRUE;
                            output_id = out->output_id;
                            out->current_mode = m;

                            mir_wait_for(
                                mir_connection_apply_display_config(this->hidden->connection,
                                                                    display_config)
                            );
                            break;
                        }
                    }
                }
            }
        }

        if (fallback_output_id == mir_display_output_id_invalid)
        {
            /* There's no native resolution for the requested format, so let's
             * just ensure we've an output large enough to show it */

            for (d = 0; d < display_config->num_outputs; ++d)
            {
                MirDisplayOutput* out = display_config->outputs + d;
                if (out->used && out->connected)
                {
                    for (m = 0; m < out->num_modes; ++m)
                    {
                        if (out->modes[m].horizontal_resolution >= width &&
                            out->modes[m].vertical_resolution >= height)
                        {
                            this->hidden->mode_changed = SDL_TRUE;
                            fallback_output_id = out->output_id;
                            out->current_mode = m;

                            mir_wait_for(
                                mir_connection_apply_display_config(this->hidden->connection,
                                                                    display_config)
                            );
                            break;
                        }
                    }
                }
            }

            /* Setting output_id = fallback_output_id here seems to cause
             * troubles to mir in creating a new surface */
        }

        mir_display_config_destroy(display_config);

        if (output_id == mir_display_output_id_invalid &&
            fallback_output_id == mir_display_output_id_invalid)
        {
            SDL_SetError("Impossible to find a valid output for mode %dx%d",
                         width, height);
            return NULL;
        }
    }
    else if (this->hidden->mode_changed)
    {
        Uint32 d;
        SDL_bool any_changed = SDL_FALSE;

        MirDisplayConfiguration* display_config =
                mir_connection_create_display_config(this->hidden->connection);

        for (d = 0; d < display_config->num_outputs; ++d)
        {
            MirDisplayOutput* out = display_config->outputs + d;
            if (out->used && out->connected)
            {
                if (out->current_mode != out->preferred_mode)
                {
                    out->current_mode = out->preferred_mode;
                    any_changed = SDL_TRUE;
                }
            }
        }

        if (any_changed)
        {
            mir_wait_for(
                mir_connection_apply_display_config(this->hidden->connection,
                                                    display_config)
            );
        }

        this->hidden->mode_changed = SDL_FALSE;
        mir_display_config_destroy(display_config);
    }

    MirSurfaceParameters surfaceparm =
    {
        .name   = "MirSurface",
        .width  = width,
        .height = height,
        .pixel_format = this->hidden->pixel_format,
        .output_id = output_id,
        .buffer_usage = (flags & SDL_OPENGL) ? mir_buffer_usage_hardware :
                                               mir_buffer_usage_software,
    };

    this->hidden->surface =
        mir_connection_create_surface_sync(this->hidden->connection, &surfaceparm);

    if (!mir_surface_is_valid(this->hidden->surface))
    {
        const char* error = mir_surface_get_error_message(this->hidden->surface);
        SDL_SetError("Failed to created a mir surface: %s", error);
        mir_surface_release_sync(this->hidden->surface);
        return NULL;
    }

    MirEventDelegate delegate = {
        Mir_HandleSurfaceEvent,
        NULL
    };

    mir_surface_set_event_handler(this->hidden->surface, &delegate);

    if (flags & SDL_OPENGL)
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

static void Mir_ModeListFree(_THIS)
{
    if (this->hidden->modelist)
    {
        int i = 0;
        while (this->hidden->modelist[i] != NULL)
        {
          SDL_free(this->hidden->modelist[i]);
          ++i;
        }

        SDL_free(this->hidden->modelist);
        this->hidden->modelist = NULL;
    }
}

static void Mir_ModeListUpdate(_THIS)
{
    Uint32 d, m;
    Uint32 valid_outputs = 0;

    Mir_ModeListFree(this);

    MirDisplayConfiguration* display_config =
            mir_connection_create_display_config(this->hidden->connection);

    for (d = 0; d < display_config->num_outputs; d++)
    {
        MirDisplayOutput const* out = display_config->outputs + d;
        if (out->used && out->connected)
            valid_outputs += out->num_modes;
    }

    this->hidden->modelist = SDL_calloc(valid_outputs + 1, sizeof(SDL_Rect*));

    valid_outputs = 0;

    for (d = 0; d < display_config->num_outputs; ++d)
    {
        MirDisplayOutput const* out = display_config->outputs + d;
        if (out->used && out->connected)
        {
            for (m = 0; m < out->num_modes; ++m)
            {
                SDL_Rect* sdl_output = SDL_calloc(1, sizeof(SDL_Rect));
                sdl_output->x = out->position_x;
                sdl_output->y = out->position_y;
                sdl_output->w = out->modes[m].horizontal_resolution;
                sdl_output->h = out->modes[m].vertical_resolution;
                this->hidden->modelist[valid_outputs] = sdl_output;

                ++valid_outputs;
            }
        }
    }

    this->hidden->modelist[valid_outputs] = NULL;

    mir_display_config_destroy(display_config);
}

static void Mir_DisplayConfigChanged(MirConnection *connection, void* data)
{
    Mir_ModeListUpdate(data);
}

int Mir_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
    this->hidden->connection = mir_connect_sync(NULL, __PRETTY_FUNCTION__);

    if (!mir_connection_is_valid(this->hidden->connection))
    {
        SDL_SetError("Failed to connect to the Mir Server: %s",
                     mir_connection_get_error_message(this->hidden->connection));
        mir_connection_release(this->hidden->connection);
        return -1;
    }

    MirPixelFormat formats[mir_pixel_formats];
    Uint32 n_formats;

    mir_connection_get_available_surface_formats (this->hidden->connection, formats,
                                                  mir_pixel_formats, &n_formats);

    if (n_formats == 0 || formats[0] == mir_pixel_format_invalid)
    {
        SDL_SetError("No valid pixel formats found");
        mir_connection_release(this->hidden->connection);
        return -1;
    }

    this->hidden->pixel_format = formats[0];
    vformat->BitsPerPixel = MIR_BYTES_PER_PIXEL(this->hidden->pixel_format) * 8;

    Mir_InitQueue(this->hidden->buffer_queue);
    Mir_ModeListUpdate(this);
    mir_connection_set_display_config_change_callback(this->hidden->connection,
                                                      Mir_DisplayConfigChanged, this);

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

    if (this->hidden->surface)
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
        mir_connection_set_display_config_change_callback(this->hidden->connection,
                                                          NULL, NULL);
    }

    if (this->hidden->connection)
    {
        mir_connection_release(this->hidden->connection);
        this->hidden->connection = NULL;
    }

    Mir_ModeListFree(this);
}
