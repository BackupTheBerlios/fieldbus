/*
 * Copyright (C) 2005 Lehrstuhl fuer praktische Informatik, Universitaet Dortmund
 * Copyright (C) 2005 Dragan Isakovic
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "Bus.h"

/**
 * This is one implementation file of the Bus class.
 */

void Bus::initialize()
{
	// DEBUG?
	if (par("debug").boolValue())
		debug = true;
	else
		debug = false;
		
//	snapshot(this);
		
	// switch to the right protocol
	switch ((int) par("protocol").longValue())
	{
		case ETHERNET:
			Bus::EthernetInitialize();
			break;
		case DEVICENET:
			Bus::DeviceNetInitialize();
			break;
		case CONTROLNET:
			Bus::ControlNetInitialize();
			break;
		case LONTALK:
			Bus::LonTalkInitialize();
			break;
	}
}

void Bus::handleMessage(cMessage * msg)
{
//	snapshot(this);
		
	// switch to the right protocol
	switch ((int) par("protocol").longValue())
	{
		case ETHERNET:
			Bus::EthernetHandleMessage(msg);
			break;
		case DEVICENET:
			Bus::DeviceNetHandleMessage(msg);
			break;
		case CONTROLNET:
			Bus::ControlNetHandleMessage(msg);
			break;
		case LONTALK:
			Bus::LonTalkHandleMessage(msg);
			break;
	}
}

void Bus::finish()
{
	// switch to the right protocol
	switch ((int) par("protocol").longValue())
	{
		case ETHERNET:
			Bus::EthernetFinish();
			break;
		case DEVICENET:
			Bus::DeviceNetFinish();
			break;
		case CONTROLNET:
			Bus::ControlNetFinish();
			break;
		case LONTALK:
			Bus::LonTalkFinish();
			break;
	}
}
