/** \file   resourcecombobox.c
 * \brief   Combo boxes connected to a resource
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
 *
 */

#include "vice.h"

#include <gtk/gtk.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "basewidget_types.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "resourcehelpers.h"
#include "resources.h"

#include "resourcecombobox.h"


/*
 * Combo box for integer resources
 *
 * Presents a combo box with strings and uses integer keys to control a resource
 */


/** \brief  Create a model for a combo box with int's as ID's
 *
 * \param[in]   list    list of options
 *
 * \return  model
 */
static GtkListStore *create_combo_int_model(const vice_gtk3_combo_entry_int_t *list)
{
    GtkListStore *model;
    GtkTreeIter iter;
    int i;

    model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    if (list == NULL) {
        return model;
    }
    for (i = 0; list[i].name != NULL; i++) {
        gtk_list_store_append(model, &iter);
        gtk_list_store_set(model, &iter,
                0, list[i].name,    /* item name */
                1, list[i].id,      /* item ID */
                -1);
    }
    return model;
}


/** \brief  Get current ID of \a combo
 *
 * \param[in]   combo   combo box
 * \param[out]  id      target of ID value
 *
 * \return  boolean
 *
 * \note    When this function returns `false`, the value in \a id is unchanged
 */
static gboolean get_combo_int_id(GtkComboBox *combo, int *id)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_combo_box_get_active(combo) >= 0) {
        model = gtk_combo_box_get_model(combo);
        if (gtk_combo_box_get_active_iter(combo, &iter)) {
            gtk_tree_model_get(model, &iter, 1, id, -1);
            return TRUE;
        }
    }
    return FALSE;
}


/** \brief  Set ID of \a combo to \a id
 *
 * \param[in,out]   combo   combo box
 * \param[in]       id      ID for \a combo
 *
 * \return  boolean
 */
static gboolean set_combo_int_id(GtkComboBox *combo, int id)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = gtk_combo_box_get_model(combo);
    if (gtk_tree_model_get_iter_first(model, &iter)) {
        do {
            int current;

            gtk_tree_model_get(model, &iter, 1, &current, -1);
            if (id == current) {
                gtk_combo_box_set_active_iter(combo, &iter);
                return TRUE;
            }
        } while (gtk_tree_model_iter_next(model, &iter));
    }
    return FALSE;
}


/** \brief  Handler for the "destroy" event of the integer combo box
 *
 * Frees the heap-allocated copy of the resource name
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   extra event data (unused)
 */
static void on_combo_int_destroy(GtkWidget *combo, gpointer user_data)
{
    resource_widget_free_resource_name(combo);
}


/** \brief  Handler for the "changed" event of the integer combo box
 *
 * Updates the resource connected to the combo box
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   extra event data (unused)
 */
static void on_combo_int_changed(GtkComboBox *combo, gpointer user_data)
{
    int id;
    const char *resource;

    resource = resource_widget_get_resource_name(GTK_WIDGET(combo));
    if (get_combo_int_id(combo, &id)) {
        if (resources_set_int(resource, id) < 0) {
            gulong handler_id;
            int prev;

            log_error(LOG_ERR, "failed to set resource '%s' to %d\n",
                    resource, id);

            /* set combo back to its previous (valid) id */
            prev = resource_widget_get_int(GTK_WIDGET(combo), "PreviousID");
            handler_id = GPOINTER_TO_ULONG(g_object_get_data(G_OBJECT(combo),
                                                             "ChangedHandlerID"));
            g_signal_handler_block(G_OBJECT(combo), handler_id);
            set_combo_int_id(combo, prev);
            g_signal_handler_unblock(G_OBJECT(combo), handler_id);

        } else {
            /* valid, so update the previous ID */
            resource_widget_set_int(GTK_WIDGET(combo), "PreviousID", id);
        }
    } else {
        log_error(LOG_ERR, "failed to get ID for resource '%s'\n", resource);
    }
}


/** \brief  Create a combo box to control an integer resource
 *
 * \param[in,out]   combo   combo box
 * \param[in]       entries list of entries for the combo box
 *
 * \return  GtkComboBox
 */
