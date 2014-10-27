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

#include "SDL_mirevents.h"

#include "../../events/SDL_events_c.h"
#include <xkbcommon/xkbcommon.h>

static SDLKey MISC_keymap[256];

void Mir_InitKeymap()
{
    int i;

    // Taken mostly from SDL_x11events.c, switched to xkbcommon keysym
    for ( i=0; i<SDL_arraysize(MISC_keymap); ++i )
        MISC_keymap[i] = SDLK_UNKNOWN;

    MISC_keymap[XKB_KEY_BackSpace&0xFF] = SDLK_BACKSPACE;
    MISC_keymap[XKB_KEY_Tab&0xFF]       = SDLK_TAB;
    MISC_keymap[XKB_KEY_Clear&0xFF]     = SDLK_CLEAR;
    MISC_keymap[XKB_KEY_Return&0xFF]    = SDLK_RETURN;
    MISC_keymap[XKB_KEY_Pause&0xFF]     = SDLK_PAUSE;
    MISC_keymap[XKB_KEY_Escape&0xFF]    = SDLK_ESCAPE;
    MISC_keymap[XKB_KEY_Delete&0xFF]    = SDLK_DELETE;

    MISC_keymap[XKB_KEY_KP_0&0xFF]         = SDLK_KP0;
    MISC_keymap[XKB_KEY_KP_1&0xFF]         = SDLK_KP1;
    MISC_keymap[XKB_KEY_KP_2&0xFF]         = SDLK_KP2;
    MISC_keymap[XKB_KEY_KP_3&0xFF]         = SDLK_KP3;
    MISC_keymap[XKB_KEY_KP_4&0xFF]         = SDLK_KP4;
    MISC_keymap[XKB_KEY_KP_5&0xFF]         = SDLK_KP5;
    MISC_keymap[XKB_KEY_KP_6&0xFF]         = SDLK_KP6;
    MISC_keymap[XKB_KEY_KP_7&0xFF]         = SDLK_KP7;
    MISC_keymap[XKB_KEY_KP_8&0xFF]         = SDLK_KP8;
    MISC_keymap[XKB_KEY_KP_9&0xFF]         = SDLK_KP9;
    MISC_keymap[XKB_KEY_KP_Insert&0xFF]    = SDLK_KP0;
    MISC_keymap[XKB_KEY_KP_End&0xFF]       = SDLK_KP1;
    MISC_keymap[XKB_KEY_KP_Down&0xFF]      = SDLK_KP2;
    MISC_keymap[XKB_KEY_KP_Page_Down&0xFF] = SDLK_KP3;
    MISC_keymap[XKB_KEY_KP_Left&0xFF]      = SDLK_KP4;
    MISC_keymap[XKB_KEY_KP_Begin&0xFF]     = SDLK_KP5;
    MISC_keymap[XKB_KEY_KP_Right&0xFF]     = SDLK_KP6;
    MISC_keymap[XKB_KEY_KP_Home&0xFF]      = SDLK_KP7;
    MISC_keymap[XKB_KEY_KP_Up&0xFF]        = SDLK_KP8;
    MISC_keymap[XKB_KEY_KP_Page_Up&0xFF]   = SDLK_KP9;
    MISC_keymap[XKB_KEY_KP_Delete&0xFF]    = SDLK_KP_PERIOD;
    MISC_keymap[XKB_KEY_KP_Decimal&0xFF]   = SDLK_KP_PERIOD;
    MISC_keymap[XKB_KEY_KP_Divide&0xFF]    = SDLK_KP_DIVIDE;
    MISC_keymap[XKB_KEY_KP_Multiply&0xFF]  = SDLK_KP_MULTIPLY;
    MISC_keymap[XKB_KEY_KP_Subtract&0xFF]  = SDLK_KP_MINUS;
    MISC_keymap[XKB_KEY_KP_Add&0xFF]       = SDLK_KP_PLUS;
    MISC_keymap[XKB_KEY_KP_Enter&0xFF]     = SDLK_KP_ENTER;
    MISC_keymap[XKB_KEY_KP_Equal&0xFF]     = SDLK_KP_EQUALS;

    MISC_keymap[XKB_KEY_Up&0xFF]        = SDLK_UP;
    MISC_keymap[XKB_KEY_Down&0xFF]      = SDLK_DOWN;
    MISC_keymap[XKB_KEY_Right&0xFF]     = SDLK_RIGHT;
    MISC_keymap[XKB_KEY_Left&0xFF]      = SDLK_LEFT;
    MISC_keymap[XKB_KEY_Insert&0xFF]    = SDLK_INSERT;
    MISC_keymap[XKB_KEY_Home&0xFF]      = SDLK_HOME;
    MISC_keymap[XKB_KEY_End&0xFF]       = SDLK_END;
    MISC_keymap[XKB_KEY_Page_Up&0xFF]   = SDLK_PAGEUP;
    MISC_keymap[XKB_KEY_Page_Down&0xFF] = SDLK_PAGEDOWN;

    MISC_keymap[XKB_KEY_F1&0xFF]  = SDLK_F1;
    MISC_keymap[XKB_KEY_F2&0xFF]  = SDLK_F2;
    MISC_keymap[XKB_KEY_F3&0xFF]  = SDLK_F3;
    MISC_keymap[XKB_KEY_F4&0xFF]  = SDLK_F4;
    MISC_keymap[XKB_KEY_F5&0xFF]  = SDLK_F5;
    MISC_keymap[XKB_KEY_F6&0xFF]  = SDLK_F6;
    MISC_keymap[XKB_KEY_F7&0xFF]  = SDLK_F7;
    MISC_keymap[XKB_KEY_F8&0xFF]  = SDLK_F8;
    MISC_keymap[XKB_KEY_F9&0xFF]  = SDLK_F9;
    MISC_keymap[XKB_KEY_F10&0xFF] = SDLK_F10;
    MISC_keymap[XKB_KEY_F11&0xFF] = SDLK_F11;
    MISC_keymap[XKB_KEY_F12&0xFF] = SDLK_F12;
    MISC_keymap[XKB_KEY_F13&0xFF] = SDLK_F13;
    MISC_keymap[XKB_KEY_F14&0xFF] = SDLK_F14;
    MISC_keymap[XKB_KEY_F15&0xFF] = SDLK_F15;

    MISC_keymap[XKB_KEY_Num_Lock&0xFF]    = SDLK_NUMLOCK;
    MISC_keymap[XKB_KEY_Caps_Lock&0xFF]   = SDLK_CAPSLOCK;
    MISC_keymap[XKB_KEY_Scroll_Lock&0xFF] = SDLK_SCROLLOCK;
    MISC_keymap[XKB_KEY_Shift_R&0xFF]     = SDLK_RSHIFT;
    MISC_keymap[XKB_KEY_Shift_L&0xFF]     = SDLK_LSHIFT;
    MISC_keymap[XKB_KEY_Control_R&0xFF]   = SDLK_RCTRL;
    MISC_keymap[XKB_KEY_Control_L&0xFF]   = SDLK_LCTRL;
    MISC_keymap[XKB_KEY_Alt_R&0xFF]       = SDLK_RALT;
    MISC_keymap[XKB_KEY_Alt_L&0xFF]       = SDLK_LALT;
    MISC_keymap[XKB_KEY_Meta_R&0xFF]      = SDLK_RMETA;
    MISC_keymap[XKB_KEY_Meta_L&0xFF]      = SDLK_LMETA;
    MISC_keymap[XKB_KEY_Super_L&0xFF]     = SDLK_LSUPER;
    MISC_keymap[XKB_KEY_Super_R&0xFF]     = SDLK_RSUPER;
    MISC_keymap[XKB_KEY_Mode_switch&0xFF] = SDLK_MODE;
    MISC_keymap[XKB_KEY_Multi_key&0xFF]   = SDLK_COMPOSE;

    MISC_keymap[XKB_KEY_Help&0xFF]    = SDLK_HELP;
    MISC_keymap[XKB_KEY_Print&0xFF]   = SDLK_PRINT;
    MISC_keymap[XKB_KEY_Sys_Req&0xFF] = SDLK_SYSREQ;
    MISC_keymap[XKB_KEY_Break&0xFF]   = SDLK_BREAK;
    MISC_keymap[XKB_KEY_Menu&0xFF]    = SDLK_MENU;
    MISC_keymap[XKB_KEY_Hyper_R&0xFF] = SDLK_MENU;
}

