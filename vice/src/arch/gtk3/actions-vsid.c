/** \file   actions-vsid.c
 * \brief   UI action implementations for VSID
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * \@note   This file cannot be used from ui.c since that causes massive
 *          linker errors due to the way vsid is linked. Currently registering
 *          the actions happens in vsidui.c, which magically does work.
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

/* Resources altered by this file:
 *
 */

#include "vice.h"

#include <gtk/gtk.h>
#include <stddef.h>
#include <stdbool.h>

#include "debug_gtk3.h"
#include "hotkeymap.h"
#include "machine.h"
#include "psid.h"
#include "resources.h"
#include "ui.h"
#include "uiactions.h"
#include "uisidattach.h"
#include "vsidcontrolwidget.h"
#include "vsidstate.h"

#include "actions-vsid.h"


/** \brief  Emulation speed during fast forward
 */
#define FFWD_SPEED  500


/** \brief  Trigger play of current tune
 *
 * Helper to (re)start playback of current selected subtune
 */
static void play_current_tune(void)
{
    vsid_state_t *state;
    int current;
    const char *filename;

    state = vsid_state_lock();
    current = state->tune_current;
    filename = state->psid_filename;

    if (state->tune_current < 0) {
        if (state->tune_previous < 0) {
            state->tune_previous = state->tune_default;
        }
        current = state->tune_current = state->tune_default;
    }
#ifdef HAVE_DEBUG_GTK3UI
    int count = state->tune_count;
    int def = state->tune_default;
    int prev = state->tune_previous;
#endif
    vsid_state_unlock();

    /* reload unloaded PSID file if loaded before */
    if (filename != NULL) {
        psid_load_file(filename);
    }

    resources_set_int("Speed", 100);
    debug_gtk3("current: %d, previous: %d  (total: %d, default: %d)",
                current, prev, count, def);
    debug_gtk3("calling psid_init_driver().");
    psid_init_driver();
    debug_gtk3("calling machine_play_psid(%d).", current);
    machine_play_psid(current);
    debug_gtk3("calling machine_trigger_reset(SOFT).");
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    ui_pause_disable();
    vsid_control_widget_set_state(VSID_PLAYING);
}

/** \brief  Play subtune
 *
 * \param[in]   tune    subtune number
 */
static void play_subtune(int tune)
{
    vsid_state_t *state = vsid_state_lock();
    int count = state->tune_count;

    if (tune < 1) {
        tune = 1;
    } else if (tune > count) {
        tune = count;
    }
    state->tune_previous = state->tune_current;
    state->tune_current = tune;
    vsid_state_unlock();

    play_current_tune();
}

/** \brief  Show PSID load dialog */
static void psid_load_action(void)
{
    uisidattach_show_dialog();
}

/** \brief  Toggle override of PSID file settings */
static void psid_override_toggle_action(void)
{
    int enabled = 0;

    resources_get_int("PSIDKeepEnv", &enabled);
    enabled = !enabled;
    resources_set_int("PSIDKeepEnv", enabled);
    ui_set_check_menu_item_blocked_by_action(ACTION_PSID_OVERRIDE_TOGGLE, enabled);
}

/** \brief  Start playback */
static void psid_play_action(void)
{
    vsid_state_t *state;
    int current;
    const char *filename;

    state = vsid_state_lock();
    current = state->tune_current;
    filename = state->psid_filename;

    if (current < 0) {
        /* restart previous tune if stopped before */
        if (state->tune_previous >= 1) {
            current = state->tune_current = state->tune_previous;
        } else {
            /* haven't stopped before resuming, use default tune */
            current = state->tune_current = state->tune_default;
        }
        vsid_state_unlock();

        /* reload unloaded PSID file if loaded before */
        if (filename != NULL) {
            psid_load_file(filename);
        }

        psid_init_driver();
        machine_play_psid(current);
        machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    } else {
        /* return emulation speed back to 100% */
        vsid_state_unlock();
        resources_set_int("Speed", 100);
    }
    ui_pause_disable();
    vsid_control_widget_set_state(VSID_PLAYING);
}

/** \brief  Toggle pause */
static void psid_pause_action(void)
{
    ui_pause_toggle();
    vsid_control_widget_set_state(ui_pause_active() ? VSID_PAUSED : VSID_PLAYING);
}

/** \brief  Stop playback */
static void psid_stop_action(void)
{
    vsid_state_t *state = vsid_state_lock();

    /* remember which tune we were playing so we can resume later */
    state->tune_previous = state->tune_current;
    state->tune_current = -1;
    vsid_state_unlock();

    psid_init_driver();
    machine_play_psid(-1);
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    vsid_control_widget_set_state(VSID_STOPPED);
}

