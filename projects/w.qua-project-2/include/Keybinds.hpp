/**
 * @file Keybinds.hpp
 * @brief Constants and definitions for keyboard and mouse input handling.
 *
 * This file provides centralized definitions of key codes, mouse buttons, and input actions
 * for consistent input handling across the application.
 */

#pragma once

namespace Keybinds 
{
    // Input modes
    static const int CURSOR = 0x00033001;
    static const int CURSOR_NORMAL = 0x00034001;
    static const int CURSOR_DISABLED = 0x00034003;
    
    // Mouse buttons
    static const int MOUSE_BUTTON_LEFT = 0;
    static const int MOUSE_BUTTON_RIGHT = 1;
    
    // Key actions
    static const int PRESS = 1;
    static const int RELEASE = 0;
    
    // Common keys
    static const int KEY_W = 87;
    static const int KEY_A = 65;
    static const int KEY_S = 83;
    static const int KEY_D = 68;
    static const int KEY_C = 67;
    static const int KEY_R = 82;
    static const int KEY_ESCAPE = 256;
    
    // Arrow keys
    static const int KEY_LEFT = 263;
    static const int KEY_RIGHT = 262;
} 