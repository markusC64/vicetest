/** \file   settings_ramlink.c
 * \brief   Settings widget to control RAMLink resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES RAMLINK             x64 x64sc xscpu64 x128
 * $VICERES RAMLINKfilename     x64 x64sc xscpu64 x128
 * $VICERES RAMLINKImageWrite   x64 x64sc xscpu64 x128
 * $VICERES RAMLINKsize         x64 x64sc xscpu64 x128
 * $VICERES RAMLINKmode         x64 x64sc xscpu64 x128
 * $VICERES RAMLINKRTCSave      x64 x64sc xscpu64 x128
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

#include "carthelpers.h"
#include "cartridge.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "vice_gtk3.h"

#include "settings_ramlink.h"


/** \brief  RAMLINK modes
 */
static const vice_gtk3_radiogroup_entry_t ramlink_modes[] = {
    { "Normal",     1 },
    { "Disable",    0 },
    { NULL,         -1 }
};


/** \brief  Create RAMLink widget
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_ramlink_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *enable;
    GtkWidget *rtc_save;
    GtkWidget *mode;
    GtkWidget *size;
    GtkWidget *cart_widget;
    GtkWidget *rom_widget;

    /* use three columns for the label */
    grid = vice_gtk3_grid_new_spaced_with_label(-1, -1, CARTRIDGE_NAME_RAMLINK " settings", 2);

    /* create 'enable ramlink' checkbox */
    enable = vice_gtk3_resource_check_button_new("RAMLINK", "Enable " CARTRIDGE_NAME_RAMLINK);
    gtk_widget_set_margin_start(enable, 16);
    gtk_grid_attach(GTK_GRID(grid), enable, 0, 1, 1, 1);

    /* create 'RTC Save' checkbox */
    rtc_save = vice_gtk3_resource_check_button_new("RAMLINKRTCSave", "RTC Save");
    gtk_widget_set_margin_start(rtc_save, 16);
    gtk_grid_attach(GTK_GRID(grid), rtc_save, 0, 2, 1, 1);

    /* create mode widget */
    mode = vice_gtk3_resource_radiogroup_new(
            "RAMLINKmode",
            ramlink_modes,
            GTK_ORIENTATION_HORIZONTAL);
    /* create mode label */
    label = gtk_label_new("Mode");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_grid_attach(GTK_GRID(grid), label, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), mode, 3, 2, 1, 1);

    /* create size widget */
    size = vice_gtk3_resource_spin_int_new(
            "RAMLINKsize",
            0, 16, 1);
    label = gtk_label_new("Size (MiB)");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_grid_attach(GTK_GRID(grid), label, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), size, 3, 1, 1, 1);
    /* fix size of the spin button */
    gtk_widget_set_hexpand(size, FALSE);
    gtk_widget_set_halign(size, GTK_ALIGN_START);

    /* ROM image browser widget */
    rom_widget = vice_gtk3_resource_browser_new(
            "RAMLINKBIOSfilename",
            NULL,
            NULL,
            "Select Ramlink ROM image",
            CARTRIDGE_NAME_RAMLINK " ROM",
            NULL);
    gtk_grid_attach(GTK_GRID(grid), rom_widget, 0, 4, 4, 1);

    /* create RAMlink image browser */
    cart_widget = cart_image_widget_create(
            parent,
            CARTRIDGE_NAME_RAMLINK " RAM Image",
            "RAMLINKfilename", "RAMLINKImageWrite",
            CARTRIDGE_NAME_RAMLINK, CARTRIDGE_RAMLINK);

    gtk_grid_attach(GTK_GRID(grid), cart_widget, 0, 5, 4, 1);
    gtk_widget_show_all(grid);
    return grid;
}