/** \brief  Toggle fast forward */
static void psid_ffwd_action(void)
{
    int speed = 0;

    resources_get_int("Speed", &speed);
    if (speed == 100) {
        resources_set_int("Speed", FFWD_SPEED);
        vsid_control_widget_set_state(VSID_FORWARDING);
    } else {
        resources_set_int("Speed", 100);
        vsid_control_widget_set_state(VSID_PLAYING);
    }
    ui_pause_disable();
}

/** \brief  Play next subtune
 *  \note   Wraps around to the first tune if the current tune is the last.
 */
static void psid_subtune_next_action(void)
{
    vsid_state_t *state = vsid_state_lock();

    if (state->tune_current < 0) {
        state->tune_current = state->tune_previous;
    }

    if (state->tune_current >= state->tune_count || state->tune_current < 1) {
        state->tune_current = 1;
    } else {
        state->tune_current++;
    }
    vsid_state_unlock();
    play_current_tune();
}

/** \brief  Play previous subtune
 *  \note   Wraps around to the last tune if the current tune is the first.
 */
static void psid_subtune_previous_action(void)
{
    vsid_state_t *state = vsid_state_lock();

    if (state->tune_current < 0) {
        state->tune_current = state->tune_previous;
    }

    if (state->tune_current <= 1) {
        state->tune_current = state->tune_count;
    } else {
        state->tune_current--;
    }
    vsid_state_unlock();
    ui_pause_disable();
    play_current_tune();
}

/** \brief  Play subtune 1 */
static void psid_subtune_1_action(void)
{
    play_subtune(1);
}

/** \brief  Play subtune 2 */
static void psid_subtune_2_action(void)
{
    play_subtune(2);
}

/** \brief  Play subtune 3 */
static void psid_subtune_3_action(void)
{
    play_subtune(3);
}

/** \brief  Play subtune 4 */
static void psid_subtune_4_action(void)
{
    play_subtune(4);
}

/** \brief  Play subtune 5 */
static void psid_subtune_5_action(void)
{
    play_subtune(5);
}

/** \brief  Play subtune 6 */
static void psid_subtune_6_action(void)
{
    play_subtune(6);
}

/** \brief  Play subtune 7 */
static void psid_subtune_7_action(void)
{
    play_subtune(7);
}

/** \brief  Play subtune 8 */
static void psid_subtune_8_action(void)
{
    play_subtune(8);
}

/** \brief  Play subtune 9 */
static void psid_subtune_9_action(void)
{
    play_subtune(9);
}

/** \brief  Play subtune 10 */
static void psid_subtune_10_action(void)
{
    play_subtune(10);
}

static void psid_loop_toggle_action(void)
{
    gboolean enabled = vsid_control_widget_get_repeat();

    vsid_control_widget_set_repeat(!enabled);
}


/** \brief  List of VSID-specific actions */
static const ui_action_map_t vsid_actions[] = {
    {
        .action = ACTION_PSID_LOAD,
        .handler = psid_load_action,
        .blocks = true,
        .dialog = true
    },
    {
        .action = ACTION_PSID_OVERRIDE_TOGGLE,
        .handler = psid_override_toggle_action,
        .uithread = true,
    },
    {
        .action = ACTION_PSID_PLAY,
        .handler = psid_play_action,
        .uithread = true,   /* if we want to toggle the button's pressed state */
    },
    {
        .action = ACTION_PSID_PAUSE,
        .handler = psid_pause_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_STOP,
        .handler = psid_stop_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_FFWD,
        .handler = psid_ffwd_action,
        .uithread = true    /* if we want to toggle the button's pressed state */
    },
    {
        .action = ACTION_PSID_SUBTUNE_NEXT,
        .handler = psid_subtune_next_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_PREVIOUS,
        .handler = psid_subtune_previous_action,
        .uithread = true
    },

    {
        .action = ACTION_PSID_SUBTUNE_1,
        .handler = psid_subtune_1_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_2,
        .handler = psid_subtune_2_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_3,
        .handler = psid_subtune_3_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_4,
        .handler = psid_subtune_4_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_5,
        .handler = psid_subtune_5_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_6,
        .handler = psid_subtune_6_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_7,
        .handler = psid_subtune_7_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_8,
        .handler = psid_subtune_8_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_9,
        .handler = psid_subtune_9_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_SUBTUNE_10,
        .handler = psid_subtune_10_action,
        .uithread = true
    },
    {
        .action = ACTION_PSID_LOOP_TOGGLE,
        .handler = psid_loop_toggle_action,
        .uithread = true
    },

    UI_ACTION_MAP_TERMINATOR
};


/** \brief  Register VSID-specific actions */
void actions_vsid_register(void)
{
    ui_actions_register(vsid_actions);
}


/** \brief  Set initial UI element states for VSID
 */
void actions_vsid_setup_ui(void)
{
    int enabled = 0;

    /* Override PSID settings */
    resources_get_int("PSIDKeepEnv", &enabled);
    ui_set_check_menu_item_blocked_by_action(ACTION_PSID_OVERRIDE_TOGGLE, enabled);
}
