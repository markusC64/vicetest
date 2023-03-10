/** \file   vsidcontrolwidget.h
 * \brief   GTK3 control widget for VSID - header
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

#ifndef VICE_VSIDCONTROLWIDGET_H
#define VICE_VSIDCONTROLWIDGET_H

#include "vice.h"
#include <gtk/gtk.h>

/** \brief  VSID player status */
typedef enum vsid_control_e {
    VSID_ERROR = -1,    /**< player encountered error */
    VSID_STOPPED = 0,   /**< player is stopped */
    VSID_PLAYING,       /**< player is playing normally */
    VSID_PAUSED,        /**< player is paused */
    VSID_FORWARDING     /**< player is fast-forwarding */
} vsid_control_t;

GtkWidget *vsid_control_widget_create(void);
void vsid_control_widget_set_tune_count(int nr);
void vsid_control_widget_set_tune_current(int nr);
void vsid_control_widget_set_tune_default(int nr);
void vsid_control_widget_set_progress(gdouble fraction);
void vsid_control_widget_next_tune(void);
void vsid_control_widget_set_repeat(gboolean enabled);
gboolean vsid_control_widget_get_repeat(void);
void vsid_control_widget_set_state(vsid_control_t state);

#endif
