/** \file   resourcecheckbutton.c
 * \brief   Check button connected to a resource
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * This widget presents a check button that controls a boolean resource. During
 * construction the current resource value is stored in the widget to allow
 * resetting to default.
 *
 * \section resource_check_button_example
 * \code{.c}
 *
 *  GtkWidget *check;
 *
 *  // create a widget
 *  check = vice_gtk3_resource_check_button_create("SomeResource");
 *  // any state change of the widget will now update the resource
 *
 *  // restore widget & resource to their initial state
 *  vice_gtk3_resource_check_button_reset(check);
 *
 *  // restore widget & resource to the resource's factory value
 *  vice_gtk3_resource_check_button_factory(check);
 *
 * \endcode
 *
 * Extra GObject data used:
 * (do not overwrite these unless you know what you're doing)
 *
 * Set via resourcehelpers.c:
 *  - ResourceName:     resource name (string)
 *  - ResourceOrig:     resource value on construction (int)
 *  - MethodReset:      reset resource to original value (function)
 *  - MethodFactory:    set resource to factory value (function)
 *  - MethodSync:       update widget with current resource value (function)
 *
 * Set locally:
 *
 *  - ExtraCallback:    user-defined callback to call on state change
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

#include "lib.h"
#include "log.h"
#include "resources.h"
#include "resourcehelpers.h"

#include "resourcecheckbutton.h"


/** \brief  Handler for the "destroy" event of the check button
 *
 * Frees the heap-allocated copy of the resource name.
 *
 * \param[in,out]   check       check button
 * \param[in]       user_data   extra event data (unused)
 */
static void on_check_button_destroy(GtkWidget *check, gpointer user_data)
{
    resource_widget_free_resource_name(check);
}


/** \brief  Handler for the 'toggled' event of the check button
 *
 * \param[in]   check       check button
 * \param[in]   user_data   resource name
 */
static void on_check_button_toggled(GtkWidget *check, gpointer user_data)
{
    int state;
    const gchar *resource;
    void (*callback)(GtkWidget *, int);

    resource = resource_widget_get_resource_name(check);
    if (resources_get_int(resource, &state) < 0) {
        /* warning */
        log_error(LOG_ERR, "invalid resource name '%s'", resource);
        return;
    }
    /* Whatever value the check button is now, use as the new resource value */
    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)) ? 1 : 0;
    resources_set_int(resource, state);

    /* call user callback if set */
    callback = g_object_get_data(G_OBJECT(check), "ExtraCallback");
    if (callback != NULL) {
        callback(check, state);
    }
}


/** \brief  Check button setup helper
 *
 * Called by either resource_check_button_create() or
 * resource_check_button_create_printf() to finish setting up the resource
 * check button \a check
 *
 * \param[in,out]   check   check button
 *
 * \return  new check button
 */
static GtkWidget *resource_check_button_new_helper(GtkWidget *check)
{
    int state;
    const char *resource;

    /* get current resource value */
    resource = resource_widget_get_resource_name(check);
    if (resources_get_int(resource, &state) < 0) {
        /* invalid resource, set state to off */
        log_error(LOG_ERR, "invalid resource name '%s'", resource);
        state = 0;
    }
    /* remember original state for the reset() method */
    resource_widget_set_int(check, "ResourceOrig", state);

    /* set extra callback to NULL */
    g_object_set_data(G_OBJECT(check), "ExtraCallback", NULL);

    /* set auto-update to true */
    resource_widget_set_auto_update(check, TRUE);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
            state ? TRUE : FALSE);

    /* register methods to be used by the resource widget manager */
    resource_widget_register_methods(
            check,
            vice_gtk3_resource_check_button_reset,
            vice_gtk3_resource_check_button_factory,
            vice_gtk3_resource_check_button_sync);

    g_signal_connect(check, "toggled", G_CALLBACK(on_check_button_toggled),
            (gpointer)resource);
    g_signal_connect_unlocked(check, "destroy", G_CALLBACK(on_check_button_destroy),
            NULL);

    gtk_widget_show(check);
    return check;
}


/** \brief  Create check button to toggle \a resource
 *
 * Creates a check button to toggle \a resource. Makes a heap-allocated copy
 * of the resource name so initializing this widget with a constructed/temporary
 * resource name works as well.
 *
 * \param[in]   resource    resource name
 * \param[in]   label       label of the check button (optional)
 *
 * \note    The resource name is stored in the "ResourceName" property.
 *
 * \return  new check button
 */
GtkWidget *vice_gtk3_resource_check_button_new(const char *resource,
                                               const char *label)
{
    GtkWidget *check;

    /* make label optional */
    if (label != NULL) {
        check = gtk_check_button_new_with_label(label);
    } else {
        check = gtk_check_button_new();
    }

    /* make a copy of the resource name and store the pointer in the propery
     * "ResourceName" */
    resource_widget_set_resource_name(check, resource);

    return resource_check_button_new_helper(check);
}


