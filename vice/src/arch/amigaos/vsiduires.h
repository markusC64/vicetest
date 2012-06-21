/*
 * vsiduires.h
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

#ifndef VICE_VSIDUIRES_H_
#define VICE_VSIDUIRES_H_

#include "uires.h"
#include "intl.h"

static struct TranslateNewMenu UI_MENU_NAME[] = {
  TITLE(IDMS_FILE, NULL)
    ITEM(IDMS_LOAD_PSID_FILE, NULL, IDM_LOAD_PSID_FILE)
    ITEM(IDMS_NEXT_TUNE,      NULL, IDM_NEXT_TUNE)
    ITEM(IDMS_PREVIOUS_TUNE,  NULL, IDM_PREVIOUS_TUNE)
    ITEMSEPARATOR()
    ITEM(IDMS_RESET,          NULL, NULL)
      SUB(IDMS_HARD,          NULL, IDM_RESET_HARD)
      SUB(IDMS_SOFT,          "R",  IDM_RESET_SOFT)
    ITEMSEPARATOR()
    ITEM(IDMS_EXIT,           "X",  IDM_EXIT)

  TITLE(IDMS_MEDIA, NULL)
    ITEM(IDMS_START_SOUND_RECORD, NULL, IDM_SOUND_RECORD_START)
    ITEM(IDMS_STOP_SOUND_RECORD,  NULL, IDM_SOUND_RECORD_STOP)

  TITLE(IDMS_OPTIONS, NULL)
    ITEM(IDMS_REFRESH_RATE,         NULL, NULL)
      SUBTOGGLE(IDMS_AUTO,          NULL, IDM_REFRESH_RATE_AUTO)
      SUBTOGGLE(IDMS_1_1,           NULL, IDM_REFRESH_RATE_1)
      SUBTOGGLE(IDMS_1_2,           NULL, IDM_REFRESH_RATE_2)
      SUBTOGGLE(IDMS_1_3,           NULL, IDM_REFRESH_RATE_3)
      SUBTOGGLE(IDMS_1_4,           NULL, IDM_REFRESH_RATE_4)
      SUBTOGGLE(IDMS_1_5,           NULL, IDM_REFRESH_RATE_5)
      SUBTOGGLE(IDMS_1_6,           NULL, IDM_REFRESH_RATE_6)
      SUBTOGGLE(IDMS_1_7,           NULL, IDM_REFRESH_RATE_7)
      SUBTOGGLE(IDMS_1_8,           NULL, IDM_REFRESH_RATE_8)
      SUBTOGGLE(IDMS_1_9,           NULL, IDM_REFRESH_RATE_9)
      SUBTOGGLE(IDMS_1_10,          NULL, IDM_REFRESH_RATE_10)
  ITEM(IDMS_MAXIMUM_SPEED,          NULL, NULL)
      SUBTOGGLE(IDMS_200_PERCENT,   NULL, IDM_MAXIMUM_SPEED_200)
      SUBTOGGLE(IDMS_100_PERCENT,   NULL, IDM_MAXIMUM_SPEED_100)
      SUBTOGGLE(IDMS_50_PERCENT,    NULL, IDM_MAXIMUM_SPEED_50)
      SUBTOGGLE(IDMS_20_PERCENT,    NULL, IDM_MAXIMUM_SPEED_20)
      SUBTOGGLE(IDMS_10_PERCENT,    NULL, IDM_MAXIMUM_SPEED_10)
      SUBTOGGLE(IDMS_NO_LIMIT,      NULL, IDM_MAXIMUM_SPEED_NO_LIMIT)
      SUBSEPARATOR()
      SUBTOGGLE(IDMS_CUSTOM,        NULL, IDM_MAXIMUM_SPEED_CUSTOM)
    ITEMTOGGLE(IDMS_WARP_MODE,      "W",  IDM_TOGGLE_WARP_MODE)
    ITEMSEPARATOR()
    ITEMTOGGLE(IDMS_SOUND_PLAYBACK, NULL, IDM_TOGGLE_SOUND)

  TITLE(IDMS_SETTINGS, NULL)
    ITEMTOGGLE(IDMS_OVERRIDE_PSID_SETTINGS, NULL, IDM_PSID_OVERRIDE)
    ITEMSEPARATOR()
    ITEMTOGGLE(IDMS_AUDIO_LEAK,             NULL, IDM_TOGGLE_AUDIO_LEAK)
    ITEM(IDMS_SOUND_SETTINGS,               NULL, NULL)
      SUB(IDS_SAMPLE_RATE,                  NULL, IDM_SAMPLE_RATE)
      SUB(IDS_BUFFER_SIZE,                  NULL, IDM_BUFFER_SIZE)
      SUB(IDS_FRAGMENT_SIZE,                NULL, IDM_FRAGMENT_SIZE)
      SUB(IDS_SPEED_ADJUSTMENT,             NULL, IDM_SPEED_ADJUSTMENT)
      SUB(IDS_VOLUME,                       NULL, IDM_VOLUME)
      SUB(IDS_SOUND_OUTPUT_MODE,            NULL, IDM_SOUND_OUTPUT_MODE)
    ITEM(IDMS_SID_SETTINGS,                 NULL, IDM_SID_SETTINGS)
    ITEMSEPARATOR()
    ITEM(IDMS_SAVE_CURRENT_SETTINGS_FILE,   NULL, IDM_SETTINGS_SAVE_FILE)
    ITEM(IDMS_LOAD_SAVED_SETTINGS_FILE,     NULL, IDM_SETTINGS_LOAD_FILE)
    ITEM(IDMS_SAVE_CURRENT_SETTINGS,        NULL, IDM_SETTINGS_SAVE)
    ITEM(IDMS_LOAD_SAVED_SETTINGS,          NULL, IDM_SETTINGS_LOAD)
    ITEM(IDMS_SET_DEFAULT_SETTINGS,         NULL, IDM_SETTINGS_DEFAULT)
    ITEMSEPARATOR()
    ITEMTOGGLE(IDMS_SAVE_SETTING_ON_EXIT,   NULL, IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT)
    ITEMTOGGLE(IDMS_CONFIRM_ON_EXIT,        NULL, IDM_TOGGLE_CONFIRM_ON_EXIT)

  TITLE(IDMS_LANGUAGE, NULL)
    ITEM(IDMS_LANGUAGE_ENGLISH,   NULL, IDM_LANGUAGE_ENGLISH)
    ITEM(IDMS_LANGUAGE_DANISH,    NULL, IDM_LANGUAGE_DANISH)
    ITEM(IDMS_LANGUAGE_GERMAN,    NULL, IDM_LANGUAGE_GERMAN)
    ITEM(IDMS_LANGUAGE_SPANISH,   NULL, IDM_LANGUAGE_SPANISH)
    ITEM(IDMS_LANGUAGE_FRENCH,    NULL, IDM_LANGUAGE_FRENCH)
    ITEM(IDMS_LANGUAGE_HUNGARIAN, NULL, IDM_LANGUAGE_HUNGARIAN)
    ITEM(IDMS_LANGUAGE_ITALIAN,   NULL, IDM_LANGUAGE_ITALIAN)
    ITEM(IDMS_LANGUAGE_KOREAN,    NULL, IDM_LANGUAGE_KOREAN)
    ITEM(IDMS_LANGUAGE_DUTCH,     NULL, IDM_LANGUAGE_DUTCH)
    ITEM(IDMS_LANGUAGE_POLISH,    NULL, IDM_LANGUAGE_POLISH)
    ITEM(IDMS_LANGUAGE_RUSSIAN,   NULL, IDM_LANGUAGE_RUSSIAN)
    ITEM(IDMS_LANGUAGE_SWEDISH,   NULL, IDM_LANGUAGE_SWEDISH)
    ITEM(IDMS_LANGUAGE_TURKISH,   NULL, IDM_LANGUAGE_TURKISH)

  TITLE(IDMS_HELP, NULL)
    ITEM(IDMS_ABOUT,                NULL, IDM_ABOUT)
    ITEMSEPARATOR()
    ITEM(IDMS_COMMAND_LINE_OPTIONS, NULL, IDM_CMDLINE)
    ITEMSEPARATOR()
    ITEM(IDMS_CONTRIBUTORS,         NULL, IDM_CONTRIBUTORS)
    ITEM(IDMS_LICENSE,              NULL, IDM_LICENSE)
    ITEM(IDMS_NO_WARRANTY,          NULL, IDM_WARRANTY)
  END()
};

static struct NewMenu UI_TRANSLATED_MENU_NAME[sizeof(UI_MENU_NAME)/sizeof(UI_MENU_NAME[0])];

#endif /* VICE_VSIDUIRES_H_ */
