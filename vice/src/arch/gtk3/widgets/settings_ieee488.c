/** \file   settings_ieee488.c
 * \brief   Setting widget for IEEE-488 adapter
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES IEEE488         x64 x64sc xscpu64 x128 xvic
 * $VICERES IEEE488Image    x64 x64sc xscpu64 x128
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

#include "vice_gtk3.h"
#include "cartridge.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "ui.h"

#include "settings_ieee488.h"


/** \brief  Reference to the entry widget used for the IEEE-488 ROM
 *
 * TODO:    Check if we can refactor this code to use a 'base widget'
 */
static GtkWidget *entry_widget;


/** \brief  Handler for the 'toggled' event of the 'enable' check button
 *
 * Toggles the 'enabled' state of the IEEE-488 adapter/cart, but only if an
 * EEPROM image has been specified, otherwise when trying to set the check
 * button to 'true', an error message is displayed and the check button is
 * reverted to 'off'.
 *
 * \param[in,out]   widget  check button
 * \param[in]       data    unused
 */
static void on_enable_toggled(GtkWidget *widget, gpointer data)
{
    int state;
    const char *image;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (state) {
        if (resources_get_string("IEEE488Image", &image) < 0) {
            image = NULL;
        }

        if (image == NULL || *image == '\0') {
            /* no image */
            vice_gtk3_message_error("VICE core",
                    "Cannot enable IEEE-488 adapter, no image specified.");
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
            state = 0;
        }
    }

    if (state) {
        if (cartridge_enable(CARTRIDGE_IEEE488) < 0) {
            log_error(LOG_ERR, "failed to enable IEEE488 cartridge.");
        }
    } else {
        if (cartridge_disable(CARTRIDGE_IEEE488) < 0) {
            log_error(LOG_ERR, "failed to disable IEEE488 cartridge.");
        }
    }
}


/** \brief  Callback for the open-file dialog
 *
 * \param[in,out]   dialog      open-file dialog
 * \param[in]       filename    filename or NULL on cancel
 * \param[in]       data        extra data (unused)
 */
static void browse_filename_callback(GtkDialog *dialog,
                                     gchar *filename,
                                     gpointer data)
{
    if (filename != NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry_widget), filename);

        /* required, since setting the text of the entry doesn't trigger an
         * update of the connected resource (it only responds to focus-out and
         * pressing 'Enter' */
        if (resources_set_string("IEEE488Image", filename) < 0) {
            vice_gtk3_message_error("VICE core",
                    "Failed to set '%s' as IEEE-488 EEPROM image.",
                    filename);
        }
        g_free(filename);
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}


/** \brief  Handler for the "clicked" event of the browse button
 *
 * Activates a file-open dialog and stores the file name in the GtkEntry passed
 * in \a user_data if valid, triggering a resource update.
 *
 * \param[in]   widget      button
 * \param[in]   user_data   entry to store filename in
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    vice_gtk3_open_file_dialog(
            "Open IEEE-488 image",
            NULL, NULL, NULL,
            browse_filename_callback,
            NULL);
}


/** \brief  Create widget to control IEEE-488 adapter
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_ieee488_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable_widget;
    GtkWidget *label;
    GtkWidget *browse;

    const char *image;
    int enable_state;

    if (resources_get_string("IEEE488Image", &image) < 0) {
        image = NULL;
    }
    enable_state = cartridge_type_enabled(CARTRIDGE_IEEE488);

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    /* we can't use a `resource_check_button` here, since toggling the resource
     * depends on whether an image file is specified
     */
    enable_widget = gtk_check_button_new_with_label("Enable IEEE-488 interface");
    gtk_grid_attach(GTK_GRID(grid), enable_widget, 0, 0, 3, 1);

    /* only set state to true if both the state is true and an image is given */
    if (enable_state && (image != NULL && *image != '\0')) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enable_widget), TRUE);
    }

    label = gtk_label_new("IEEE-488 image");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    entry_widget = vice_gtk3_resource_entry_full_new("IEEE488Image");
    gtk_widget_set_hexpand(entry_widget, TRUE);
    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked), NULL);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_widget, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    g_signal_connect(enable_widget, "toggled", G_CALLBACK(on_enable_toggled), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