static GtkWidget *resource_combo_box_int_new_helper(
        GtkWidget *combo,
        const vice_gtk3_combo_entry_int_t *entries)
{
    GtkListStore *model;
    GtkCellRenderer *renderer;
    const char *resource;
    int current;
    gulong handler_id;

    /* setup combo box with model and renderers */
    model = create_combo_int_model(entries);
    gtk_combo_box_set_model(GTK_COMBO_BOX(combo), GTK_TREE_MODEL(model));
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
            "text", 0, NULL);

    /* set current ID */
    resource = resource_widget_get_resource_name(combo);
    if (resources_get_int(resource, &current) < 0) {
        /* couldn't read resource */
        log_error(LOG_ERR,
                "failed to get value for resource %s, "
                "reverting to the first entry\n",
                resource);
        current = 0;
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    } else {
        if (!set_combo_int_id(GTK_COMBO_BOX(combo), current)) {
            /* failed to set ID, revert to first entry */
            log_error(LOG_ERR,
                    "failed to set ID to %d for resource '%s',"
                    " reverting to the first entry\n",
                    current, resource);
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
        }
    }

    /* remember original value for reset() */
    resource_widget_set_int(combo, "ResourceOrig", current);
    /* used to reset combo box to its previous state if setting the resource
     * fails */
    resource_widget_set_int(combo, "PreviousID", current);

    /* register methods to be used by the resource widget manager */
    resource_widget_register_methods(
            combo,
            vice_gtk3_resource_combo_box_int_reset,
            vice_gtk3_resource_combo_box_int_factory,
            vice_gtk3_resource_combo_box_int_sync);

    /* connect signal handlers */
    handler_id = g_signal_connect(combo, "changed", G_CALLBACK(on_combo_int_changed), NULL);
    /* used to temporarily block the signal handler to avoid extra resource_set_int() calls */
    g_object_set_data(G_OBJECT(combo), "ChangedHandlerID", GULONG_TO_POINTER(handler_id));
    g_signal_connect_unlocked(combo, "destroy", G_CALLBACK(on_combo_int_destroy), NULL);

    gtk_widget_show(combo);
    return combo;
}


/** \brief  Create a combo box to control an integer resource
 *
 * \param[in]   resource    resource name
 * \param[in]   entries     list of entries for the combo box
 *
 * \return  GtkComboBoxText
 */
GtkWidget *vice_gtk3_resource_combo_box_int_new(
        const char *resource,
        const vice_gtk3_combo_entry_int_t *entries)
{
    GtkWidget * combo = gtk_combo_box_new();

    /* store a heap-allocated copy of the resource name in the object */
    resource_widget_set_resource_name(combo, resource);

    return resource_combo_box_int_new_helper(combo, entries);
}


/** \brief  Create a combo box to control an integer resource
 *
 * Allows setting the resource name via sprintf()-syntax
 *
 * \param[in]   fmt     format string for the resource name
 * \param[in]   entries list of entries for the combo box
 *
 * \return  GtkComboBoxText
 */
GtkWidget *vice_gtk3_resource_combo_box_int_new_sprintf(
        const char *fmt,
        const vice_gtk3_combo_entry_int_t *entries,
        ...)
{
    GtkWidget *combo;
    char *resource;
    va_list args;

    combo = gtk_combo_box_new();

    va_start(args, entries);
    resource = lib_mvsprintf(fmt, args);
    g_object_set_data(G_OBJECT(combo), "ResourceName", (gpointer)resource);
    va_end(args);

    return resource_combo_box_int_new_helper(combo, entries);
}


/** \brief  Create combo box for integer \a resource with a \a label
 *
 * \param[in]   resource    resource name
 * \param[in]   entries     list of entries for the combo box
 * \param[in]   label       text for the label
 *
 * \return  GtkGrid
 */
GtkWidget *vice_gtk3_resource_combo_box_int_new_with_label(
        const char *resource,
        const vice_gtk3_combo_entry_int_t *entries,
        const char *label)
{
    GtkWidget *grid;
    GtkWidget *lbl;
    GtkWidget *combo;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    lbl = gtk_label_new(label);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);

    combo = vice_gtk3_resource_combo_box_int_new(resource, entries);
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set new ID for integer combo box
 *
 * Set new ID of the combo box
 *
 * \param[in,out]   widget  integer resource combo box
 * \param[in]       id      new ID for the combo box resource
 *
 * \return  TRUE if the new \a id was set
 */
