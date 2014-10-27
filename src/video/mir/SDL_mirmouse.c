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

#include "SDL_mirmouse.h"

#include "../SDL_cursor_c.h"

struct WMcursor {
	int mir_cursor;
};

void Mir_FreeWMCursor(_THIS, WMcursor* cursor)
{
    SDL_free(cursor);
    cursor = NULL;
}

WMcursor* Mir_CreateWMCursor(_THIS, Uint8* data, Uint8* mask,
                                    int w, int h, int hot_x, int hot_y)
{
    WMcursor* cursor;

    cursor = (WMcursor*)SDL_calloc(1, sizeof(WMcursor));
    if (!cursor)
    {
        SDL_OutOfMemory();
        return NULL;
    }

    return cursor;
}

int Mir_ShowWMCursor(_THIS, WMcursor* cursor)
{
    return 1;
}

void Mir_WrapWMCursor(_THIS, Uint16 x, Uint16 y)
{
}

void Mir_CheckMouseMode(_THIS)
{
}
