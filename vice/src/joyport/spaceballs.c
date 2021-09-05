/*
 * spaceballs.c - Spaceballs 8-player joystick adapter emulation.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "joyport.h"
#include "joystick.h"
#include "spaceballs.h"
#include "resources.h"
#include "snapshot.h"
#include "userport.h"


#include "log.h"

/* 8 joysticks are wired in parallel with respect to their
   data lines. The ground of each joystick is hooked up to
   the data pins of the userport:

   GND  | USERPORT
   -----------------
   JOY1 | pin C (D0)
   JOY2 | pin D (D1)
   JOY3 | pin E (D2)
   JOY4 | pin F (D3)
   JOY5 | pin H (D4)
   JOY6 | pin J (D5)
   JOY7 | pin K (D6)
   JOY8 | pin L (D7)
   
   The userport is driven in such a way that only 1 joystick
   has ground.

   Works on:
   - native joystick port(s) (x64/x64sc/xscpu64/x128/xvic)

 */

static int spaceballs_enabled = 0;

static uint8_t spaceballs_grounds = 0xff;

/* ------------------------------------------------------------------------- */

static joyport_t joyport_spaceballs_device;

static old_userport_device_t userport_spaceballs_device;

static old_userport_device_list_t *userport_spaceballs_list_item = NULL;

static int joyport_spaceballs_enable(int port, int value)
{
    int val = value ? 1 : 0;

    if (val == spaceballs_enabled) {
        return 0;
    }

    if (val) {
        userport_spaceballs_list_item = old_userport_device_register(&userport_spaceballs_device);
        if (userport_spaceballs_list_item == NULL) {
            return -1;
        }
        joystick_adapter_activate(JOYSTICK_ADAPTER_ID_SPACEBALLS, joyport_spaceballs_device.name);
        joystick_adapter_set_ports(8);
    } else {
        joystick_adapter_deactivate();
        old_userport_device_unregister(userport_spaceballs_list_item);
        userport_spaceballs_list_item = NULL;
    }

    spaceballs_enabled = val;

    return 0;
}

static uint8_t spaceballs_read(int port)
{
    uint8_t retval = 0;
    uint16_t joyval = 0;

    int i;

    for (i = 0; i < 8; i++) {
        if (!((spaceballs_grounds >> i) & 1)) {
            joyval = get_joystick_value(JOYPORT_3 + i);
            retval |= (joyval & 0x1f);
        }
    }

    return ~(retval);
}

/* ------------------------------------------------------------------------- */

static int spaceballs_write_snapshot(struct snapshot_s *s, int p);
static int spaceballs_read_snapshot(struct snapshot_s *s, int p);

static joyport_t joyport_spaceballs_device = {
    "Joystick Adapter (Spaceballs)",  /* name of the device */
    JOYPORT_RES_ID_NONE,              /* device can be used in multiple ports at the same time */
    JOYPORT_IS_NOT_LIGHTPEN,          /* device is NOT a lightpen */
    JOYPORT_POT_OPTIONAL,             /* device does NOT use the potentiometer lines */
    JOYSTICK_ADAPTER_ID_SPACEBALLS,   /* device is a joystick adapter */
    JOYPORT_DEVICE_JOYSTICK_ADAPTER,  /* device is a Joystick adapter */
    0,                                /* NO output bits */
    joyport_spaceballs_enable,        /* device enable function */
    spaceballs_read,                  /* digital line read function */
    NULL,                             /* NO digital line store function */
    NULL,                             /* NO pot-x read function */
    NULL,                             /* NO pot-y read function */
    spaceballs_write_snapshot,        /* device write snapshot function */
    spaceballs_read_snapshot,         /* device read snapshot function */
    NULL,                             /* NO device hook function */
    0                                 /* NO device hook function mask */
};

/* ------------------------------------------------------------------------- */

int joyport_spaceballs_resources_init(void)
{
    return joyport_device_register(JOYPORT_ID_SPACEBALLS, &joyport_spaceballs_device);
}

/* ------------------------------------------------------------------------- */

int userport_spaceballs_enabled = 0;

static void userport_spaceballs_store_pbx(uint8_t value)
{
    spaceballs_grounds = value;
}

static old_userport_device_t userport_spaceballs_device = {
    USERPORT_DEVICE_SPACEBALLS,      /* device id */
    "Joystick Adapter (Spaceballs)", /* device name */
    JOYSTICK_ADAPTER_ID_NONE,        /* NOT a joystick adapter, the joyport part is the joystick adapter */
    NULL,                            /* NO read pb0-pb7 function */
    userport_spaceballs_store_pbx,   /* store pb0-pb7 function */
    NULL,                            /* NO read pa2 pin function */
    NULL,                            /* NO store pa2 pin function */
    NULL,                            /* NO read pa3 pin function */
    NULL,                            /* NO store pa3 pin function */
    0,                               /* pc pin is NOT needed */
    NULL,                            /* NO store sp1 pin function */
    NULL,                            /* NO read sp1 pin function */
    NULL,                            /* NO store sp2 pin function */
    NULL,                            /* NO read sp2 pin function */
    NULL,                            /* resource used by the device */
    0xff,                            /* NO return value */
    0xff,                            /* validity mask of the device, doesn't change */
    0,                               /* device involved in a read collision, to be filled in by the collision detection system */
    0                                /* a tag to indicate the order of insertion */
};

/* ------------------------------------------------------------------------- */

/* SPACEBALLS snapshot module format:

   type  |   name  | description
   ----------------------------------
   BYTE  | GROUNDS | userport PBx state
 */

static char snap_module_name[] = "SPACEBALLS";
#define SNAP_MAJOR   0
#define SNAP_MINOR   0

static int spaceballs_write_snapshot(struct snapshot_s *s, int p)
{
    snapshot_module_t *m;

    m = snapshot_module_create(s, snap_module_name, SNAP_MAJOR, SNAP_MINOR);

    if (m == NULL) {
        return -1;
    }

    if (0 
        || SMW_B(m, spaceballs_grounds) < 0) {
            snapshot_module_close(m);
            return -1;
    }
    return snapshot_module_close(m);
}

static int spaceballs_read_snapshot(struct snapshot_s *s, int p)
{
    uint8_t major_version, minor_version;
    snapshot_module_t *m;

    m = snapshot_module_open(s, snap_module_name, &major_version, &minor_version);

    if (m == NULL) {
        return -1;
    }

    /* Do not accept versions higher than current */
    if (snapshot_version_is_bigger(major_version, minor_version, SNAP_MAJOR, SNAP_MINOR)) {
        snapshot_set_error(SNAPSHOT_MODULE_HIGHER_VERSION);
        goto fail;
    }

    if (0
        || SMR_B(m, &spaceballs_grounds) < 0) {
        goto fail;
    }

    return snapshot_module_close(m);

fail:
    snapshot_module_close(m);
    return -1;
}
