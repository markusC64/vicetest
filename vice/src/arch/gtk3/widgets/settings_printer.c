/** \file   settings_printer.c
 * \brief   Widget to control printer settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/* FIXME:   Some of the resources mentioned here are actually controlled by
 *          widgets in other files:
 *
 * $VICERES VirtualDevice4              -vsid
 * $VICERES VirtualDevice5              -vsid
 * $VICERES VirtualDevice6              -vsid
 * $VICERES VirtualDevice7              -vsid
 * $VICERES IECDevice4                  -vsid -xvic
 * $VICERES IECDevice5                  -vsid -xvic
 * $VICERES IECDevice6                  -vsid -xvic
 * $VICERES IECDevice7                  -vsid -xvic
 * $VICERES Printer4                    -vsid
 * $VICERES Printer5                    -vsid
 * $VICERES Printer6                    -vsid
 * $VICERES Printer7                    -vsid
 * $VICERES Printer4Driver              -vsid
 * $VICERES Printer5Driver              -vsid
 * $VICERES Printer6Driver              -vsid
 * $VICERES Printer4Output              -vsid
 * $VICERES Printer5Output              -vsid
 * $VICERES Printer6Output              -vsid
 * $VICERES PrinterTextDevice1          -vsid
 * $VICERES PrinterTextDevice2          -vsid
 * $VICERES PrinterTextDevice3          -vsid
 * $VICERES Printer4TextDevice          -vsid
 * $VICERES Printer5TextDevice          -vsid
 * $VICERES Printer6TextDevice          -vsid
 * $VICERES Printer7TextDevice          -vsid
 * $VICERES PrinterUserport             -vsid
 * $VICERES PrinterUserportTextDevice   -vsid
 * $VICERES PrinterUserportDriver       -vsid
 * $VICERES PrinterUserportOutput       -vsid
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
#include "debug_gtk3.h"
#include "resources.h"
#include "machine.h"
#include "printer.h"

/* widgets */
#include "printeremulationtypewidget.h"
#include "printerdriverwidget.h"
#include "printeroutputmodewidget.h"
#include "printeroutputdevicewidget.h"
#include "userportprinterwidget.h"

#include "settings_printer.h"


#define PRINTER_NUM 4   /**< number of printer devices supported */
#define PRINTER_MIN 4   /**< lowest device number for a printer */
#define PRINTER_MAX 7   /**< highest device number for a printer */


/** \brief  Handler for the "toggled" event of the Printer7 checkbox
 *
 * Switches between `PRINTER_DEVICE_NONE` and `PRINTER_DEVICE_REAL`.
 *
 * \param[in]   check       check button
 * \param[in]   user_data   extra data (unused)
 */
static void on_real_device7_toggled(GtkCheckButton *check, gpointer user_data)
{
    int state;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check))) {
        state = PRINTER_DEVICE_REAL;
    } else {
        state = PRINTER_DEVICE_NONE;
    }
    resources_set_int("Printer7", state);
}


/** \brief  Set PrinterTextDevice[1-3] resource
 *
 * \param[in]   entry       text entry
 * \param[in]   user_data   resource index (1-3)
 */
static void on_text_device_changed(GtkEntry *entry, gpointer user_data)
{
    int num = GPOINTER_TO_INT(user_data);
    const gchar *text = gtk_entry_get_text(entry);

    resources_set_string_sprintf("PrinterTextDevice%d", text, num);
}


