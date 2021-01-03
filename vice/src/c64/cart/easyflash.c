
/*
 * easyflash.c - Cartridge handling of the easyflash cart.
 *
 * Written by
 *  ALeX Kazik <alx@kazik.de>
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
#include <string.h>

#include "archdep.h"
#define CARTRIDGE_INCLUDE_SLOTMAIN_API
#include "c64cartsystem.h"
#undef CARTRIDGE_INCLUDE_SLOTMAIN_API
#include "c64mem.h"
#include "cartio.h"
#include "cartridge.h"
#include "cmdline.h"
#include "crt.h"
#include "easyflash.h"
#include "export.h"
#include "flash040.h"
#include "lib.h"
#include "log.h"
#include "maincpu.h"
#include "mem.h"
#include "monitor.h"
#include "ram.h"
#include "resources.h"
#include "snapshot.h"
#include "util.h"
#include "sysfile.h"

#define EASYFLASH_N_BANK_BITS 6
#define EASYFLASH_N_BANKS     (1 << (EASYFLASH_N_BANK_BITS))
#define EASYFLASH_BANK_MASK   ((EASYFLASH_N_BANKS) -1)

/* the 29F040B statemachine */
static flash040_context_t *easyflash_state_low = NULL;
static flash040_context_t *easyflash_state_high = NULL;

/* the jumper */
static int easyflash_jumper;

/* writing back to crt enabled */
static int easyflash_crt_write;

/* optimizing crt enabled */
static int easyflash_crt_optimize;

static int easyflash_1541u2_mode;
static int easyflash_tc64_mode;
static int easyflash_replace_eapi;

struct uci_queue
{
	int size;
	int pos;
	unsigned char data[896];
};

static struct uci_queue easyflash_1541u2_uci_command;
static struct uci_queue easyflash_1541u2_uci_status;
static struct uci_queue easyflash_1541u2_uci_data;

static int easyflash_1541u2_data_acc;
static int easyflash_1541u2_error;
static int easyflash_1541u2_state;
static int easyflash_1541u2_reset;

static int easyflash_1541u2_changestateWait;
static int easyflash_1541u2_changestateNewState;

/* backup of the registers */
static uint8_t easyflash_register_00, easyflash_register_02;

/* decoding table of the modes */
static const uint8_t easyflash_memconfig[] = {
    /* bit3 = jumper, bit2 = mode, bit1 = !exrom, bit0 = game */

    /* jumper off, mode 0, trough 00,01,10,11 in game/exrom bits */
    3, /* exrom high, game low, jumper off */
    3, /* Reserved, don't use this */
    1, /* exrom low, game low, jumper off */
    1, /* Reserved, don't use this */

    /* jumper off, mode 1, trough 00,01,10,11 in game/exrom bits */
    2, 3, 0, 1,

    /* jumper on, mode 0, trough 00,01,10,11 in game/exrom bits */
    2, /* exrom high, game low, jumper on */
    3, /* Reserved, don't use this */
    0, /* exrom low, game low, jumper on */
    1, /* Reserved, don't use this */

    /* jumper on, mode 1, trough 00,01,10,11 in game/exrom bits */
    2, 3, 0, 1,
};

#define CART_RAM_SIZE 256

/* extra RAM */
static uint8_t easyflash_ram[CART_RAM_SIZE];

/* filename when attached */
static char *easyflash_filename = NULL;
static int easyflash_filetype = 0;

static const char STRING_EASYFLASH[] = CARTRIDGE_NAME_EASYFLASH;

