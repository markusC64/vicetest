/** \file   settings_keyboard.c
 * \brief   GTK3 keyboard settings main widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES KbdStatusbar    all
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


#include "vice.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#include "kbdlayoutwidget.h"
#include "kbdmappingwidget.h"
#include "keyboard.h"
#include "keymap.h"
#include "lib.h"
#include "resources.h"
#include "settings_keyboard.h"
#include "ui.h"
#include "ui.h"
#include "uistatusbar.h"
#include "util.h"
#include "vice_gtk3.h"
#include "vsync.h"


/** \brief  Toggle statusbar keyboard debugging widget display
 *
 *
 * \param[in]   widget  toggle button
 * \param[in]   data    extra event data (unused)
 */
static void on_kbd_debug_toggled(GtkWidget *widget, gpointer data)
{
    ui_statusbar_set_kbd_debug(
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}


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
        char *path;
        int oops;


        /* `filename` is owned by GLib */
        path = util_add_extension_const(filename, "vkm");

        oops = keyboard_keymap_dump(path);
        if (oops == 0) {
            vice_gtk3_message_info(
                    "Succesfully saved current keymap",
                    "Wrote current keymap as '%s'.", filename);
        } else {
            vice_gtk3_message_error("Failed to save custom keymap",
                    "Error %d: %s", errno, strerror(errno));
        }
        g_free(filename);
        lib_free(path);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}


/** \brief  Write current keymap to host system
 *
 * \param[in]   widget  button triggering the event (ignored)
 * \param[in]   data    extra event data (ignored)
 */
static void on_save_custom_keymap_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;

    dialog = vice_gtk3_save_file_dialog(
            "Save current keymap",  /* title */
            NULL,                   /* proposed filename: might use this later */
            TRUE,                   /* query user before overwrite */
            NULL,                   /* base path, maybe ~ ?) */
            save_filename_callback, /* filename callback */
            NULL                    /* extra data */
         );
    gtk_widget_show(dialog);
}


/** \brief  Create keyboard settings widget
 *
 * \param[in]   widget  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_keyboard_widget_create(GtkWidget *widget)
{
    GtkWidget *layout;
    GtkWidget *mapping_widget;
    GtkWidget *layout_widget;
    GtkWidget *kbdstatusbar;
    GtkWidget *custom_button;

    layout = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    mapping_widget = kbdmapping_widget_create(widget);
    gtk_grid_attach(GTK_GRID(layout), mapping_widget, 0, 0, 1, 1);

    layout_widget = kbdlayout_widget_create();
    gtk_widget_set_margin_top(layout_widget, 32);

    gtk_grid_attach(GTK_GRID(layout), layout_widget, 0, 1, 1, 1);

    /* Add button to save current (perhaps altered) keymap */
    custom_button = gtk_button_new_with_label("Save current keymap");
    gtk_widget_set_margin_top(custom_button, 16);
    g_signal_connect(custom_button, "clicked",
            G_CALLBACK(on_save_custom_keymap_clicked), NULL);
    gtk_grid_attach(GTK_GRID(layout), custom_button, 0, 2, 1, 1);

    kbdstatusbar = vice_gtk3_resource_check_button_new("KbdStatusbar",
            "Enable keyboard debugging on statusbar");
    gtk_widget_set_margin_top(kbdstatusbar, 16);
    gtk_grid_attach(GTK_GRID(layout), kbdstatusbar, 0, 3, 1, 1);
    g_signal_connect(kbdstatusbar, "toggled", G_CALLBACK(on_kbd_debug_toggled),
            NULL);
    gtk_widget_show_all(layout);

    /* update widget so sym/pos is greyed out correctly */
    kbdmapping_widget_update();

    return layout;
}