void Mir_InitOSKeymap(_THIS)
{
    Mir_InitKeymap();
}

void HandleMouseButton(Uint8 state, MirMotionButton button_state)
{
    static uint32_t last_sdl_button;
    uint32_t sdl_button;

    switch (button_state)
    {
        case mir_motion_button_primary:
            sdl_button = SDL_BUTTON_LEFT;
            break;
        case mir_motion_button_secondary:
            sdl_button = SDL_BUTTON_RIGHT;
            break;
        case mir_motion_button_tertiary:
            sdl_button = SDL_BUTTON_MIDDLE;
            break;
        case mir_motion_button_forward:
            sdl_button = SDL_BUTTON_X1;
            break;
        case mir_motion_button_back:
            sdl_button = SDL_BUTTON_X2;
            break;
        default:
            sdl_button = last_sdl_button;
            break;
    }

    last_sdl_button = sdl_button;
    SDL_PrivateMouseButton(state, sdl_button, 0, 0);
}

void Mir_HandleMotionEvent(MirSurface const* surface, MirMotionEvent const* motion)
{
    MirMotionButton button_state = motion->button_state;

    if (motion->pointer_coordinates[0].tool_type == mir_motion_tool_type_finger)
      button_state = mir_motion_button_primary;

    switch (motion->action)
    {
        case(mir_motion_action_move):
        case(mir_motion_action_hover_move):
            SDL_PrivateMouseMotion(0, 0, motion->pointer_coordinates[0].x, motion->pointer_coordinates[0].y);
            break;
        case(mir_motion_action_down):
        case(mir_motion_action_pointer_down):
            HandleMouseButton(SDL_PRESSED, button_state);
            break;
        case(mir_motion_action_up):
        case(mir_motion_action_pointer_up):
            HandleMouseButton(SDL_RELEASED, button_state);
            break;
        case(mir_motion_action_cancel):
        case(mir_motion_action_outside):
        case(mir_motion_action_scroll):
        case(mir_motion_action_hover_exit):
            break;
        default:
            break;
    }
}