static const unsigned char eapiam29f040[768] = {
    0x65, 0x61, 0x70, 0x69, 0xc1, 0x4d,
    0x2f, 0xcd, 0x32, 0x39, 0xc6, 0x30, 0x34, 0x30,
    0x20, 0xd6, 0x31, 0x2e, 0x34, 0x00, 0x08, 0x78,
    0xa5, 0x4b, 0x48, 0xa5, 0x4c, 0x48, 0xa9, 0x60,
    0x85, 0x4b, 0x20, 0x4b, 0x00, 0xba, 0xbd, 0x00,
    0x01, 0x85, 0x4c, 0xca, 0xbd, 0x00, 0x01, 0x85,
    0x4b, 0x18, 0x90, 0x70, 0x4c, 0x67, 0x01, 0x4c,
    0xa4, 0x01, 0x4c, 0x39, 0x02, 0x4c, 0x40, 0x02,
    0x4c, 0x44, 0x02, 0x4c, 0x4e, 0x02, 0x4c, 0x58,
    0x02, 0x4c, 0x8e, 0x02, 0x4c, 0xd9, 0x02, 0x4c,
    0xd9, 0x02, 0x8d, 0x02, 0xde, 0xa9, 0xaa, 0x8d,
    0x55, 0x85, 0xa9, 0x55, 0x8d, 0xaa, 0x82, 0xa9,
    0xa0, 0x8d, 0x55, 0x85, 0xad, 0xf2, 0xdf, 0x8d,
    0x00, 0xde, 0xa9, 0x00, 0x8d, 0xff, 0xff, 0xa2,
    0x07, 0x8e, 0x02, 0xde, 0x60, 0x8d, 0x02, 0xde,
    0xa9, 0xaa, 0x8d, 0x55, 0xe5, 0xa9, 0x55, 0x8d,
    0xaa, 0xe2, 0xa9, 0xa0, 0x8d, 0x55, 0xe5, 0xd0,
    0xdb, 0xa2, 0x55, 0x8e, 0xe3, 0xdf, 0x8c, 0xe4,
    0xdf, 0xa2, 0x85, 0x8e, 0x02, 0xde, 0x8d, 0xff,
    0xff, 0x4c, 0xbb, 0xdf, 0xad, 0xff, 0xff, 0x60,
    0xcd, 0xff, 0xff, 0x60, 0xa2, 0x6f, 0xa0, 0x7f,
    0xb1, 0x4b, 0x9d, 0x80, 0xdf, 0xdd, 0x80, 0xdf,
    0xd0, 0x21, 0x88, 0xca, 0x10, 0xf2, 0xa2, 0x00,
    0xe8, 0x18, 0xbd, 0x80, 0xdf, 0x65, 0x4b, 0x9d,
    0x80, 0xdf, 0xe8, 0xbd, 0x80, 0xdf, 0x65, 0x4c,
    0x9d, 0x80, 0xdf, 0xe8, 0xe0, 0x1e, 0xd0, 0xe8,
    0x18, 0x90, 0x06, 0xa9, 0x01, 0x8d, 0xb9, 0xdf,
    0x38, 0x68, 0x85, 0x4c, 0x68, 0x85, 0x4b, 0xb0,
    0x48, 0xa9, 0xaa, 0xa0, 0xe5, 0x20, 0xd5, 0xdf,
    0xa0, 0x85, 0x20, 0xd5, 0xdf, 0xa9, 0x55, 0xa2,
    0xaa, 0xa0, 0xe2, 0x20, 0xd7, 0xdf, 0xa2, 0xaa,
    0xa0, 0x82, 0x20, 0xd7, 0xdf, 0xa9, 0x90, 0xa0,
    0xe5, 0x20, 0xd5, 0xdf, 0xa0, 0x85, 0x20, 0xd5,
    0xdf, 0xad, 0x00, 0xa0, 0x8d, 0xf1, 0xdf, 0xae,
    0x01, 0xa0, 0x8e, 0xb9, 0xdf, 0xc9, 0x01, 0xd0,
    0x06, 0xe0, 0xa4, 0xd0, 0x02, 0xf0, 0x0c, 0xc9,
    0x20, 0xd0, 0x39, 0xe0, 0xe2, 0xd0, 0x35, 0xf0,
    0x02, 0xb0, 0x50, 0xad, 0x00, 0x80, 0xae, 0x01,
    0x80, 0xc9, 0x01, 0xd0, 0x06, 0xe0, 0xa4, 0xd0,
    0x02, 0xf0, 0x08, 0xc9, 0x20, 0xd0, 0x19, 0xe0,
    0xe2, 0xd0, 0x15, 0xa0, 0x3f, 0x8c, 0x00, 0xde,
    0xae, 0x02, 0x80, 0xd0, 0x13, 0xae, 0x02, 0xa0,
    0xd0, 0x12, 0x88, 0x10, 0xf0, 0x18, 0x90, 0x12,
    0xa9, 0x02, 0xd0, 0x0a, 0xa9, 0x03, 0xd0, 0x06,
    0xa9, 0x04, 0xd0, 0x02, 0xa9, 0x05, 0x8d, 0xb9,
    0xdf, 0x38, 0xa9, 0x00, 0x8d, 0x00, 0xde, 0xa0,
    0xe0, 0xa9, 0xf0, 0x20, 0xd7, 0xdf, 0xa0, 0x80,
    0x20, 0xd7, 0xdf, 0xad, 0xb9, 0xdf, 0xb0, 0x08,
    0xae, 0xf1, 0xdf, 0xa0, 0x40, 0x28, 0x18, 0x60,
    0x28, 0x38, 0x60, 0x8d, 0xb7, 0xdf, 0x8e, 0xb9,
    0xdf, 0x8e, 0xed, 0xdf, 0x8c, 0xba, 0xdf, 0x08,
    0x78, 0x98, 0x29, 0xbf, 0x8d, 0xee, 0xdf, 0xa9,
    0x00, 0x8d, 0x00, 0xde, 0xa9, 0x85, 0xc0, 0xe0,
    0x90, 0x05, 0x20, 0xc1, 0xdf, 0xb0, 0x03, 0x20,
    0x9e, 0xdf, 0xa2, 0x14, 0x20, 0xec, 0xdf, 0xf0,
    0x06, 0xca, 0xd0, 0xf8, 0x18, 0x90, 0x63, 0xad,
    0xf2, 0xdf, 0x8d, 0x00, 0xde, 0x18, 0x90, 0x72,
    0x8d, 0xb7, 0xdf, 0x8e, 0xb9, 0xdf, 0x8c, 0xba,
    0xdf, 0x08, 0x78, 0x98, 0xc0, 0x80, 0xf0, 0x04,
    0xa0, 0xe0, 0xa9, 0xa0, 0x8d, 0xee, 0xdf, 0xc8,
    0xc8, 0xc8, 0xc8, 0xc8, 0xa9, 0xaa, 0x20, 0xd5,
    0xdf, 0xa9, 0x55, 0xa2, 0xaa, 0x88, 0x88, 0x88,
    0x20, 0xd7, 0xdf, 0xa9, 0x80, 0xc8, 0xc8, 0xc8,
    0x20, 0xd5, 0xdf, 0xa9, 0xaa, 0x20, 0xd5, 0xdf,
    0xa9, 0x55, 0xa2, 0xaa, 0x88, 0x88, 0x88, 0x20,
    0xd7, 0xdf, 0xad, 0xb7, 0xdf, 0x8d, 0x00, 0xde,
    0xa2, 0x00, 0x8e, 0xed, 0xdf, 0x88, 0x88, 0xa9,
    0x30, 0x20, 0xd7, 0xdf, 0xa9, 0xff, 0xaa, 0xa8,
    0xd0, 0x24, 0xad, 0xf2, 0xdf, 0x8d, 0x00, 0xde,
    0xa0, 0x80, 0xa9, 0xf0, 0x20, 0xd7, 0xdf, 0xa0,
    0xe0, 0xa9, 0xf0, 0x20, 0xd7, 0xdf, 0x28, 0x38,
    0xb0, 0x02, 0x28, 0x18, 0xac, 0xba, 0xdf, 0xae,
    0xb9, 0xdf, 0xad, 0xb7, 0xdf, 0x60, 0x20, 0xec,
    0xdf, 0xf0, 0x09, 0xca, 0xd0, 0xf8, 0x88, 0xd0,
    0xf5, 0x18, 0x90, 0xce, 0xad, 0xf2, 0xdf, 0x8d,
    0x00, 0xde, 0x18, 0x90, 0xdd, 0x8d, 0xf2, 0xdf,
    0x8d, 0x00, 0xde, 0x60, 0xad, 0xf2, 0xdf, 0x60,
    0x8d, 0xf3, 0xdf, 0x8e, 0xe9, 0xdf, 0x8c, 0xea,
    0xdf, 0x60, 0x8e, 0xf4, 0xdf, 0x8c, 0xf5, 0xdf,
    0x8d, 0xf6, 0xdf, 0x60, 0xad, 0xf2, 0xdf, 0x8d,
    0x00, 0xde, 0x20, 0xe8, 0xdf, 0x8d, 0xb7, 0xdf,
    0x8e, 0xf0, 0xdf, 0x8c, 0xf1, 0xdf, 0xa9, 0x00,
    0x8d, 0xba, 0xdf, 0xf0, 0x3b, 0xad, 0xf4, 0xdf,
    0xd0, 0x10, 0xad, 0xf5, 0xdf, 0xd0, 0x08, 0xad,
    0xf6, 0xdf, 0xf0, 0x0b, 0xce, 0xf6, 0xdf, 0xce,
    0xf5, 0xdf, 0xce, 0xf4, 0xdf, 0x90, 0x45, 0x38,
    0xb0, 0x42, 0x8d, 0xb7, 0xdf, 0x8e, 0xf0, 0xdf,
    0x8c, 0xf1, 0xdf, 0xae, 0xe9, 0xdf, 0xad, 0xea,
    0xdf, 0xc9, 0xa0, 0x90, 0x02, 0x09, 0x40, 0xa8,
    0xad, 0xb7, 0xdf, 0x20, 0x80, 0xdf, 0xb0, 0x24,
    0xee, 0xe9, 0xdf, 0xd0, 0x19, 0xee, 0xea, 0xdf,
    0xad, 0xf3, 0xdf, 0x29, 0xe0, 0xcd, 0xea, 0xdf,
    0xd0, 0x0c, 0xad, 0xf3, 0xdf, 0x0a, 0x0a, 0x0a,
    0x8d, 0xea, 0xdf, 0xee, 0xf2, 0xdf, 0x18, 0xad,
    0xba, 0xdf, 0xf0, 0xa1, 0xac, 0xf1, 0xdf, 0xae,
    0xf0, 0xdf, 0xad, 0xb7, 0xdf, 0x60, 0xff, 0xff,
    0xff, 0xff
};

