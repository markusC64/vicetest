/** \file   settings_retroreplay.c
 * \brief   Settings widget to control Retro Replay resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * \todo    Check if the cartimagehelper can be used here to simplify the code
 *          for image selection and save/flush.
 */

/*
 * $VICERES RRFlashJumper   x64 x64sc xscpu64 x128
 * $VICERES RRBankJumper    x64 x64sc xscpu64/x128
 * $VICERES RRBiosWrite     x64 x64sc xscpu64/x128
 * $VICERES RRrevision      x64 x64sc xscpu64/x128
 * $VICERES RRClockPort     x64 x64sc xscpu64/x128
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

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "cartimagewidget.h"
#include "openfiledialog.h"
#include "savefiledialog.h"
#include "clockportdevicewidget.h"
#include "cartridge.h"
#include "carthelpers.h"

#include "settings_retroreplay.h"


/** \brief  List of Retro Replay revisions
 */
static const vice_gtk3_combo_entry_int_t rr_revisions[] = {
    { CARTRIDGE_NAME_RETRO_REPLAY,   0 },
    { CARTRIDGE_NAME_NORDIC_REPLAY,  1 },
    { NULL,                         -1 }
};


/** \brief  Callback for the save-dialog response handler
 *
 * \param[in,out]   dialog      save-file dialog
 * \param[in,out]   filename    filename
 * \param[in]       data        extra data (unused)
 */
static void save_filename_callback(GtkDialog *dialog,
                                   gchar *filename,
                                   gpointer data)
{
    if (filename != NULL) {
        if (cartridge_save_image(CARTRIDGE_RETRO_REPLAY, filename) < 0) {
            vice_gtk3_message_error("VICE core",
                    "Failed to save Retro Replay image '%s'.", filename);
        }
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}



/** \brief  Handler for the 'clicked' event of the "Save As" button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_save_clicked(GtkWidget *widget, gpointer user_data)
{
    vice_gtk3_save_file_dialog("Save image as",
                               NULL, TRUE, NULL,
                               save_filename_callback,
                               NULL);
}


/** \brief  Handler for the 'clicked' event of the "Flush now" button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    if (cartridge_flush_image(CARTRIDGE_RETRO_REPLAY) < 0) {
        vice_gtk3_message_error("VICE core",
                "Failed to flush current Retro Replay image.");
    }
}


/** \brief  Create widget to control Retro Replay resources
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_retroreplay_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *flash;
    GtkWidget *bank;
    GtkWidget *label;
    GtkWidget *rev_combo;
    GtkWidget *cp_combo;
    GtkWidget *save_button;
    GtkWidget *flush_button;
    GtkWidget *bios_write;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    /* RRFlashJumper */
    flash = vice_gtk3_resource_check_button_new("RRFlashJumper",
            "Enable flash jumper");
    gtk_grid_attach(GTK_GRID(grid), flash, 0, 0, 1, 1);

    /* RRBankJumper */
    bank = vice_gtk3_resource_check_button_new("RRBankJumper",
            "Enable bank jumper");
    gtk_grid_attach(GTK_GRID(grid), bank, 0, 1, 1, 1);

    /* RRrevision */
    label = gtk_label_new("Revision");
    gtk_widget_set_margin_start(label, 16);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
    rev_combo = vice_gtk3_resource_combo_box_int_new("RRrevision",
            rr_revisions);
    gtk_grid_attach(GTK_GRID(grid), rev_combo, 2, 0, 1, 1);

    /* RRClockPort */
    label = gtk_label_new("Clockport device");
    gtk_widget_set_margin_start(label, 16);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 1, 1, 1);
    cp_combo = clockport_device_widget_create("RRClockPort");
    gtk_grid_attach(GTK_GRID(grid), cp_combo, 2, 1, 1, 1);

    /* RRBiosWrite */
    bios_write = vice_gtk3_resource_check_button_new("RRBiosWrite",
            "Write back RR Flash ROM image automatically");
    gtk_grid_attach(GTK_GRID(grid), bios_write, 0, 2, 2, 1);

    /* Save image as... */
    save_button = gtk_button_new_with_label("Save image as ...");
    gtk_grid_attach(GTK_GRID(grid), save_button, 2, 2, 1, 1);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked),
            NULL);

    /* Flush image now */
    flush_button = gtk_button_new_with_label("Save image now");
    gtk_grid_attach(GTK_GRID(grid), flush_button, 2, 3, 1, 1);
    g_signal_connect(flush_button, "clicked", G_CALLBACK(on_flush_clicked),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}