gboolean vice_gtk3_resource_combo_box_int_set(GtkWidget *widget, int id)
{
    GtkWidget *combo;

    if (GTK_IS_GRID(widget)) {
        /* widget is a grid with label & combo */
        combo = gtk_grid_get_child_at(GTK_GRID(widget), 1, 0);
    } else {
        combo = widget;
    }
    if (GTK_IS_COMBO_BOX(combo)) {
        return set_combo_int_id(GTK_COMBO_BOX(combo), id);
    }
    return FALSE;   /* should not get here */
}


/** \brief  Get current ID for integer combo box
 *
 * \param[in]   widget  integer resource combo box
 * \param[out]  dest    object to store ID
 *
 * \return  TRUE if the ID was properly read
 */
gboolean vice_gtk3_resource_combo_box_int_get(GtkWidget *widget, int *dest)
{
    GtkWidget *combo;

    if (GTK_IS_GRID(widget)) {
        /* widget is a grid with label & combo */
        combo = gtk_grid_get_child_at(GTK_GRID(widget), 1, 0);
    } else {
        combo = widget;
    }
    if (GTK_IS_COMBO_BOX(combo)) {
        return get_combo_int_id(GTK_COMBO_BOX(combo), dest);
    }
    return FALSE;   /* should not get here */
}


/** \brief  Reset \a widget to its factory default
 *
 * \param[in,out]   widget  integer resource combo box
 *
 * \return  TRUE if the widget was reset to its factory value
 */
gboolean vice_gtk3_resource_combo_box_int_factory(GtkWidget *widget)
{
    const char *resource;
    int value;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_default_value(resource, &value) < 0) {
        return FALSE;
    }
    return vice_gtk3_resource_combo_box_int_set(widget, value);
}


/** \brief  Reset \a widget to its state when it was instanciated
 *
 * \param[in,out]   widget  integer resource combo box
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_combo_box_int_reset(GtkWidget *widget)
{
    int orig;

    orig = resource_widget_get_int(widget, "ResourceOrig");
    return vice_gtk3_resource_combo_box_int_set(widget, orig);
}


/** \brief  Synchronize \a widget with its resource value
 *
 * \param[in,out]   widget  integer resource combo box
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_combo_box_int_sync(GtkWidget *widget)
{
    const char *resource_name;
    int resource_value;
    gulong handler_id;
    gboolean result;

    resource_name = resource_widget_get_resource_name(widget);
    if (resources_get_int(resource_name, &resource_value) < 0) {
        return FALSE;
    }

    handler_id = GPOINTER_TO_ULONG(g_object_get_data(G_OBJECT(widget),
                                                     "ChangedHandlerID"));
    debug_gtk3("changed handler id = %lu.", handler_id);
    g_signal_handler_block(G_OBJECT(widget), handler_id);
    result = set_combo_int_id(GTK_COMBO_BOX(widget), resource_value);
    g_signal_handler_unblock(G_OBJECT(widget), handler_id);
    return result;
}



/*
 * Combo box for string resources
 *
 * Presents a combo box with strings, and uses strings for keys
 */

/** \brief  Handler for the 'destroy' event of the combo box
 *
 * Frees the heap-allocated copy of the resource name
 *
 * \param[in,out]   combo       combo box
 * \param[in]       user_data   extra event data (unused)
 */
static void on_combo_str_destroy(GtkWidget *combo, gpointer user_data)
{
    resource_widget_free_resource_name(combo);
    resource_widget_free_string(combo, "ResourceOrig");
}


/** \brief  Handler for the 'changed' event of the string combo box
 *
 * Updates the resource connected to the combo box
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   extra event data (unused)
 */
static void on_combo_str_changed(GtkWidget *combo, gpointer user_data)
{
    const char *id_str;
    const char *resource;

    resource = resource_widget_get_resource_name(combo);
    id_str = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    resources_set_string(resource, id_str);
}