static int easyflash_write = 0;
static int easyflash_write_addr = 0;
static int easyflash_write_flash = 0;

static uint8_t easyflash_tc64_d0fe = 255;
static uint8_t easyflash_tc64_d0fa = 0;
static uint8_t easyflash_tc64_d0a0 = 0;
static uint8_t easyflash_tc64_d0a1 = 0;
static uint8_t easyflash_tc64_d0a2 = 0;
static uint8_t easyflash_tc64_d0a3 = 0;
static uint8_t easyflash_tc64_d0af = 0;


static char eapiNew[768] = {0x00};

static char eapiTc64[768] = { 0 };


/* ---------------------------------------------------------------------*/
static io_source_list_t *easyflash_io1_list_item = NULL;
static io_source_list_t *easyflash_io2_list_item = NULL;
static io_source_list_t *easyflash_d0a0_list_item = NULL;
static io_source_list_t *easyflash_d0f0_list_item = NULL;
static io_source_list_t *easyflash_d700_list_item = NULL;

static uint8_t easyflash_d080_read(uint16_t addr);
static void easyflash_d080_store(uint16_t addr, uint8_t value);
static uint8_t easyflash_d700_read(uint16_t addr);
static void easyflash_d700_store(uint16_t addr, uint8_t value);
static void easyflash_io1_store(uint16_t addr, uint8_t value);
static uint8_t easyflash_io1_read(uint16_t addr);
static uint8_t easyflash_io2_read(uint16_t addr);
static void easyflash_io2_store(uint16_t addr, uint8_t value);
static uint8_t easyflash_io1_peek(uint16_t addr);
static int easyflash_io1_dump(void);


static io_source_t easyflash_d700_device = {
    CARTRIDGE_NAME_EASYFLASH,
    IO_DETACH_CART,
    NULL,
    0xd700, 0xd7ff, 0xff,
    1,
    easyflash_d700_store,
    easyflash_d700_read,
    easyflash_d700_read,
    NULL,
    CARTRIDGE_EASYFLASH,
    0,
    0
};

static io_source_t easyflash_d0a0_device = {
    CARTRIDGE_NAME_EASYFLASH,
    IO_DETACH_CART,
    NULL,
    0xd0a0, 0xd0af, 0x7f,
    1,
    easyflash_d080_store,
    easyflash_d080_read,
    easyflash_d080_read,
    NULL,
    CARTRIDGE_EASYFLASH,
    0,
    0
};

static io_source_t easyflash_d0f0_device = {
    CARTRIDGE_NAME_EASYFLASH,
    IO_DETACH_CART,
    NULL,
    0xd0f1, 0xd0ff, 0x7f,
    1,
    easyflash_d080_store,
    easyflash_d080_read,
    easyflash_d080_read,
    NULL,
    CARTRIDGE_EASYFLASH,
    0,
    0
};

static io_source_t easyflash_io1_device = {
    CARTRIDGE_NAME_EASYFLASH,
    IO_DETACH_CART,
    NULL,
    0xde00, 0xdeff, 0xff,
    1,
    easyflash_io1_store,
    easyflash_io1_read,
    easyflash_io1_peek,
    easyflash_io1_dump,
    CARTRIDGE_EASYFLASH,
    0,
    0
};

static io_source_t easyflash_io2_device = {
    CARTRIDGE_NAME_EASYFLASH,
    IO_DETACH_CART,
    NULL,
    0xdf00, 0xdfff, 0xff,
    1, /* read is always valid */
    easyflash_io2_store,
    easyflash_io2_read,
    easyflash_io2_read, /* same implementation */
    NULL, /* nothing to dump */
    CARTRIDGE_EASYFLASH,
    0,
    0
};

static const export_resource_t export_res = {
    CARTRIDGE_NAME_EASYFLASH, 1, 1, &easyflash_io1_device, &easyflash_io2_device, CARTRIDGE_EASYFLASH
};

static void easyflash_process_uci_command()
{
  int size = easyflash_1541u2_uci_command.size;
	unsigned char target = easyflash_1541u2_uci_command.data[0];
	unsigned char command = easyflash_1541u2_uci_command.data[1];
	unsigned char subcommand = easyflash_1541u2_uci_command.data[2];
	
	if (target == 4 && command == 1 && size >= 2)
   {
      memcpy(easyflash_1541u2_uci_status.data, "00,OK", 5);
      easyflash_1541u2_uci_status.size = 5;
      memcpy(easyflash_1541u2_uci_data.data, "CONTROL TARGET V1.0", 19);
      easyflash_1541u2_uci_data.size = 19;
	}
	else if (target == 4 && command == 0x20 && subcommand == 0 && size >= 5)
   {
      memcpy(easyflash_1541u2_uci_status.data, "00,OK", 5);
      easyflash_1541u2_uci_status.size = 5;
      easyflash_1541u2_uci_data.size = 0;
      unsigned char bank = easyflash_1541u2_uci_command.data[3];
      unsigned char base = easyflash_1541u2_uci_command.data[4];
      int addr = 8192*(bank & 0x38);
      
      if (base & 0x20)
      	for (int i=0; i<65536; i++) romh_banks[addr+i] = 0xff;
      else
      	for (int i=0; i<65536; i++) roml_banks[addr+i] = 0xff;
	}
	else
   {
      memcpy(easyflash_1541u2_uci_status.data, "21,UNKNOWN COMMAND", 19);
      easyflash_1541u2_uci_status.size = 19;
      easyflash_1541u2_uci_data.size = 0;
	}
}

