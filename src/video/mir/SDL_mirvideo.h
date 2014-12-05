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

#ifndef _SDL_mirvideo_h
#define _SDL_mirvideo_h

#include "../SDL_sysvideo.h"

#include <mir_toolkit/mir_client_library.h>

#define _THIS   SDL_VideoDevice *this

struct SDL_PrivateVideoData {
    MirConnection* connection;
    MirSurface* surface;
    MirPixelFormat pixel_format;

    struct Queue* buffer_queue;

    SDL_bool mode_changed;
    SDL_Rect** modelist;
};

#endif //_SDL_mirvideo_h
