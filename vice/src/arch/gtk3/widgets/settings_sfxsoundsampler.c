/** \file   settings_sfxsoundsampler.c
 * \brief   Settings widget controlling SFX Sound Sampler resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SFXSoundSampler         x64 x64sc xscpu64 x128 xvic
 * $VICERES SFXSoundSamplerIOSwap   xvic
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
#include <gtk/gtk.h>

#include "lib.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "machine.h"
#include "vice_gtk3.h"

#include "settings_sfxsoundsampler.h"


/** \brief  I/O swap toggle button */
static GtkWidget *io_swap = NULL;


/** \brief  Handler for the "toggled" event of the Enable check button
 *
 * \param[in]   widget      check button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_enable_toggled(GtkWidget *widget, gpointer user_data)
{
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(io_swap, state);
}


/** \brief  Create SFX Sound Sampler widget (VIC-20)
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_sfxsoundsampler_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    enable = vice_gtk3_resource_check_button_new("SFXSoundSampler",
            "Enable SFX Sound Sampler");
    gtk_grid_attach(GTK_GRID(grid), enable, 0, 0, 1, 1);

    if (machine_class == VICE_MACHINE_VIC20) {
        io_swap = vice_gtk3_resource_check_button_new(
                "SFXSoundSamplerIOSwap", "Enable MasC=uerade I/O swap");
        gtk_widget_set_margin_start(io_swap, 16);
        gtk_grid_attach(GTK_GRID(grid), io_swap, 0, 1, 1, 1);

        g_signal_connect(enable, "toggled", G_CALLBACK(on_enable_toggled), NULL);

        on_enable_toggled(enable, NULL);
    }

    gtk_widget_show_all(grid);
    return grid;
}
