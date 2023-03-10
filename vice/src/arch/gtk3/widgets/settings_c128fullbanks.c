/** \file   settings_c128fullbanks.c
 * \brief   Settings widget to control C128 full banks
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES C128FullBanks x128
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

#include "settings_c128fullbanks.h"


/** \brief  Create widget to control C128 full banks
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_c128fullbanks_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();

    gtk_grid_attach(GTK_GRID(grid),
            vice_gtk3_resource_check_button_new("C128FullBanks",
                "Enable RAM banks 2 & 3"),
            0, 0, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