/** \brief  Create check button to toggle a resource
 *
 * Creates a check button to toggle a resource. Makes a heap-allocated copy
 * of the resource name so initializing this widget with a constructed/temporary
 * resource name works as well. The resource name can be constructed with a
 * printf() format string.
 *
 * \param[in]   fmt         resource name format string
 * \param[in]   label       label of the check button and optional printf args
 *
 * \note    The resource name is stored in the "ResourceName" property.
 *
 * \return  new check button
 */
GtkWidget *vice_gtk3_resource_check_button_new_sprintf(const char *fmt,
                                                       const char *label,
                                                       ...)
{
    GtkWidget *check;
    va_list args;
    char *resource;

    check = gtk_check_button_new_with_label(label);

    va_start(args, label);
    resource = lib_mvsprintf(fmt, args);
    g_object_set_data(G_OBJECT(check), "ResourceName", (gpointer)resource);
    va_end(args);

    return resource_check_button_new_helper(check);
}


/** \brief  Set new \a value for \a widget
 *
 * \param[in,out]   widget  resource check button widget
 * \param[in]       value   new value
 *
 * \return  TRUE
 */
gboolean vice_gtk3_resource_check_button_set(GtkWidget *widget, gboolean value)
{
    /* the 'toggled' event handler will update the resouce if required */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), value);
    return TRUE;
}


/** \brief  Get the current value of the resource
 *
 * \param[in]   widget  resource check button widget
 * \param[out]  dest    object to store value
 *
 * \return  TRUE if the resource value was retrieved
 */
gboolean vice_gtk3_resource_check_button_get(GtkWidget *widget, gboolean *dest)
{
    const char *resource;
    int value;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_int(resource, &value) < 0) {
        *dest = FALSE;
        return FALSE;
    }
    *dest = (gboolean)value;
    return TRUE;
}


/** \brief  Reset check button to factory state
 *
 * \param[in,out]   widget  resource check button widget
 *
 * \return  TRUE if the resource was reset to its factory value
 */
gboolean vice_gtk3_resource_check_button_factory(GtkWidget *widget)
{
    const char *resource;
    int value;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_default_value(resource, &value) < 0) {
        return FALSE;
    }
    return vice_gtk3_resource_check_button_set(widget, (gboolean)value);
}


/** \brief  Reset \a widget to the state it was when it was created
 *
 * \param[in,out]   widget  resource check button widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_check_button_reset(GtkWidget *widget)
{
    int orig;

    orig = resource_widget_get_int(widget, "ResourceOrig");
    return vice_gtk3_resource_check_button_set(widget, (gboolean)orig);
}


/** \brief  Synchronize the \a widget state with its resource
 *
 * \param[in,out]   widget  resource check button
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_check_button_sync(GtkWidget *widget)
{
    int widget_val;
    const char *resource_name;
    int resource_val;

    /* get widget state */
    widget_val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    /* get resource state */
    resource_name = resource_widget_get_resource_name(widget);
    if (resources_get_int(resource_name, &resource_val) < 0) {
        return FALSE;
    }

    /* do we need to update the widget? */
    if ((gboolean)resource_val != (gboolean)widget_val) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                (gboolean)resource_val);
    }
    return TRUE;
}


/** \brief  Set resource to the widget's value
 *
 * \param[in,out]   widget  resource check button
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_check_button_apply(GtkWidget *widget)
{
    const char *resource;
    int state;
    int current;

    resource = resource_widget_get_resource_name(widget);
    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (resources_get_int(resource, &current) < 0) {
        /* invalid resource, exit */
        log_error(LOG_ERR, "invalid resource name'%s'", resource);
        return FALSE;
    }

    /* make sure we don't update a resource when the UI happens to be out of
     * sync for some reason */
    if (state != current) {
        if (resources_set_int(resource, state ? 1 : 0) < 0) {
            log_error(LOG_ERR,
                    "setting %s to %s failed",
                    resource, state ? "True": "False");
            /* get current resource value (validity of the name has been
             * checked already */
            resources_get_int(resource, &current);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                    current ? TRUE : FALSE);
        }
    }
    return TRUE;
}


/** \brief  Disable the auto updating of the bound resource
 *
 * \param[in,out]   widget  resource check button widget
 */
void vice_gtk3_resource_check_button_disable_auto_update(GtkWidget *widget)
{
    resource_widget_set_auto_update(widget, FALSE);
}


/** \brief  Add user callback to resource checkbutton
 *
 * \param[in,out]   widget      resource checkbutton
 * \param[in]       callback    function to call when the checkbutton state
 *                              changes
 */
void vice_gtk3_resource_check_button_add_callback(
        GtkWidget *widget,
        void (*callback)(GtkWidget *, int))
{
    g_object_set_data(G_OBJECT(widget), "ExtraCallback", (gpointer)callback);
}
