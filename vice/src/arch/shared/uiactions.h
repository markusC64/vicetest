/** \file   uiactions.h
 * \brief   UI action names and descriptions - header
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 */

#ifndef VICE_UIACTIONS_H
#define VICE_UIACTIONS_H

#include <stdbool.h>
#include <stddef.h>
/* this header is required if the macro IS_ACTION_NAME_CHAR() is used: */
#include <ctype.h>

/* for the VICE_MACHINE_* masks */
#include "machine.h"


/** \brief  Mapping of action names to descriptions
 */
typedef struct ui_action_info_s {
    int id;             /**< action ID */
    const char *name;   /**< action name */
    const char *desc;   /**< action description */
} ui_action_info_t;


/** \brief  Check for valid action name character
 *
 * Check if \a ch is a valid character in an action name.
 *
 * Supported characters are:
 * * a-z
 * * A-Z
 * * 0-9
 * * '_', '-' and ':'
 */
#define IS_ACTION_NAME_CHAR(ch) \
    (isalpha(ch) || isdigit(ch) || ch == '_' || ch == '-' || ch == ':')


/** \brief  IDs for the UI actions
 *
 * These IDs are used to refer to specific UI actions/dialogs.
 */
enum {
    ACTION_INVALID = -1,
    ACTION_NONE = 0,
    ACTION_ADVANCE_FRAME,
    ACTION_CART_ATTACH,
    ACTION_CART_DETACH,
    ACTION_CART_FREEZE,
    ACTION_DEBUG_AUTOPLAYBACK_FRAMES,
    ACTION_DEBUG_BLITTER_LOG_TOGGLE,
    ACTION_DEBUG_CORE_DUMP_TOGGLE,
    ACTION_DEBUG_DMA_LOG_TOGGLE,
    ACTION_DEBUG_FLASH_LOG_TOGGLE,
    ACTION_DEBUG_TRACE_CPU_TOGGLE,
    ACTION_DEBUG_TRACE_DRIVE_10_TOGGLE,
    ACTION_DEBUG_TRACE_DRIVE_11_TOGGLE,
    ACTION_DEBUG_TRACE_DRIVE_8_TOGGLE,
    ACTION_DEBUG_TRACE_DRIVE_9_TOGGLE,
    ACTION_DEBUG_TRACE_IEC_TOGGLE,
    ACTION_DEBUG_TRACE_IEEE488_TOGGLE,
    ACTION_DEBUG_TRACE_MODE,
    ACTION_DRIVE_ATTACH_10_0,
    ACTION_DRIVE_ATTACH_10_1,
    ACTION_DRIVE_ATTACH_11_0,
    ACTION_DRIVE_ATTACH_11_1,
    ACTION_DRIVE_ATTACH_8_0,
    ACTION_DRIVE_ATTACH_8_1,
    ACTION_DRIVE_ATTACH_9_0,
    ACTION_DRIVE_ATTACH_9_1,
    ACTION_DRIVE_CREATE,
    ACTION_DRIVE_DETACH_10_0,
    ACTION_DRIVE_DETACH_10_1,
    ACTION_DRIVE_DETACH_11_0,
    ACTION_DRIVE_DETACH_11_1,
    ACTION_DRIVE_DETACH_8_0,
    ACTION_DRIVE_DETACH_8_1,
    ACTION_DRIVE_DETACH_9_0,
    ACTION_DRIVE_DETACH_9_1,
    ACTION_DRIVE_DETACH_ALL,
    ACTION_EDIT_COPY,
    ACTION_EDIT_PASTE,
    ACTION_FLIPLIST_ADD,
    ACTION_FLIPLIST_CLEAR,
    ACTION_FLIPLIST_LOAD,
    ACTION_FLIPLIST_NEXT,
    ACTION_FLIPLIST_PREVIOUS,
    ACTION_FLIPLIST_REMOVE,
    ACTION_FLIPLIST_SAVE,
    ACTION_FULLSCREEN_DECORATIONS_TOGGLE,
    ACTION_FULLSCREEN_TOGGLE,
    ACTION_HELP_ABOUT,
    ACTION_HELP_COMMAND_LINE,
    ACTION_HELP_COMPILE_TIME,
    ACTION_HELP_HOTKEYS,
    ACTION_HELP_MANUAL,
    ACTION_HISTORY_MILESTONE_RESET,
    ACTION_HISTORY_MILESTONE_SET,
    ACTION_HISTORY_PLAYBACK_START,
    ACTION_HISTORY_PLAYBACK_STOP,
    ACTION_HISTORY_RECORD_START,
    ACTION_HISTORY_RECORD_STOP,
    ACTION_KEYSET_JOYSTICK_TOGGLE,
    ACTION_MEDIA_RECORD,
    ACTION_MEDIA_STOP,
    ACTION_MONITOR_OPEN,
    ACTION_MOUSE_GRAB_TOGGLE,
    ACTION_PAUSE_TOGGLE,
    ACTION_QUIT,
    ACTION_RESET_DRIVE_10,
    ACTION_RESET_DRIVE_11,
    ACTION_RESET_DRIVE_8,
    ACTION_RESET_DRIVE_9,
    ACTION_RESET_HARD,
    ACTION_RESET_SOFT,
    ACTION_RESTORE_DISPLAY,
    ACTION_SCREENSHOT_QUICKSAVE,
    ACTION_SETTINGS_DEFAULT,
    ACTION_SETTINGS_DIALOG,
    ACTION_SETTINGS_LOAD_EXTRA,
    ACTION_SETTINGS_LOAD_FROM,
    ACTION_SETTINGS_LOAD,
    ACTION_SETTINGS_SAVE,
    ACTION_SETTINGS_SAVE_TO,
    /* TODO: Add ACTION_SHOW_STATUSBAR_VDC_TOGGLE or something for x128 */
    ACTION_SHOW_STATUSBAR_TOGGLE,
    ACTION_SMART_ATTACH,
    ACTION_SNAPSHOT_LOAD,
    ACTION_SNAPSHOT_QUICKLOAD,
    ACTION_SNAPSHOT_QUICKSAVE,
    ACTION_SNAPSHOT_SAVE,
    ACTION_SPEED_CPU_100,
    ACTION_SPEED_CPU_10,
    ACTION_SPEED_CPU_200,
    ACTION_SPEED_CPU_20,
    ACTION_SPEED_CPU_50,
    ACTION_SPEED_CPU_CUSTOM,
    ACTION_SPEED_FPS_50,
    ACTION_SPEED_FPS_60,
    ACTION_SPEED_FPS_CUSTOM,
    ACTION_SPEED_FPS_REAL,
    ACTION_SWAP_CONTROLPORT_TOGGLE,
    ACTION_TAPE_ATTACH_1,
    ACTION_TAPE_ATTACH_2,
    ACTION_TAPE_CREATE_1,
    ACTION_TAPE_CREATE_2,
    ACTION_TAPE_DETACH_1,
    ACTION_TAPE_DETACH_2,
    ACTION_TAPE_FFWD_1,
    ACTION_TAPE_FFWD_2,
    ACTION_TAPE_PLAY_1,
    ACTION_TAPE_PLAY_2,
    ACTION_TAPE_RECORD_1,
    ACTION_TAPE_RECORD_2,
    ACTION_TAPE_RESET_1,
    ACTION_TAPE_RESET_2,
    ACTION_TAPE_RESET_COUNTER_1,
    ACTION_TAPE_RESET_COUNTER_2,
    ACTION_TAPE_REWIND_1,
    ACTION_TAPE_REWIND_2,
    ACTION_TAPE_STOP_1,
    ACTION_TAPE_STOP_2,
    ACTION_WARP_MODE_TOGGLE,

    /* VSID actions */
    ACTION_PSID_LOAD,
    ACTION_PSID_OVERRIDE_TOGGLE,
    ACTION_PSID_SUBTUNE_1,
    ACTION_PSID_SUBTUNE_2,
    ACTION_PSID_SUBTUNE_3,
    ACTION_PSID_SUBTUNE_4,
    ACTION_PSID_SUBTUNE_5,
    ACTION_PSID_SUBTUNE_6,
    ACTION_PSID_SUBTUNE_7,
    ACTION_PSID_SUBTUNE_8,
    ACTION_PSID_SUBTUNE_9,
    ACTION_PSID_SUBTUNE_10,

    ACTION_ID_COUNT     /**< number of action IDs */
};


int                 ui_action_get_id(const char *name);
const char *        ui_action_get_name(int id);
const char *        ui_action_get_desc(int id);
ui_action_info_t *  ui_action_get_info_list(void);

/* TODO: implement the following: */
bool                ui_action_def(int id, const char *hotkey);
bool                ui_action_undef(int id);
bool                ui_action_redef(int id, const char *hotkey);

#endif