static uint8_t easyflash_d080_read(uint16_t addr)
{
   if (!easyflash_tc64_mode) return 0;

   if (((addr & 0x7f) == 0x7e))
      return easyflash_tc64_d0fe;

   if (((addr & 0x7f) == 0x7a) && (easyflash_tc64_d0fe == 0x2a))
      return easyflash_tc64_d0fa;

   if (((addr & 0x7f) == 0x2f) && (easyflash_tc64_d0fa & 2))
      return easyflash_tc64_d0af;

   if (((addr & 0x7f) == 0x20) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      return easyflash_tc64_d0a0;
   if (((addr & 0x7f) == 0x21) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      return easyflash_tc64_d0a1;
   if (((addr & 0x7f) == 0x22) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      return easyflash_tc64_d0a2;
   if (((addr & 0x7f) == 0x23) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      return easyflash_tc64_d0a3;


   return 0;
}

static void easyflash_d080_store(uint16_t addr, uint8_t value)
{
   if (!easyflash_tc64_mode) return;

// printf("d080.store: %x, %x\n", addr, value);

   if (((addr & 0x7f) == 0x7e) && (value == 0x2a))
      easyflash_tc64_d0fe = 0x2a;
   if (((addr & 0x7f) == 0x7e) && (value == 0xff))
      easyflash_tc64_d0fe = 0xff;

   if (((addr & 0x7f) == 0x7a) && (easyflash_tc64_d0fe == 0x2a))
   {
      easyflash_tc64_d0fa = value;
      if (value & 2)
         if (!easyflash_d0a0_list_item)
	    easyflash_d0a0_list_item = io_source_register(&easyflash_d0a0_device);
      else
      {
         if (easyflash_d0a0_list_item)
            io_source_unregister(easyflash_d0a0_list_item);
         easyflash_d0a0_list_item = 0;
      }

      if (value & 0x20)
         if (!easyflash_d700_list_item)
	    easyflash_d700_list_item = io_source_register(&easyflash_d700_device);
      else
      {
         if (easyflash_d700_list_item)
            io_source_unregister(easyflash_d700_list_item);
         easyflash_d700_list_item = 0;
      }
   }
   if (((addr & 0x7f) == 0x2f) && (easyflash_tc64_d0fa & 2))
      easyflash_tc64_d0af = value;

   if (((addr & 0x7f) == 0x20) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      easyflash_tc64_d0a0 = value;
   if (((addr & 0x7f) == 0x21) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      easyflash_tc64_d0a1 = value;
   if (((addr & 0x7f) == 0x22) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      easyflash_tc64_d0a2 = value;
   if (((addr & 0x7f) == 0x23) && (easyflash_tc64_d0af == 0x27) && (easyflash_tc64_d0fa & 2))
      easyflash_tc64_d0a3 = value;
}


static uint8_t easyflash_d700_read(uint16_t addr)
{
   if (!easyflash_tc64_mode) return 0;
   // return (uint8_t) addr;
   if (!(easyflash_tc64_d0fa & 0x20)) return 0;
   // FIXME???
   if (easyflash_tc64_d0af != 0x27)  return 0;

   uint32_t flashaddr = (((uint32_t)easyflash_tc64_d0a3) << 24) | (((uint32_t)easyflash_tc64_d0a2) << 16) | (((uint32_t)easyflash_tc64_d0a1) << 8) | (((uint32_t)easyflash_tc64_d0a0) << 0);
   flashaddr -= 0xf2f1f0f0;
   flashaddr += addr & 0xff;
   uint32_t a1 = flashaddr & 0x1fff;
   uint32_t a2 = flashaddr & 0x2000;
   uint32_t a3 = flashaddr >> 14;
   
   uint32_t ad = a1 | (a3 << 13);

   if (ad >= 512*1024)  return 0;

   if (a2)
      return romh_banks[ad];
   else
      return roml_banks[ad];
}

static void easyflash_d700_store(uint16_t addr, uint8_t value)
{
   if (!easyflash_tc64_mode) return;

   if (!(easyflash_tc64_d0fa & 0x20)) return;
   if (easyflash_tc64_d0af != 0x27)  return;

//printf("d700.store: %x, %x, %02x%02x%02x%02x\n", addr, value, easyflash_tc64_d0a3, easyflash_tc64_d0a2, easyflash_tc64_d0a1, easyflash_tc64_d0a0);

   uint32_t flashaddr = (((uint32_t)easyflash_tc64_d0a3) << 24) | (((uint32_t)easyflash_tc64_d0a2) << 16) | (((uint32_t)easyflash_tc64_d0a1) << 8) | (((uint32_t)easyflash_tc64_d0a0) << 0);
   flashaddr += addr & 0xff;
   uint32_t a1 = flashaddr & 0x1fff;
   uint32_t a2 = flashaddr & 0x2000;
   uint32_t a3 = flashaddr >> 14;
   
   uint32_t ad = a1 | (a3 << 13);

//printf("d700.store: %x, %x, %02x%02x%02x%02x -> %x %x %x %02x\n", addr, value, easyflash_tc64_d0a3, easyflash_tc64_d0a2, easyflash_tc64_d0a1, easyflash_tc64_d0a0, a1, a2, a3, ad);

   if (ad >= 512*1024)  return;

   if (a2)
      romh_banks[ad] = value;
   else
      roml_banks[ad] = value;
}



static void easyflash_io1_store(uint16_t addr, uint8_t value)
{
    uint8_t mem_mode;
    // printf("DEBUG: addr&255=%i, value=%i\n", addr & 255, value);
    if (!easyflash_1541u2_mode)
    	 addr &= 2;
    switch (addr & 255) {
        case 0:
            /* bank register */
            easyflash_register_00 = (uint8_t)(value & EASYFLASH_BANK_MASK);
            break;
        case 2:
            /* mode register */
            easyflash_register_02 = value & 0x87; /* we only remember led, mode, exrom, game */
            mem_mode = easyflash_memconfig[(easyflash_jumper << 3) | (easyflash_register_02 & 0x07)];
            cart_config_changed_slotmain(mem_mode, mem_mode, CMODE_READ);
            /* TODO: change led */
            /* (value & 0x80) -> led on if true, led off if false */
            break;
        case 9:
        	easyflash_write = 0;
        	break;
        case 8:
        	if ( easyflash_write == 0 && value == 0x65)  easyflash_write = 1;
        	else if ( easyflash_write == 1 && value == 0x66)  easyflash_write = 2;
        	else if ( easyflash_write == 1)  easyflash_write = 0;
        	else if ( easyflash_write == 2 && value == 0x77)  easyflash_write = 3;
        	else if ( easyflash_write == 2)  easyflash_write = 0;
         else if ( easyflash_write == 3)
        	{
        		easyflash_write_addr = (easyflash_write_addr & ~0xff) | value;
        		easyflash_write = 4;
        	}
        	else if ( easyflash_write == 4)
        	{
        		easyflash_write_addr = (easyflash_write_addr & ~0x1f00) | (((unsigned int)value)<<8);
        		easyflash_write_flash = (value >> 5) & 1;
        		easyflash_write = 5;
        	}
                else if ( easyflash_write == 5)
        	{
        		easyflash_write_addr = (easyflash_write_addr & ~ (0x3f << 13)) | (((unsigned int)value)<<13);
        		easyflash_write = 6;
        	}
        	else
        		easyflash_write = 0;
        	break;
        case 7:
            if (easyflash_write == 6)
            	{
        	//printf("Updating flash\n");
            	if (easyflash_write_flash)
            		romh_banks[easyflash_write_addr] = value;
            	else
            		roml_banks[easyflash_write_addr] = value;
            	}
        	break;
            	
        case 0x1D:
        	 if (easyflash_1541u2_state)
        	 	   easyflash_1541u2_error = 1;
          else
          	{
          		if (easyflash_1541u2_uci_command.size < 895)
          		   easyflash_1541u2_uci_command.data[easyflash_1541u2_uci_command.size++] = value;
          		else
          		  easyflash_1541u2_error = 1;
          	}
        	break;

        case 0x1C:
          	{
          		if (value & 8)   easyflash_1541u2_error = 0;
          		if (value & 4) 
          		{
          			if (!easyflash_1541u2_reset)
          			   easyflash_1541u2_reset = 8; 
          		}
          		if ((value & 2) &&(easyflash_1541u2_state == 2))
          		{
          	   		easyflash_1541u2_changestateWait = 4;
          	   		easyflash_1541u2_changestateNewState = 0;
          	   }
          	   if ((value & 1) && (easyflash_1541u2_state == 0))
               {
          	   		easyflash_1541u2_changestateWait = 4;
          	   		easyflash_1541u2_changestateNewState = 2;
                  easyflash_1541u2_state = 1;                                                         
          	   }
          		
          	}
        	break;
    }
//    printf("DEBUG: easyflash_write=%i, easyflash_write_addr=%i, easyflash_write_flash=%i\n", easyflash_write, easyflash_write_addr, easyflash_write_flash);
    cart_romhbank_set_slotmain(easyflash_register_00);
    cart_romlbank_set_slotmain(easyflash_register_00);
    cart_port_config_changed_slotmain();
}

static uint8_t easyflash_io1_read(uint16_t addr)
{
    uint8_t val = 0;
	  if ( (addr & 255) == 7)
            if (easyflash_write == 6)
            	{
//            	printf("Reading flash\n");
//    printf("DEBUG: read: easyflash_write=%i, easyflash_write_addr=%i, easyflash_write_flash=%i\n", easyflash_write, easyflash_write_addr, easyflash_write_flash);

            	if (easyflash_write_flash)
            		val = romh_banks[easyflash_write_addr];
            	else
            		val = roml_banks[easyflash_write_addr];
            	}
	  if ( (addr & 255) == 0x1d && easyflash_1541u2_mode)
	  	   val = 0xC9;
	  if ( (addr & 255) == 0x1e && easyflash_1541u2_mode && (easyflash_1541u2_state == 2))
	  {
	  	   if (easyflash_1541u2_uci_data.pos< easyflash_1541u2_uci_data.size)
	  	   	val = easyflash_1541u2_uci_data.data[easyflash_1541u2_uci_data.pos++];
	  }
	  if ( (addr & 255) == 0x1f && easyflash_1541u2_mode && (easyflash_1541u2_state == 2))
	  {
	  	   if (easyflash_1541u2_uci_status.pos< easyflash_1541u2_uci_status.size)
	  	   	val = easyflash_1541u2_uci_status.data[easyflash_1541u2_uci_status.pos++];
	  }
	  if ( (addr & 255) == 0x1c && easyflash_1541u2_mode)
	  	{
	  		if (easyflash_1541u2_reset > 0)
	  			{
	  				if (easyflash_1541u2_reset > 1)
	  					easyflash_1541u2_reset--;
	  				else
	  				{
    					easyflash_1541u2_data_acc = 0;
    					easyflash_1541u2_error = 0;
    					easyflash_1541u2_state = 0;
    					easyflash_1541u2_reset = 0;
    		
    					easyflash_1541u2_changestateWait = -1;
    					easyflash_1541u2_changestateNewState = -1;
    		
    					easyflash_1541u2_uci_command.size = 0;
    					easyflash_1541u2_uci_command.pos = 0;
	  				}
	  			}
	  		
	  		if (easyflash_1541u2_changestateWait >= 0)
	  			{
	  				if (easyflash_1541u2_changestateWait == 0)
	  				{
	  					easyflash_1541u2_state = easyflash_1541u2_changestateNewState;
	  					easyflash_1541u2_changestateWait = -1;
	  					if (easyflash_1541u2_changestateNewState == 2)
	  						{
	  							easyflash_process_uci_command();
	  							easyflash_1541u2_uci_status.pos = 0;
	  							easyflash_1541u2_uci_data.pos = 0;
	  						}
	  					if (easyflash_1541u2_changestateNewState == 0)
	  						{
	  							easyflash_1541u2_data_acc = 0;
    							easyflash_1541u2_uci_command.size = 0;
    							easyflash_1541u2_uci_command.pos = 0;
	  						}
	  				}
	  				else
	  					easyflash_1541u2_changestateWait--;
	  					
	  			}
	  		int stat = easyflash_1541u2_state << 4;
	  		if (easyflash_1541u2_reset)   stat |= 4;
	  		if (easyflash_1541u2_error)   stat |= 8;
	  		if (easyflash_1541u2_data_acc)   stat |= 2;
	  		// Bit 1 unknown
	  		if ((easyflash_1541u2_state == 2) && (easyflash_1541u2_uci_status.pos< easyflash_1541u2_uci_status.size))   stat |= 0x40;
	  		if ((easyflash_1541u2_state == 2) && (easyflash_1541u2_uci_data.pos< easyflash_1541u2_uci_data.size))   stat |= 0x80;
        val = (unsigned char) stat;
	  	}
            	

//    printf("DEBUG: read: addr&255=%i, val=%i\n", addr & 255, val);
        return val;
}

static uint8_t easyflash_io2_read(uint16_t addr)
{
    return easyflash_ram[addr & 0xff];
}

static void easyflash_io2_store(uint16_t addr, uint8_t value)
{
    easyflash_ram[addr & 0xff] = value;
}

/* ---------------------------------------------------------------------*/

static uint8_t easyflash_io1_peek(uint16_t addr)
{
    return (addr & 2) ? easyflash_register_02 : easyflash_register_00;
}

static int easyflash_io1_dump(void)
{
    mon_out("Mode: %s, Bank: %d, LED %s, jumper %s\n",
            cart_config_string(easyflash_memconfig[(easyflash_jumper << 3) | (easyflash_register_02 & 0x07)]),
            easyflash_register_00,
            (easyflash_register_02 & 0x80) ? "on" : "off",
            easyflash_jumper ? "on" : "off");
    mon_out("EAPI found: %s\n", (memcmp(&romh_banks[0x1800], "eapi", 4) == 0) ? "yes" : "no");
    return 0;
}

/* ---------------------------------------------------------------------*/

static int set_easyflash_jumper(int val, void *param)
{
    easyflash_jumper = val ? 1 : 0;
    return 0;
}

static int set_easyflash_crt_write(int val, void *param)
{
    easyflash_crt_write = val ? 1 : 0;
    return 0;
}

static int set_easyflash_crt_optimize(int val, void *param)
{
    easyflash_crt_optimize = val ? 1 : 0;
    return 0;
}

static int set_easyflash_1541u2_mode(int val, void *param)
{
    easyflash_1541u2_mode = val ? 1 : 0;
    return 0;
}

static int set_easyflash_tc64_mode(int val, void *param)
{
    easyflash_tc64_mode = val ? 1 : 0;
    return 0;
}

static int set_easyflash_replace_eapi(int val, void *param)
{
    easyflash_replace_eapi = val ? 1 : 0;
    return 0;
}

static int easyflash_write_chip_if_not_empty(FILE* fd, crt_chip_header_t *chip, uint8_t *data)
{
    int i;

    for (i = 0; i < chip->size; i++) {
        if ((data[i] != 0xff) || (easyflash_crt_optimize == 0)) {
            if (crt_write_chip(data, chip, fd)) {
                return -1;
            }
            return 0;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------*/

static const resource_int_t resources_int[] = {
    { "EasyFlashJumper", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &easyflash_jumper, set_easyflash_jumper, NULL },
    { "EasyFlashWriteCRT", 1, RES_EVENT_STRICT, (resource_value_t)0,
      &easyflash_crt_write, set_easyflash_crt_write, NULL },
    { "EasyFlashOptimizeCRT", 1, RES_EVENT_STRICT, (resource_value_t)1,
      &easyflash_crt_optimize, set_easyflash_crt_optimize, NULL },
    { "EasyFlash1541U2Mode", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &easyflash_1541u2_mode, set_easyflash_1541u2_mode, NULL },
    { "EasyFlashTC64Mode", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &easyflash_tc64_mode, set_easyflash_tc64_mode, NULL },
    { "EasyFlashReplaceEAPI", 1, RES_EVENT_STRICT, (resource_value_t)1,
      &easyflash_replace_eapi, set_easyflash_replace_eapi, NULL },
    RESOURCE_INT_LIST_END
};

int easyflash_resources_init(void)
{
    return resources_register_int(resources_int);
}

void easyflash_resources_shutdown(void)
{
}

/* ---------------------------------------------------------------------*/

static const cmdline_option_t cmdline_options[] =
{
    { "-easyflashjumper", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashJumper", (resource_value_t)1,
      NULL, "Enable EasyFlash jumper" },
    { "+easyflashjumper", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashJumper", (resource_value_t)0,
      NULL, "Disable EasyFlash jumper" },
    { "-easyflashcrtwrite", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashWriteCRT", (resource_value_t)1,
      NULL, "Enable writing to EasyFlash .crt image" },
    { "+easyflashcrtwrite", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashWriteCRT", (resource_value_t)0,
      NULL, "Disable writing to EasyFlash .crt image" },
    { "-easyflashcrtoptimize", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashOptimizeCRT", (resource_value_t)1,
      NULL, "Enable EasyFlash .crt image optimize on write" },
    { "+easyflashcrtoptimize", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashOptimizeCRT", (resource_value_t)0,
      NULL, "Disable writing to EasyFlash .crt image" },

    { "-easyflash1541u2mode", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlash1541U2Mode", (resource_value_t)1,
      NULL, "Enable 1541 Ultimate II mode" },
    { "+easyflash1541u2mode", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlash1541U2Mode", (resource_value_t)0,
      NULL, "Disable 1541 Ultimate II mode" },

    { "-easyflashtc64mode", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashTC64Mode", (resource_value_t)1,
      NULL, "Enable Turbo Chameleon 64 mode" },
    { "+easyflashtc64mode", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlashTC64Mode", (resource_value_t)0,
      NULL, "Disable Turbo Chameleon 64 mode" },

    { "-easyflashreplaceeapi", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlash1541ReplaceEapi", (resource_value_t)1,
      NULL, "Enable replacing EAPI" },
    { "+easyflashreplaceeapi", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "EasyFlash1541ReplaceEapi", (resource_value_t)0,
      NULL, "Disable replacing EAPI" },

    CMDLINE_LIST_END
};

int easyflash_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ---------------------------------------------------------------------*/

uint8_t easyflash_roml_read(uint16_t addr)
{
    return flash040core_read(easyflash_state_low, (easyflash_register_00 * 0x2000) + (addr & 0x1fff));
}

void easyflash_roml_store(uint16_t addr, uint8_t value)
{
     if (!easyflash_1541u2_mode && !easyflash_tc64_mode)
        flash040core_store(easyflash_state_low, (easyflash_register_00 * 0x2000) + (addr & 0x1fff), value);
}

uint8_t easyflash_romh_read(uint16_t addr)
{
    return flash040core_read(easyflash_state_high, (easyflash_register_00 * 0x2000) + (addr & 0x1fff));
}

void easyflash_romh_store(uint16_t addr, uint8_t value)
{
     if (!easyflash_1541u2_mode && !easyflash_tc64_mode)
        flash040core_store(easyflash_state_high, (easyflash_register_00 * 0x2000) + (addr & 0x1fff), value);
}

void easyflash_mmu_translate(unsigned int addr, uint8_t **base, int *start, int *limit)
{
    if (easyflash_state_high && easyflash_state_high->flash_data &&
        easyflash_state_low && easyflash_state_low->flash_data) {
        switch (addr & 0xe000) {
            case 0xe000:
                if (easyflash_state_high->flash_state == FLASH040_STATE_READ) {
                    *base = easyflash_state_high->flash_data + (easyflash_register_00 * 0x2000) - 0xe000;
                    *start = 0xe000;
                    *limit = 0xfffd;
                    return;
                }
                break;
            case 0xa000:
                if (easyflash_state_high->flash_state == FLASH040_STATE_READ) {
                    *base = easyflash_state_high->flash_data + (easyflash_register_00 * 0x2000) - 0xa000;
                    *start = 0xa000;
                    *limit = 0xbffd;
                    return;
                }
                break;
            case 0x8000:
                if (easyflash_state_low->flash_state == FLASH040_STATE_READ) {
                    *base = easyflash_state_low->flash_data + (easyflash_register_00 * 0x2000) - 0x8000;
                    *start = 0x8000;
                    *limit = 0x9ffd;
                    return;
                }
                break;
            default:
                break;
        }
    }
    *base = NULL;
    *start = 0;
    *limit = 0;
}

/* ---------------------------------------------------------------------*/

/* FIXME: this still needs to be tweaked to match the hardware */
static RAMINITPARAM ramparam = {
    .start_value = 255,
    .value_invert = 2,
    .value_offset = 1,

    .pattern_invert = 0x100,
    .pattern_invert_value = 255,

    .random_start = 0,
    .random_repeat = 0,
    .random_chance = 0,
};

void easyflash_powerup(void)
{
    /* fill easyflash ram with startup value(s). this shall not be zeros, see
     * http://sourceforge.net/p/vice-emu/bugs/469/
     *
     * FIXME: the real hardware likely behaves somewhat differently
     */
    /*memset(easyflash_ram, 0xff, CART_RAM_SIZE);*/
    ram_init_with_pattern(easyflash_ram, CART_RAM_SIZE, &ramparam);
}

void easyflash_config_init(void)
{
    easyflash_io1_store((uint16_t)0xde00, 0);
    easyflash_io1_store((uint16_t)0xde02, 0);
}

void easyflash_config_setup(uint8_t *rawcart)
{
    int i;
    if (!eapiNew[0])
       sysfile_load("eapi1541u2", "DRIVES", eapiNew, 768, 768);
       
    if (!eapiTc64[0])
    {
    	char* tmp = malloc(1000000);
    	memset(tmp, 0, 1000000);
    	sysfile_load("rom-menu.bin", "DRIVES", tmp, 768, 1000000);
    	for (int i=0; i<1000000-768; i++)
    	{
           if ( (tmp[i+0] == eapiam29f040[0]) && (tmp[i+1] == eapiam29f040[1]) && (tmp[i+2] == eapiam29f040[2]) && (tmp[i+3] == eapiam29f040[3]) )
           {
              memcpy(eapiTc64, tmp+i, 768);
              break;
           }
       }
       free(tmp);
    }		

    easyflash_state_low = lib_malloc(sizeof(flash040_context_t));
    easyflash_state_high = lib_malloc(sizeof(flash040_context_t));

    flash040core_init(easyflash_state_low, maincpu_alarm_context, FLASH040_TYPE_B, roml_banks);
    flash040core_init(easyflash_state_high, maincpu_alarm_context, FLASH040_TYPE_B, romh_banks);

    for (i = 0; i < EASYFLASH_N_BANKS; i++) { /* split interleaved low and high banks */
        memcpy(easyflash_state_low->flash_data + i * 0x2000, rawcart + i * 0x4000, 0x2000);
        memcpy(easyflash_state_high->flash_data + i * 0x2000, rawcart + i * 0x4000 + 0x2000, 0x2000);
    }

    /*
     * check for presence of EAPI
     */
    if (memcmp(&romh_banks[0x1800], "eapi", 4) == 0) {
        char eapi[17];
        int k;
        for (k = 0; k < 16; k++) {
            eapi[k] = romh_banks[0x1804 + k] & 0x7f;
        }
        eapi[k] = 0;
        log_message(LOG_DEFAULT, "EF: EAPI found (%s)", eapi);

        if (easyflash_replace_eapi)
	          if (easyflash_1541u2_mode)
               memcpy(romh_banks+0x1800, eapiNew, 768 );
            else if (easyflash_tc64_mode)
               memcpy(romh_banks+0x1800, eapiTc64, 768 );
            else
               memcpy(romh_banks+0x1800, eapiam29f040, 768 );
    } else {
        log_warning(LOG_DEFAULT, "EF: EAPI not found! Are you sure this is a proper EasyFlash image?");
    }
    
    if (easyflash_1541u2_mode)
    	{
    		easyflash_1541u2_data_acc = 0;
    		easyflash_1541u2_error = 0;
    		easyflash_1541u2_state = 0;
    		easyflash_1541u2_reset = 0;
    		
    		easyflash_1541u2_changestateWait = -1;
    		easyflash_1541u2_changestateNewState = -1;
    		
    		easyflash_1541u2_uci_command.size = 0;
    		easyflash_1541u2_uci_command.pos = 0;
    	}
}

/* ---------------------------------------------------------------------*/

static int easyflash_common_attach(const char *filename)
{
    if (export_add(&export_res) < 0) {
        return -1;
    }

    easyflash_io1_list_item = io_source_register(&easyflash_io1_device);
    easyflash_io2_list_item = io_source_register(&easyflash_io2_device);
    if (easyflash_tc64_mode)
    {
       easyflash_d0f0_list_item = io_source_register(&easyflash_d0f0_device);
    }
    else
    {
       easyflash_d0f0_list_item = 0;
    }
    easyflash_d0a0_list_item = 0;
    easyflash_d700_list_item = 0;

    easyflash_filename = lib_strdup(filename);

    return 0;
}

int easyflash_bin_attach(const char *filename, uint8_t *rawcart)
{
    easyflash_filetype = 0;

    if (util_file_load(filename, rawcart, 0x4000 * EASYFLASH_N_BANKS, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }

    easyflash_filetype = CARTRIDGE_FILETYPE_BIN;
    return easyflash_common_attach(filename);
}

int easyflash_crt_attach(FILE *fd, uint8_t *rawcart, const char *filename)
{
    crt_chip_header_t chip;

    easyflash_filetype = 0;
    memset(rawcart, 0xff, 0x100000); /* empty flash */

    while (1) {
        if (crt_read_chip_header(&chip, fd)) {
            break;
        }

        if (chip.size == 0x2000) {
            if (chip.bank >= EASYFLASH_N_BANKS || !(chip.start == 0x8000 || chip.start == 0xa000 || chip.start == 0xe000)) {
                return -1;
            }
            if (crt_read_chip(rawcart, (chip.bank << 14) | (chip.start & 0x2000), &chip, fd)) {
                return -1;
            }
        } else if (chip.size == 0x4000) {
            if (chip.bank >= EASYFLASH_N_BANKS || chip.start != 0x8000) {
                return -1;
            }
            if (crt_read_chip(rawcart, chip.bank << 14, &chip, fd)) {
                return -1;
            }
        } else {
            return -1;
        }
    }

    easyflash_filetype = CARTRIDGE_FILETYPE_CRT;
    return easyflash_common_attach(filename);
}

void easyflash_detach(void)
{
    if (easyflash_crt_write) {
        easyflash_flush_image();
    }
    flash040core_shutdown(easyflash_state_low);
    flash040core_shutdown(easyflash_state_high);
    lib_free(easyflash_state_low);
    lib_free(easyflash_state_high);
    lib_free(easyflash_filename);
    easyflash_filename = NULL;
    io_source_unregister(easyflash_io1_list_item);
    io_source_unregister(easyflash_io2_list_item);
    if (easyflash_d0a0_list_item)
       io_source_unregister(easyflash_d0a0_list_item);
    if (easyflash_d0f0_list_item)
       io_source_unregister(easyflash_d0f0_list_item);
    if (easyflash_d700_list_item)
       io_source_unregister(easyflash_d700_list_item);
    easyflash_io1_list_item = NULL;
    easyflash_io2_list_item = NULL;
    easyflash_d0a0_list_item = NULL;
    easyflash_d0a0_list_item = NULL;
    easyflash_d700_list_item = NULL;
    export_remove(&export_res);
}

int easyflash_flush_image(void)
{
    if (easyflash_filename != NULL) {
        if (easyflash_filetype == CARTRIDGE_FILETYPE_BIN) {
            return easyflash_bin_save(easyflash_filename);
        } else if (easyflash_filetype == CARTRIDGE_FILETYPE_CRT) {
            return easyflash_crt_save(easyflash_filename);
        }
        return -1;
    }
    return -2;
}

int easyflash_bin_save(const char *filename)
{
    FILE *fd;
    int i;
    uint8_t *low;
    uint8_t *high;

    if (filename == NULL) {
        return -1;
    }

    fd = fopen(filename, MODE_WRITE);

    if (fd == NULL) {
        return -1;
    }

    low = easyflash_state_low->flash_data;
    high = easyflash_state_high->flash_data;

    for (i = 0; i < EASYFLASH_N_BANKS; i++, low += 0x2000, high += 0x2000) {
        if ((fwrite(low, 1, 0x2000, fd) != 0x2000) || (fwrite(high, 1, 0x2000, fd) != 0x2000)) {
            fclose(fd);
            return -1;
        }
    }

    fclose(fd);
    return 0;
}

int easyflash_crt_save(const char *filename)
{
    FILE *fd;
    crt_chip_header_t chip;
    uint8_t *data;
    int bank;

    fd = crt_create(filename, CARTRIDGE_EASYFLASH, 1, 0, STRING_EASYFLASH);

    if (fd == NULL) {
        return -1;
    }

    chip.type = 2;
    chip.size = 0x2000;

    for (bank = 0; bank < EASYFLASH_N_BANKS; bank++) {
        chip.bank = bank;

        data = easyflash_state_low->flash_data + bank * 0x2000;
        chip.start = 0x8000;
        if (easyflash_write_chip_if_not_empty(fd, &chip, data) != 0) {
            fclose(fd);
            return -1;
        }

        data = easyflash_state_high->flash_data + bank * 0x2000;
        chip.start = 0xa000;
        if (easyflash_write_chip_if_not_empty(fd, &chip, data) != 0) {
            fclose(fd);
            return -1;
        }
    }
    fclose(fd);
    return 0;
}

/* ---------------------------------------------------------------------*/

/* CARTEF snapshot module format:

   type  | name       | description
   --------------------------------
   BYTE  | jumper     | jumper
   BYTE  | register 0 | register 0
   BYTE  | register 2 | register 2
   ARRAY | RAM        | 256 BYTES of RAM data
   ARRAY | ROML       | 524288 BYTES of ROML data
   ARRAY | ROMH       | 524288 BYTES of ROMH data
 */

static const char snap_module_name[] = "CARTEF";
static const char flash_snap_module_name[] = "FLASH040EF";
#define SNAP_MAJOR   0
#define SNAP_MINOR   0

int easyflash_snapshot_write_module(snapshot_t *s)
{
    snapshot_module_t *m;

    m = snapshot_module_create(s, snap_module_name, SNAP_MAJOR, SNAP_MINOR);

    if (m == NULL) {
        return -1;
    }

    if (0
        || (SMW_B(m, (uint8_t)easyflash_jumper) < 0)
        || (SMW_B(m, easyflash_register_00) < 0)
        || (SMW_B(m, easyflash_register_02) < 0)
        || (SMW_BA(m, easyflash_ram, CART_RAM_SIZE) < 0)
        || (SMW_BA(m, roml_banks, 0x80000) < 0)
        || (SMW_BA(m, romh_banks, 0x80000) < 0)) {
        snapshot_module_close(m);
        return -1;
    }

    snapshot_module_close(m);

    if (0
        || (flash040core_snapshot_write_module(s, easyflash_state_low, flash_snap_module_name) < 0)
        || (flash040core_snapshot_write_module(s, easyflash_state_high, flash_snap_module_name) < 0)) {
        return -1;
    }

    return 0;
}

int easyflash_snapshot_read_module(snapshot_t *s)
{
    uint8_t vmajor, vminor;
    snapshot_module_t *m;

    m = snapshot_module_open(s, snap_module_name, &vmajor, &vminor);
    if (m == NULL) {
        return -1;
    }

    /* Do not accept versions higher than current */
    if (snapshot_version_is_bigger(vmajor, vminor, SNAP_MAJOR, SNAP_MINOR)) {
        snapshot_set_error(SNAPSHOT_MODULE_HIGHER_VERSION);
        goto fail;
    }

    if (0
        || (SMR_B_INT(m, &easyflash_jumper) < 0)
        || (SMR_B(m, &easyflash_register_00) < 0)
        || (SMR_B(m, &easyflash_register_02) < 0)
        || (SMR_BA(m, easyflash_ram, CART_RAM_SIZE) < 0)
        || (SMR_BA(m, roml_banks, 0x80000) < 0)
        || (SMR_BA(m, romh_banks, 0x80000) < 0)) {
        goto fail;
    }

    snapshot_module_close(m);

    easyflash_state_low = lib_malloc(sizeof(flash040_context_t));
    easyflash_state_high = lib_malloc(sizeof(flash040_context_t));

    flash040core_init(easyflash_state_low, maincpu_alarm_context, FLASH040_TYPE_B, roml_banks);
    flash040core_init(easyflash_state_high, maincpu_alarm_context, FLASH040_TYPE_B, romh_banks);

    if (0
        || (flash040core_snapshot_read_module(s, easyflash_state_low, flash_snap_module_name) < 0)
        || (flash040core_snapshot_read_module(s, easyflash_state_low, flash_snap_module_name) < 0)) {
        flash040core_shutdown(easyflash_state_low);
        flash040core_shutdown(easyflash_state_high);
        lib_free(easyflash_state_low);
        lib_free(easyflash_state_high);
        return -1;
    }

    easyflash_common_attach("dummy");

    /* remove dummy filename, set filetype to none */
    lib_free(easyflash_filename);
    easyflash_filename = NULL;
    easyflash_filetype = 0;

    return 0;

fail:
    snapshot_module_close(m);
    return -1;
}