void Mir_HandleKeyEvent(MirSurface const* surface, MirKeyEvent const* key_event)
{
    SDLKey key = SDLK_UNKNOWN;
    Uint8 key_state = 0;
    char text[8];
    int size = 0;

    if (key_event->action == mir_key_action_down)
        key_state = SDL_PRESSED;
    else if (key_event->action == mir_key_action_up)
        key_state = SDL_RELEASED;

    switch (key_event->key_code >> 8)
    {
        case(0x00):
            key = key_event->key_code & 0xFF;
          break;
        case(0xFF):
            key = MISC_keymap[key_event->key_code & 0xFF];
          break;
        default:
          break;
    }

    size = xkb_keysym_to_utf8(key_event->key_code, text, sizeof text);

    SDL_keysym keysym;
    keysym.sym = key;
    keysym.scancode = key_event->scan_code;
    keysym.mod = KMOD_NONE;
    memcpy(&keysym.unicode, text, size);

    SDL_PrivateKeyboard(key_state, &keysym);
}

void Mir_HandleSurfaceEvent(MirSurface* surface,
                            MirEvent const* event, void* context)
{
    switch (event->type)
    {
        case(mir_event_type_key):
            Mir_HandleKeyEvent(surface, &event->key);
            break;
        case(mir_event_type_motion):
            Mir_HandleMotionEvent(surface, &event->motion);
            break;
        default:
            break;
    }
}
