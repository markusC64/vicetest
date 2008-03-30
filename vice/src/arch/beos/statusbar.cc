/*
 * statusbar.cc - Implementation of the BeVICE's statusbar
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include <stdio.h>

extern "C" {
#include "datasette.h"
#include "drive.h"
#include "ui.h"
}
#include "statusbar.h"

const rgb_color statusbar_background = {200,200,200,0};
const rgb_color statusbar_green_led = {10,200,10,0};
const rgb_color statusbar_red_led = {200,10,10,0};
const rgb_color statusbar_black_led = {5,5,5,0};
const rgb_color statusbar_motor_on = {250,250,0,0};
const rgb_color statusbar_motor_off = {120,120,120,0};

ViceStatusbar::ViceStatusbar(BRect r) 
	: BView(r,"statusbar",B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW)
{
	BView::SetViewColor(statusbar_background);
	r.OffsetTo(0,0);
	statusbitmap = new BBitmap(r,B_CMAP8,true,true);
	drawview = new BView(r,"drawview",B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	statusbitmap->AddChild(drawview);
	statusbitmap->Lock();
	drawview->SetLowColor(statusbar_background);
	drawview->FillRect(r, B_SOLID_LOW);
	drawview->Sync();
	statusbitmap->Unlock();
	Draw(r);
}


ViceStatusbar::~ViceStatusbar()
{
	statusbitmap->RemoveChild(drawview);
	delete drawview;
	delete statusbitmap;
} 
	

void ViceStatusbar::DisplaySpeed(
	float percent, 
	float framerate, 
	int warp_flag)
{
	char str[256];
	
	if (percent > 9999)
		percent = 9999;
	sprintf(str, "Speed: %.0f%% at %.0ffps %s",
		percent, framerate, (warp_flag? "(warp)":"      "));
	statusbitmap->Lock();
	drawview->SetLowColor(statusbar_background);
	drawview->FillRect(BRect(1,1,150,20), B_SOLID_LOW);
	drawview->DrawString(str, BPoint(5,10));
	drawview->Sync();
	statusbitmap->Unlock();
	Draw(BRect(1,1,150,20));
}


void ViceStatusbar::DisplayDriveStatus(
	int drive_num,
	int drive_led_color,
	double drive_track)
{
	char str[256];
	bool erase_bar=false;
	BRect frame;
	rgb_color led_col;
	
	if (drive_num < 0) {
		erase_bar = true;
		drive_num = -drive_num-1;
	}	 
	frame = BRect(155,1+drive_num*13,215,13+drive_num*13);
	sprintf(str, "%d:  %.1f", drive_num+8, drive_track);
	statusbitmap->Lock();
	drawview->SetLowColor(statusbar_background);
	drawview->FillRect(frame, B_SOLID_LOW);
	if (!erase_bar) {
		drawview->DrawString(str, BPoint(160,10+drive_num*13));
		switch (drive_led_color) {
			case DRIVE_ACTIVE_GREEN:
				led_col = statusbar_green_led;
				break;
			case DRIVE_ACTIVE_RED:
				led_col = statusbar_red_led;
				break;
			default:
				led_col = statusbar_black_led;
		}		
		drawview->SetLowColor(led_col);
		drawview->FillRect(BRect(195,3+drive_num*13,205,9+drive_num*13), B_SOLID_LOW);
	}
	drawview->Sync();
	statusbitmap->Unlock();
	Draw(frame);
}

void ViceStatusbar::DisplayTapeStatus(
	int enabled,
	int counter,int motor,
	int control)
{
	char str[256];
	BRect frame;
	const BPoint play_button[] = {BPoint(198,29), BPoint(201,32), BPoint(198,35)};
	const BPoint ff_button1[] = {BPoint(197,29), BPoint(200,32), BPoint(197,35)};
	const BPoint ff_button2[] = {BPoint(200,29), BPoint(203,32), BPoint(200,35)};
	const BPoint rewind_button1[] = {BPoint(203,29), BPoint(200,32), BPoint(203,35)};
	const BPoint rewind_button2[] = {BPoint(200,29), BPoint(197,32), BPoint(200,35)};

	frame = BRect(155,27,215,39);
	sprintf(str, "T:  %03d", counter);
	statusbitmap->Lock();
	drawview->SetLowColor(statusbar_background);
	drawview->FillRect(frame, B_SOLID_LOW);
	if (enabled) {
		drawview->DrawString(str, BPoint(160,36));
		if (motor)
			drawview->SetLowColor(statusbar_motor_on);
		else
			drawview->SetLowColor(statusbar_motor_off);
		drawview->FillRect(BRect(195, 27, 205, 37), B_SOLID_LOW);
		switch (control) {
			case DATASETTE_CONTROL_STOP:
				drawview->FillRect(BRect(197, 29, 203, 35), B_SOLID_HIGH);
				break;
            case DATASETTE_CONTROL_RECORD:
            	drawview->SetLowColor(statusbar_red_led);
            	drawview->FillEllipse(BRect(208,30,212,34), B_SOLID_LOW);
            case DATASETTE_CONTROL_START:
            	drawview->FillPolygon(play_button, 3);
                break;
            case DATASETTE_CONTROL_REWIND:
            	drawview->StrokePolygon(rewind_button1, 3, false);
            	drawview->StrokePolygon(rewind_button2, 3, false);
                break;
            case DATASETTE_CONTROL_FORWARD:
            	drawview->StrokePolygon(ff_button1, 3, false);
            	drawview->StrokePolygon(ff_button2, 3, false);
                break;
        }

	}
	drawview->Sync();
	statusbitmap->Unlock();
	Draw(frame);
}

void ViceStatusbar::DisplayImage(
	int drive_num,
	const char *image)
{
	BRect frame;
	
	frame = BRect(220,1+drive_num*13,250,13+drive_num*13);
	frame.right = Bounds().Width()-5;
	statusbitmap->Lock();
	drawview->SetLowColor(statusbar_background);
	drawview->FillRect(frame, B_SOLID_LOW);
	if (image)
		drawview->DrawString(image, BPoint(220,10+drive_num*13));
	drawview->Sync();
	statusbitmap->Unlock();
	Draw(frame);
}		

void ViceStatusbar::Draw(BRect rect)
{	
	DrawBitmap(statusbitmap,rect,rect);
}

