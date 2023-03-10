/** \file   settings_rrnetmk3.c
 * \brief   Settings widget to control RRNet MK3 resourcs
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * \todo    Reimplement using cartimagehelper.c
 */

/*
 * $VICERES RRNETMK3_flashjumper    x64 x64sc xscpu64 x128
 * $VICERES RRNETMK3_bios_write     x64 x64sc xscpu64 x128
 */


/*
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

#include "vice.h"
#include <gtk/gtk.h>
#include <stdlib.h>

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "savefiledialog.h"
#include "cartridge.h"
#include "carthelpers.h"

#include "settings_rrnetmk3.h"


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
        if (cartridge_save_image(CARTRIDGE_RRNETMK3, filename) < 0) {
            vice_gtk3_message_error("Saving failed",
                    "Failed to save cartridge image '%s'",
                    filename);
        }
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}


/** \brief  Handler for the "clicked" event of the "Save As" button
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


/** \brief  Handler for the "clicked" event of the "Flush now" button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    if (cartridge_flush_image(CARTRIDGE_RRNETMK3) < 0) {
        vice_gtk3_message_error("VICE core",
                "Failed to flush RR-Net Mk3 image.");
    }
}


/** \brief  Create widget to control RRNet Mk3 resources
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_rrnetmk3_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *flash_jumper;
    GtkWidget *bios_write;
    GtkWidget *save_button;
    GtkWidget *flush_button;

    grid = vice_gtk3_grid_new_spaced(8, 8);

    flash_jumper = vice_gtk3_resource_check_button_new(
            "RRNETMK3_flashjumper", "Enable flash jumper");
    gtk_grid_attach(GTK_GRID(grid), flash_jumper, 0, 0, 1, 1);

    /* RRBiosWrite */
    bios_write = vice_gtk3_resource_check_button_new("RRNETMK3_bios_write",
            "Write back RRNetMk3 Flash ROM image automatically");
    /* That's a very long sentence, I18N will enjoy this*/
    gtk_grid_attach(GTK_GRID(grid), bios_write, 0, 1, 1, 1);

    /* Save image as... */
    save_button = gtk_button_new_with_label("Save image as ...");
    gtk_grid_attach(GTK_GRID(grid), save_button, 1, 1, 1, 1);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked),
            NULL);

    /* Flush image now */
    flush_button = gtk_button_new_with_label("Save image now");
    gtk_grid_attach(GTK_GRID(grid), flush_button, 1, 2, 1, 1);
    g_signal_connect(flush_button, "clicked", G_CALLBACK(on_flush_clicked),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}
