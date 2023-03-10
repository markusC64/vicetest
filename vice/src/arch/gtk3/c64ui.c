/** \file   c64ui.c
 * \brief   Native GTK3 C64 UI
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
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
 *
 */

#include "vice.h"

#include <stdio.h>

#include "debug_gtk3.h"
#include "c64model.h"
#include "crtcontrolwidget.h"
#include "machinemodelwidget.h"
#include "sampler.h"
#include "ui.h"
#include "uimachinewindow.h"
#include "vicii.h"
#include "videomodelwidget.h"
#include "widgethelpers.h"
#include "settings_model.h"

#include "clockportdevicewidget.h"
#include "clockport.h"

#include "reu.h"
#include "cartridge.h"
#include "machine.h"
#include "tapecart.h"
#include "crtpreviewwidget.h"

#include "c64ui.h"


/** \brief  List of C64 models
 *
 * Used in the machine-model widget
 */
static const char *c64_model_list[] = {
    "C64 PAL", "C64C PAL", "C64 old PAL",
    "C64 NTSC", "C64C NTSC", "C64 old NTSC",
    "Drean",
    "C64 SX PAL", "C64 SX NTSC",
    "Japanese", "C64 GS",
    "PET64 PAL", "PET64 NTSC",
    "MAX Machine", NULL
};


/** \brief  List of VIC-II models
 *
 * Used in the VIC-II model widget
 */
static const vice_gtk3_radiogroup_entry_t c64_vicii_models[] = {
    { "PAL",        MACHINE_SYNC_PAL },
    { "NTSC",       MACHINE_SYNC_NTSC },
    { "Old NTSC",   MACHINE_SYNC_NTSCOLD },
    { "PAL-N",      MACHINE_SYNC_PALN },
    { NULL, -1 }
};


/** \brief  Identify the canvas used to create a window
 *
 * \param[in]   canvas  video canvas
 *
 * \return  window index on success, -1 on failure
 */
static int identify_canvas(video_canvas_t *canvas)
{
    if (canvas != vicii_get_canvas()) {
        return -1;
    }

    return PRIMARY_WINDOW;
}

/** \brief  Create CRT controls widget for \a target window
 *
 * \param[in]   target_window   target window index
 *
 * \return  GtkGrid
 */
static GtkWidget *create_crt_widget(int target_window)
{
    return crt_control_widget_create(NULL, "VICII", TRUE);
}

/** \brief  Pre-initialize the UI before the canvas window gets created
 *
 * \return  0 on success, -1 on failure
 */
int c64ui_init_early(void)
{
    ui_machine_window_init();
    ui_set_identify_canvas_func(identify_canvas);
    ui_set_create_controls_widget_func(create_crt_widget);
    return 0;
}


/** \brief  Initialize the UI
 *
 * \return  0 on success, -1 on failure
 */
int c64ui_init(void)
{
    machine_model_widget_getter(c64model_get);
    machine_model_widget_setter(c64model_set);
    machine_model_widget_set_models(c64_model_list);

    video_model_widget_set_title("VIC-II model");
    video_model_widget_set_resource("MachineVideoStandard");
    video_model_widget_set_models(c64_vicii_models);

    /* set C64 model_get function */
    settings_model_widget_set_model_func(c64model_get);

    return 0;
}


/** \brief  Shut down the UI
 */
void c64ui_shutdown(void)
{
    /* NOP */
}