/** \brief  Create a combo box to control a string resource
 *
 * \param[in]   combo   combo box
 * \param[in]   entries list of entries for the combo box
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *resource_combo_box_str_new_helper(
        GtkWidget *combo,
        const vice_gtk3_combo_entry_str_t *entries)
{
    int index;
    const char *current;
    const char *resource;

    resource = resource_widget_get_resource_name(combo);

    /* get current value of resource */
    if (resources_get_string(resource, &current) < 0) {
        current = "";
    }
    /* store current resource value (NULL gets allocated as "") */
    resource_widget_set_string(combo, "ResourceOrig", current);

    /* add entries */
    for (index = 0; entries[index].name != NULL; index++) {
        const char *id_str;

        if (entries[index].id == NULL) {
            id_str = entries[index].name;
        } else {
            id_str = entries[index].id;
        }

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                id_str, entries[index].name);
        if (strcmp(current, id_str) == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
        }
    }

    /* register methods to be used by the resource widget manager */
    resource_widget_register_methods(
            combo,
            vice_gtk3_resource_combo_box_str_reset,
            vice_gtk3_resource_combo_box_str_factory,
            vice_gtk3_resource_combo_box_str_sync);


    /* connect signal handlers */
    g_signal_connect(combo, "changed", G_CALLBACK(on_combo_str_changed), NULL);
    g_signal_connect(combo, "destroy", G_CALLBACK(on_combo_str_destroy), NULL);

    gtk_widget_show(combo);
    return combo;
}


/** \brief  Create a combo box to control a string resource
 *
 * \param[in]   resource    resource name
 * \param[in]   entries     list of entries for the combo box
 *
 * \return  GtkComboBoxText
 */
GtkWidget *vice_gtk3_resource_combo_box_str_new(
        const char *resource,
        const vice_gtk3_combo_entry_str_t *entries)
{
    GtkWidget *combo;

    combo = gtk_combo_box_text_new();

    /* store a heap-allocated copy of the resource name in the object */
    resource_widget_set_resource_name(combo, resource);

    return resource_combo_box_str_new_helper(combo, entries);
}


/** \brief  Create a combo box to control a string resource
 *
 * \param[in]   fmt         resource name, printf-style format string
 * \param[in]   entries     list of entries for the combo box
 *
 * \return  GtkComboBoxText
 */
GtkWidget *vice_gtk3_resource_combo_box_str_new_sprintf(
        const char *fmt,
        const vice_gtk3_combo_entry_str_t *entries,
        ...)
{
    GtkWidget *combo;
    char *resource;
    va_list args;

    combo = gtk_combo_box_text_new();

    va_start(args, entries);
    resource = lib_mvsprintf(fmt, args);
    g_object_set_data(G_OBJECT(combo), "ResourceName", (gpointer)resource);
    va_end(args);

    return resource_combo_box_str_new_helper(combo, entries);
}


/** \brief  Create combo box for string \a resource with a \a label
 *
 * \param[in]   resource    resource name
 * \param[in]   entries     list of entries for the combo box
 * \param[in]   label       text for the label
 *
 * \return  GtkGrid
 */
GtkWidget *vice_gtk3_resource_combo_box_str_new_with_label(
        const char *resource,
        const vice_gtk3_combo_entry_str_t *entries,
        const char *label)
{
    GtkWidget *grid;
    GtkWidget *lbl;
    GtkWidget *combo;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    lbl = gtk_label_new(label);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);

    combo = vice_gtk3_resource_combo_box_str_new(resource, entries);
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update string combo box by ID
 *
 * Set new ID of the combo box
 *
 * \param[in,out]   widget  string resource combo box
 * \param[in]       id      new ID of the combo box
 *
 * \return  TRUE if the new \a id was set
 */
gboolean vice_gtk3_resource_combo_box_str_set(GtkWidget *widget, const char *id)
{
    GtkWidget *combo;

    if (GTK_IS_GRID(widget)) {
        combo = gtk_grid_get_child_at(GTK_GRID(widget), 1, 0);
    } else {
        combo = widget;
    }
    return gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), id);
}


/** \brief  Get current ID of combo box
 *
 * The ID is not allocated, only the pointer is copied. So to use the value
 * retrieved after the widget is destroyed, a copy needs to be made. Also note
 * that the current value of the resource is returned, not the value of the
 * combo box, so even if the combo box is 'out of sync', the proper resource
 * value will be returned. On failure `FALSE` will be returned and `NULL`stored
 * in \a *dest.
 *
 * \param[in]   widget  string resource combo box
 * \param[out]  dest    object to store ID
 *
 * \return  TRUE if the \a id was set
 */
gboolean vice_gtk3_resource_combo_box_str_get(GtkWidget *widget, const char **dest)
{
    const char *resource;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_string(resource, dest) < 0) {
        *dest = NULL;
        return FALSE;
    }
    return TRUE;
}


