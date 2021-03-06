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

#include "SDL_mirhw.h"

int Mir_AllocHWSurface(_THIS, SDL_Surface* surface)
{
    return -1;
}

void Mir_FreeHWSurface(_THIS, SDL_Surface* surface)
{
}

int Mir_LockHWSurface(_THIS, SDL_Surface* surface)
{
    return 0;
}

void Mir_UnlockHWSurface(_THIS, SDL_Surface* surface)
{
}

int Mir_FlipHWSurface(_THIS, SDL_Surface* surface)
{
    return 0;
}