/** \brief  Create Virtual device widget for \a device
 *
 * \param[in]   device  printer device
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_virtual_device_widget(int device)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new_sprintf("VirtualDevice%d",
            "Enable Virtual Device", device);
    gtk_widget_set_margin_start(check, 16);
    return check;
}

/** \brief  Create IEC device emulation widget for \a device
 *
 * \param[in]   device  printer device
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_iec_widget(int device)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new_sprintf("IECDevice%d",
            "Enable IEC device", device);
    gtk_widget_set_margin_start(check, 16);
    return check;
}


/** \brief  Create checkbox to switch between NONE/REAL emu mode for Printer7
 *
 * NOTE: Cannot use resourcecheckbutton here since this toggle button switches
 *       between PRINTER_DEVICE_NONE (0) and PRINTER_DEVICE_REAL (2).
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_real_device7_checkbox(void)
{
    GtkWidget *check;
    int value;

    check = gtk_check_button_new_with_label("Real device access");
    gtk_widget_set_margin_start(check, 16);
    resources_get_int("Printer7", &value);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), value);
    g_signal_connect(check, "toggled", G_CALLBACK(on_real_device7_toggled),
            NULL);
    return check;
}


/** \brief  Handler for the 'clicked event of the "formfeed" button
 *
 * \param[in]   widget  button
 * \param[in]   data    device number (4-7)
 */
static void on_formfeed_clicked(GtkWidget *widget, gpointer data)
{
    int device;

    device = GPOINTER_TO_INT(data);

    printer_formfeed((unsigned int)device - 4);
}


/** \brief  Create button to send formfeed to the printer
 *
 * \param[in]   device  device number (4-7)
 *
 * \return  GtkButton
 */
static GtkWidget *create_formfeed_button(int device)
{
    GtkWidget *button;

    button = gtk_button_new_with_label("Send formfeed");
    g_signal_connect(button, "clicked", G_CALLBACK(on_formfeed_clicked),
            GINT_TO_POINTER(device));
    return button;
}


/** \brief  Create a widget for the settings of printer # \a device
 *
 * Creates a widget for \a device to control its resource. The widget for
 * device #7 is different/simpler.
 *
 * \param[in]   device  device number (4-7)
 *
 * \return  GtkGrid
 */
static GtkWidget *create_printer_widget(int device)
{
    GtkWidget *grid;
    char title[256];

    g_snprintf(title, sizeof(title), "Printer #%d settings", device);

    grid = vice_gtk3_grid_new_spaced_with_label(-1, -1, title, 4);

    if (device == 4 || device == 5 || device == 6) {
        /* device 4,5,6 are 'normal' printers */
        GtkWidget *wrapper;

        switch (machine_class) {
            /* these machines have IEC */
            case VICE_MACHINE_C64:      /* fall through */
            case VICE_MACHINE_C64SC:    /* fall through */
            case VICE_MACHINE_SCPU64:   /* fall through */
            case VICE_MACHINE_C128:     /* fall through */
            case VICE_MACHINE_C64DTV:   /* fall through */
#if 0
            /* FIXME: xvic does not use the generic IEC bus code in src/iecbus/iecbus.c yet */
            case VICE_MACHINE_VIC20:    /* fall through */
#endif
            case VICE_MACHINE_PLUS4:

                wrapper = gtk_grid_new();
                gtk_grid_attach(GTK_GRID(wrapper),
                        printer_emulation_type_widget_create(device),
                        0, 0, 1, 1);
                gtk_grid_attach(GTK_GRID(wrapper),
                        create_virtual_device_widget(device), 0, 1, 1, 1);
                gtk_grid_attach(GTK_GRID(wrapper),
                        create_iec_widget(device), 0, 2, 1, 1);

                gtk_grid_attach(GTK_GRID(grid), wrapper, 0, 1, 1, 1);
                break;

            default:
                /* No IEC */
                gtk_grid_attach(GTK_GRID(grid),
                        printer_emulation_type_widget_create(device),
                        0, 1, 1, 1);
                gtk_grid_attach(GTK_GRID(grid),
                        create_virtual_device_widget(device), 0, 2, 1, 1);
                break;
        }

        gtk_grid_attach(GTK_GRID(grid),
                printer_driver_widget_create(device), 1, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid),
                printer_output_mode_widget_create(device), 2, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid),
                printer_output_device_widget_create(device), 3, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid),
                create_formfeed_button(device), 0, 6, 1, 1);

    } else if (device == 7) {
        /* device 7 is 'special' */
        GtkWidget *iec_widget;
        GtkWidget *virtual_device_widget = create_virtual_device_widget(device);

        gtk_grid_attach(GTK_GRID(grid), create_real_device7_checkbox(),
                0, 1, 1, 1);

        switch (machine_class) {
            /* these machines have IEC */
            case VICE_MACHINE_C64:      /* fall through */
            case VICE_MACHINE_C64SC:    /* fall through */
            case VICE_MACHINE_SCPU64:   /* fall through */
            case VICE_MACHINE_C128:     /* fall through */
            case VICE_MACHINE_C64DTV:   /* fall through */
#if 0
            /* FIXME: xvic does not use the generic IEC bus code in src/iecbus/iecbus.c yet */
            case VICE_MACHINE_VIC20:    /* fall through */
#endif
            case VICE_MACHINE_PLUS4:
                iec_widget = create_iec_widget(device);
                gtk_grid_attach(GTK_GRID(grid), virtual_device_widget, 0, 2, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), iec_widget, 0, 3, 1, 1);
                break;

            default:
                break;
        }
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control Printer Text Devices 1-3
 *
 * \return  GtkGrid
 */