/** \brief  Reset string combo box to its factory default
 *
 * \param[in,out]   widget  string resource combo box
 *
 * \return  TRUE if the factory default was set
 */
gboolean vice_gtk3_resource_combo_box_str_factory(GtkWidget *widget)
{
    const char *resource;
    const char *value;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_default_value(resource, &value) < 0) {
        return FALSE;
    }
    return vice_gtk3_resource_combo_box_str_set(widget, value);
}


/** \brief  Reset string combo box to its original value
 *
 * Restores the state of the widget as it was when instanciated.
 *
 * \param[in,out]   widget  string resource combo box
 *
 * \return  TRUE if the combo box was reset
 */
gboolean vice_gtk3_resource_combo_box_str_reset(GtkWidget *widget)
{
    const char *orig;

    orig = resource_widget_get_string(widget, "ResourceOrig");
    return vice_gtk3_resource_combo_box_str_set(widget, orig);
}


/** \brief  Synchronize widget with its resource
 *
 * Updates the widget state to what its resource currently is.
 *
 * \param[in,out]   widget  string resource combo box
 *
 * \return  TRUE if the widget was synchronized with its resource
 */
gboolean vice_gtk3_resource_combo_box_str_sync(GtkWidget *widget)
{
    const char *resource;
    const char *current;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_string(resource, &current) < 0) {
        return FALSE;
    }
    return vice_gtk3_resource_combo_box_str_set(widget, current);
}



/*
 * GtkComboBoxText with free text entry
 *
 * Simple combo box with a text entry that allows entering a string not in the
 * drop down list.
 */

static gboolean combo_with_entry_fix_index(GtkWidget *widget, const char *value);


/** \brief  Handler for the 'destroy' event of the combo box with entry
 *
 * \param[in]   widget  resource combo box
 * \param[in]   unused  extra event data (unused)
 */
static void on_combo_with_entry_destroy(GtkWidget *widget, gpointer unused)
{
    resource_widget_free_resource_name(widget);
    resource_widget_free_string(widget, "ResourceOrig");
}


/** \brief  Handler for the 'changed' event of the combo box with entry
 *
 * Sets the resource to the new value, if it's a value in the drop down list.
 *
 * If the new value doesn't match an entry in the list we depend on the
 * 'focus-out' event to set the resource since each character entered triggers
 * this handler, which would result in setting the resource a lot with
 * (presumably) incomplete text.
 *
 * XXX: Also of note is the fact that manually entering a string that matches
 *      one of the values in the drop down list doesn't result in a valid
 *      index, -1 is still returned.
 *
 * \param[in]   widget  resource combo box
 * \param[in]   unused  extra event data (unused)
 */
static void on_combo_with_entry_changed(GtkWidget *widget, gpointer unused)
{
    gchar *text;
    gint index;

    text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
#if 0
    debug_gtk3("Index = %d, text = '%s'", index, text);
#endif

    if (index >= 0) {
        /* an item from the list was selected, we can accept that */
        const char *resource = resource_widget_get_resource_name(widget);
        resources_set_string(resource, text);
        combo_with_entry_fix_index(widget, text);
    }

    g_free(text);
}


/** \brief  Handler for the 'focus-out' event of the combo box' text entry
 *
 * To avoid setting the resource on each key press, we wait for the focus-out
 * event and then update the resource.
 *
 * \note    We pass \a combo since gtk_widget_get_parent(entry) doesn't return
 *          the combo box, although gtk_bin_get_child(combo) does return the
 *          entry :)
 *
 * \param[in]   entry   entry widget of the combo box
 * \param[in]   unused  event data (unused)
 * \param[in]   combo   parent combo box
 *
 * \return  `FALSE` to propagate events further, Gtk 3.24.31 complains when we
 *          return `TRUE` here (3.24.24 is fine with it)
 */
static gboolean on_combo_with_entry_focus_out_event(GtkEntry *entry,
                                                    GdkEvent *unused,
                                                    gpointer combo)
{
    const char *resource;
    const char *text;

    resource = resource_widget_get_resource_name(GTK_WIDGET(combo));
    text = gtk_entry_get_text(entry);
#if 0
    debug_gtk3("focus-out: setting \"%s\" to '%s'", resource, text);
#endif
    resources_set_string(resource, text);
    combo_with_entry_fix_index(GTK_WIDGET(combo), text);

    return FALSE;   /* always let the event propagate */
}


