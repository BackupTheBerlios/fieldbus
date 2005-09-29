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

#ifndef MAC_H
#define MAC_H
#include <omnetpp.h>

/*
 * This is a definition file for the MAC.
 */

/**
 * This is the MAC class.
 * 
 * This is the abstract base class for other MAC classes (EthernetMAC, DeviceNetMAC,
 * ControlNetMAC, LonTalkMAC).
 */
class MAC : public cSimpleModule
{
protected:
	/**
	 * The message output queue. Every message send from upper layers is enqueued first
	 * before it can be send to lower layers.
	 */
    cQueue queue;

	/**
	 * Debugging flag. It decides if additional debugging information is printet on output.
	 */
	bool debug;

	/**
	 * Bit rate is the bit per sec. It is the speed  the devices are sending on the bus.
	 */
	double bitrate;
	
protected:
	/**
	 * The class declaration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
	 * See OMNeT's user manual.</i>
	 */
	Module_Class_Members(MAC, cSimpleModule, 0);

	/**
	 * The abstract initialization routine for MAC. Every MAC has to initialize data members.
	 */
	virtual void initialize() = 0;

	/**
	 * The abstract MAC message handle function. Every MAC has to handle messages.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg) = 0;

	/**
	 * The abstract MAC finish member. Every MAC has to write statistics.
	 */
	virtual void finish() = 0;

	/**
	 * Here we handle messages comming from upper layer. 
	 * @param msg is the message object we get
	 */
	virtual void handleUpperLayerMessage(cMessage* msg) = 0;

	/**
	 * A frame comming from lower layer is handled here. 
	 * @param msg is the message object we get
	 */
	virtual void handleLowerLayerMessage(cMessage* msg) = 0;
};

#endif
