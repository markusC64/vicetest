/** \file   videopalettewidget.c
 * \brief   Widget to select palette
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * FIXME:   Note sure x64dtv here:
 *
 * $VICERES CrtcPaletteFile         xpet xcbm2
 * $VICERES CrtcExternalPalette     xpet xcbm2
 * $VICEREs TEDPaletteFile          xplus4
 * $VICERES TEDExternalPalette      xplu4
 * $VICERES VDCPaletteFile          x128
 * $VICERES VDCExternalPalette      x128
 * $VICERES VICPaletteFile          xvic
 * $VICERES VICExternalPalette      xvic
 * $VICERES VICIIPaletteFile        x64 x64sc xscpu64 xd64tv xcbm5x0
 * $VICERES VICIIExternalPalette    x64 x64sc xscpu64 xd64tv xcbm5x0
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
#include <string.h>

#include "vice_gtk3.h"
#include "debug_gtk3.h"
#include "palette.h"
#include "resources.h"
#include "ui.h"

#include "videopalettewidget.h"


/** \brief  video chip prefix, used to construct proper resource names
 */
static const char *chip_prefix = NULL;

/** \brief  Internal palette radio button */
static GtkWidget *radio_internal = NULL;

/** \brief  External palette radio button */
static GtkWidget *radio_external = NULL;

/** \brief  External palette combo box
 */
static GtkWidget *combo_external = NULL;

/** \brief  Browse button for custom palette files
 *
 * TODO:    Replace with resourcebrowser.c
 */
static GtkWidget *button_custom = NULL;


/** \brief  Handler for the "toggled" events of the internal/external radios
 *
 * \param[in]   radio   Internal/External radio button
 * \param[in]   data    setting (bool)
 */
static void on_internal_toggled(GtkWidget *radio, gpointer data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
        resources_set_int_sprintf("%sExternalPalette", GPOINTER_TO_INT(data),
                chip_prefix);
    }
}


/** \brief  Handler for the "changed" event of the palettes combo box
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   extra event data (unused)
 */
static void on_combo_changed(GtkComboBox *combo, gpointer user_data)
{
    const char *id = gtk_combo_box_get_active_id(combo);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_external), TRUE);
    resources_set_string_sprintf("%sPaletteFile", id, chip_prefix);
    resources_set_int_sprintf("%sExternalPalette", 1, chip_prefix);
}


/** \brief  Callback for the custom palette file chooser
 *
 * \param[in,out]   dialog      file chooser dialog
 * \param[in,out]   filename    palette filename (NULL to cancel)
 * \param[in]       data        extra data (unused)
 */
static void browse_filename_callback(GtkDialog *dialog,
                                     gchar *filename,
                                     gpointer data)
{
    if (filename != NULL) {
        resources_set_string_sprintf("%sPaletteFile", filename, chip_prefix);

        /* add to combo box */
        gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_external), 0,
                filename, filename);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_external), 0);
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}


/** \brief  Handler for the "clicked" event of the "browse ..." button
 *
 * Opens a custom palette file and adds the file name to the combo box.
 *
 * \param[in]   button      browse button (unused)
 * \param[in]   user_data   extra event data (unused)
 */
static void on_browse_clicked(GtkButton *button, gpointer user_data)
{
    const char *flist[] = { "*.vpl", NULL };
    GtkWidget *dialog;

    dialog = vice_gtk3_open_file_dialog(
            "Open palette file",
            "Palette files", flist, NULL,
            browse_filename_callback,
            NULL);
    gtk_widget_show(dialog);
}


/** \brief  Create combo box with available palettes for the current chip
 *
 * If the file in the resource "${chip}PaletteFile" doesn't match any of the
 * palette files provided by VICE, it is assumed to be a user-defined custom
 * palette and inserted at the top of the combo box.
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_combo_box(void)
{
    GtkWidget *combo;
    int index;
    const palette_info_t *list;
    int row;
    const char *current;
    gboolean found = FALSE;

    if (resources_get_string_sprintf("%sPaletteFile",
                &current, chip_prefix) < 0) {
        current = NULL;
    }

    row = 0;
    list = palette_get_info_list();
    combo = gtk_combo_box_text_new();
    for (index = 0; list[index].chip != NULL; index++) {
        if (strcmp(list[index].chip, chip_prefix) == 0) {
            /* got a valid entry */
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                    list[index].file, list[index].name);
            if (current != NULL && strcmp(list[index].file, current) == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), row);
                found = TRUE;
            }
            row++;
        }
    }

    /* if we didn't find `current` in the list of VICE palette files, add it as
     * a custom user-defined file */
    if ((!found) && current != NULL) {
        gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, current, current);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    }

    g_signal_connect(combo, "changed", G_CALLBACK(on_combo_changed), NULL);
    return combo;
}


/** \brief  Create "browse ..." button to select a palette file
 *
 * \return  GtkButton
 */
static GtkWidget *create_browse_button(void)
{
    GtkWidget *button;

    button = gtk_button_new_with_label("Browse ...");
    g_signal_connect(button, "clicked", G_CALLBACK(on_browse_clicked), NULL);
    return button;
}


/** \brief  Create video palette widget
 *
 * \param[in]   chip    chip name (used as prefix for resources)
 *
 * \return  GtkGrid
 */
GtkWidget *video_palette_widget_create(const char *chip)
{
    GtkWidget *grid;
    GSList *group = NULL;
    int external;

    chip_prefix = chip;

    resources_get_int_sprintf("%sExternalPalette", &external, chip);

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT,
            "Palette settings", 4);

    radio_internal = gtk_radio_button_new_with_label(group, "Internal");
    gtk_widget_set_margin_start(radio_internal, 16);
    radio_external = gtk_radio_button_new_with_label(group, "External");
    gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_external),
                                GTK_RADIO_BUTTON(radio_internal));

    combo_external = create_combo_box();
    gtk_widget_set_hexpand(combo_external, TRUE);
    button_custom = create_browse_button();

    gtk_grid_attach(GTK_GRID(grid), radio_internal, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio_external, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo_external, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_custom, 3, 1, 1, 1);

    if (external) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_external), TRUE);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_internal), TRUE);
    }

    g_signal_connect(radio_internal, "toggled",
            G_CALLBACK(on_internal_toggled), GINT_TO_POINTER(0));
     g_signal_connect(radio_external, "toggled",
            G_CALLBACK(on_internal_toggled), GINT_TO_POINTER(1));

    gtk_widget_show_all(grid);
    return grid;
}