static GtkWidget *create_printer_text_devices_widget(void)
{
    GtkWidget *grid;
    int i;

    grid = vice_gtk3_grid_new_spaced_with_label(
            -1, -1, "Printer output devices", 6);
    for (i = 0; i < 3; i++) {
        GtkWidget *label;
        GtkWidget *entry;
        const char *text;
        gchar title[64];

        g_snprintf(title, sizeof(title), "#%d", i + 1);

        label= gtk_label_new(title);
        if (i == 0) {
            gtk_widget_set_margin_start(label, 16);
        }
        gtk_grid_attach(GTK_GRID(grid), label, i * 2, 1, 1, 1);
        entry = gtk_entry_new();
        resources_get_string_sprintf("PrinterTextDevice%d", &text, i + 1);
        gtk_entry_set_text(GTK_ENTRY(entry), text);
        gtk_grid_attach(GTK_GRID(grid), entry, i * 2 + 1, 1, 1, 1);

        g_signal_connect(entry, "changed", G_CALLBACK(on_text_device_changed),
                GINT_TO_POINTER(i + 1));
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control printer settings
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_printer_widget_create(GtkWidget *parent)
{
    GtkWidget *layout;
    GtkWidget *stack;
    GtkWidget *stack_switcher;
    GtkWidget *layout_userport = NULL;
    int p;
    char buffer[256];

    layout = vice_gtk3_grid_new_spaced(0, 16);

    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack),
            GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 1000);

    for (p = PRINTER_MIN; p <= PRINTER_MAX; p++) {
        g_snprintf(buffer, 256, "Printer #%d", p);
        gtk_stack_add_titled(GTK_STACK(stack), create_printer_widget(p),
                buffer, buffer);
    }

    /* determine if the userport is available */
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            /* create userport printer widget and add to stack */
            layout_userport = userport_printer_widget_create();
            gtk_stack_add_titled(GTK_STACK(stack), layout_userport, "userport",
                    "Userport printer");
            break;
        default:
            /* No userport (C64DTV, CBM 5x0, VSID) or not usable for printer
             * (Plus4) */
            break;
    }

    stack_switcher = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(stack_switcher),
            GTK_STACK(stack));
    gtk_widget_set_halign(stack_switcher, GTK_ALIGN_CENTER);

    gtk_widget_show_all(stack);
    gtk_widget_show_all(stack_switcher);

    gtk_grid_attach(GTK_GRID(layout), stack_switcher, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), stack, 0, 1, 1, 1);

    gtk_orientable_set_orientation(GTK_ORIENTABLE(stack_switcher),
            GTK_ORIENTATION_HORIZONTAL);

    gtk_grid_attach(GTK_GRID(layout), create_printer_text_devices_widget(),
            0, 2, 1, 1);

    gtk_widget_show_all(layout);
    return layout;
}