/** \brief  Handler for the 'key-press' event of the combo box' text entry
 *
 * If the user presses Return or Enter we store the value in the resource.
 *
 * \param[in]   entry   text entry
 * \param[in]   event   event data
 * \param[in]   combo   combo box
 *
 * \return  `FALSE` to propagate event further
 */
static gboolean on_combo_with_entry_key_press_event(GtkEntry *entry,
                                                    GdkEvent *event,
                                                    gpointer combo)
{
    GdkEventKey *keyev = (GdkEventKey *)event;

    if (keyev->type == GDK_KEY_PRESS &&
            (keyev->keyval == GDK_KEY_Return ||
             keyev->keyval == GDK_KEY_KP_Enter)) {
        const char *resource;
        const char *text;

        resource =  resource_widget_get_resource_name(GTK_WIDGET(combo));
        text = gtk_entry_get_text(entry);
        resources_set_string(resource, text);
        combo_with_entry_fix_index(GTK_WIDGET(combo), text);
    }
    return FALSE;
}


/** \brief  Fix combo box index to match value if its present in the list
 *
 * For some reason setting a string in the entry of the combo box that matches
 * a string in the drop down list does not update the list index to match the
 * string, so we brute force that here.
 *
 * \param[in]   widget  combo box with entry
 * \param[in]   value   value to try to match to string in the drop down list
 *
 * \return  `TRUE` when \a value was found in the drop down list
 */
static gboolean combo_with_entry_fix_index(GtkWidget *widget, const char *value)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gulong handler_id;
    int index = 0;
    gboolean found = FALSE;

    /* make sure we don't trigger the 'changed' event handler */
    handler_id = GPOINTER_TO_ULONG(g_object_get_data(G_OBJECT(widget),
                                                     "ChangedHandlerID"));
    /* when this function is called from the constructor the handler isn't
     * connected yet so handler_id will be 0, which isn't valid for the
     * block/unblock functions */
    if (handler_id > 0) {
        g_signal_handler_block(G_OBJECT(widget), handler_id);
    }

#if 0
    debug_gtk3("Iterating model for value '%s'", value);
#endif
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter)) {
        do {
            char *text = NULL;

            gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 0, &text, -1);
#if 0
            debug_gtk3("Checking '%s'", text);
#endif
            if (text != NULL && strcmp(text, value) == 0) {
#if 0
                debug_gtk3("Found index %d", index);
#endif
                gtk_combo_box_set_active(GTK_COMBO_BOX(widget), index);
                g_free(text);
                found = TRUE;
                break;
            }
            g_free(text);
            index++;
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter));
    }

    if (handler_id > 0) {
        g_signal_handler_unblock(G_OBJECT(widget), handler_id);
    }
    return found;
}


/** \brief  Create combo box with text entry
 *
 * Create a combo box where the user can enter a string not present in the
 * list of \a values.
 *
 * \note    This combo box only works for string-type resources and doesn't
 *          support (key,value) pairs.
 *
 * \param[in]   resource    resource name
 * \param[in]   entries     NULL-terminated list of strings
 *
 * \return  GtkComboBoxText
 */
GtkWidget *vice_gtk3_resource_combo_box_with_entry(const char *resource,
                                                   const char **values)

{
    GtkWidget *combo;
    GtkWidget *entry;
    const char *orig = NULL;
    gulong handler_id;

    combo = gtk_combo_box_text_new_with_entry();
    /* keep copy of resource name */
    resource_widget_set_resource_name(combo, resource);
    /* keep copy of resource value at instanciation */
    resources_get_string(resource, &orig);
    resource_widget_set_string(combo, "ResourceOrig", orig);

    /* add entries to the combo box */
    while (*values != NULL) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, *values);
        values++;
    }
    /* set the resource's value */
    entry = gtk_bin_get_child(GTK_BIN(combo));
    gtk_entry_set_text(GTK_ENTRY(entry), orig);
    /* fix up index */
    combo_with_entry_fix_index(combo, orig);

    /* now connect the event handlers */
    g_signal_connect_unlocked(combo,
                              "destroy",
                              G_CALLBACK(on_combo_with_entry_destroy),
                              NULL);
    handler_id = g_signal_connect(combo,
                                  "changed",
                                  G_CALLBACK(on_combo_with_entry_changed),
                                  NULL);
    g_object_set_data(G_OBJECT(combo),
                      "ChangedHandlerID",
                      GULONG_TO_POINTER(handler_id));
    /* focus-out must be connected to the child */
    g_signal_connect(entry,
                     "focus-out-event",
                     G_CALLBACK(on_combo_with_entry_focus_out_event),
                     (gpointer)combo);
    /* return/kp-enter to accept text entry */
    g_signal_connect(entry,
                     "key-press-event",
                     G_CALLBACK(on_combo_with_entry_key_press_event),
                     (gpointer)combo);

    return combo;
}


/** \brief  Set new value for the combo box, also setting the resource
 *
 * \param[in]   widget  combo box with entry
 * \param[in]   value   new value for the combo box
 *
 * \return  `TRUE` when setting the resource succeeded
 */
gboolean vice_gtk3_resource_combo_box_with_entry_set(GtkWidget *widget,
                                                     const char *value)
{
    GtkWidget *entry;
    const char *resource;

    entry = gtk_bin_get_child(GTK_BIN(widget));
    resource = resource_widget_get_resource_name(widget);

    gtk_entry_set_text(GTK_ENTRY(entry), value);
    combo_with_entry_fix_index(widget, value);
    return resources_set_string(resource, value) == 0 ? TRUE : FALSE;
}


/** \brief  Get text of the widget
 *
 * \param[in]   widget  combo box
 * \param[out]  value   location to store pointer to text
 *
 * \note    The data returned in \a value must be freed with g_free()
 *
 * \return  TRUE if text isn't `NULL`
 */
gboolean vice_gtk3_resource_combo_box_with_entry_get(GtkWidget *widget,
                                                     gchar **value)
{
    gchar *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

    *value = text;
    return text != NULL ? TRUE : FALSE;
}


/** \brief  Set the combo box to the resource's factory value
 *
 * This sets the combo box to the factory value of the connected resource, to
 * reset the combo box to the value the resource had when the combo box was
 * created use vice_gtk3_resource_combo_box_with_entry_reset().
 *
 * \note    This also sets the resource to its factory value.
 *
 * \param[in]   widget  combo box with entry
 *
 * \return  `TRUE` on success
 */
gboolean vice_gtk3_resource_combo_box_with_entry_factory(GtkWidget *widget)
{
    const char *resource;
    const char *factory;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_default_value(resource, &factory) == 0) {
        return vice_gtk3_resource_combo_box_with_entry_set(widget, factory);
    }
    return FALSE;
}


/** \brief  Reset the combo box to its value at the combo box' creation
 *
 * This sets the combo box to the value of the connected resource at the time
 * the combo box was created, to reset the resource to factory, use
 * vice_gtk3_resource_combo_box_with_entry_factory().
 *
 * \note    This also sets the resource to its original value.
 *
 * \param[in]   widget  combo box with entry
 *
 * \return  `TRUE` on success
 */
gboolean vice_gtk3_resource_combo_box_with_entry_reset(GtkWidget *widget)
{
    const char *orig = resource_widget_get_string(widget, "ResourceOrig");

    return vice_gtk3_resource_combo_box_with_entry_set(widget, orig);
}


/** \brief  Synchronize the combo box with its resource value
 *
 * \param[in]   widget  combo box with entry
 *
 * \return  `TRUE` on success
 */
gboolean vice_gtk3_resource_combo_box_with_entry_sync(GtkWidget *widget)
{
    GtkWidget *entry;
    const char *resource;
    const char *value;
    gulong handler_id;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_string(resource, &value) != 0) {
        return FALSE;
    }
    /* make sure we don't trigger the changed event handler */
    handler_id = GPOINTER_TO_ULONG(g_object_get_data(G_OBJECT(widget),
                                                     "ChangedHandlerID"));
    g_signal_handler_block(G_OBJECT(widget), handler_id);
    entry = gtk_bin_get_child(GTK_BIN(widget));
    gtk_entry_set_text(GTK_ENTRY(entry), value);
    combo_with_entry_fix_index(widget, value);
    g_signal_handler_unblock(G_OBJECT(widget), handler_id);
    return TRUE;
}
