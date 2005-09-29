/**
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
 
#ifndef ETHERNETTENNODESCLIENT_H
#define ETHERNETTENNODESCLIENT_H
#include <omnetpp.h>
#include "Util.h"

/**
 * This is the simulation client for the 10-Nodes case in Ethernet. His activity is only
 * controlled by the period range and the beginning time.
 */
class EthernetTenNodesClient : public cSimpleModule
{
private:
	/**
	 * Debugging flag. It decides if additional debugging information is printet on output.
	 */
	bool debug;

	/**
	 * Message counter. It couts the messages sent to the MAC.
	 */
	long messagesSent;

	/**
	 * Message counter. It couts the messages received fromthe MAC.
	 */
	long messagesReceived;
	
public:
	/**
	 * The class declaration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
	 * See OMNeT's user manual.</i>
	 */
	Module_Class_Members(EthernetTenNodesClient, cSimpleModule, 0);

	/**
	 * The initialize function. The first message (event) is created here.
	 */
	virtual void initialize();

	/**
	 * The main handle function. Copies of the first message created are sent to time
	 * points accordingly to the period length.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * The finish function. Messages counters are written in logging files. 
	 */
	virtual void finish();
};

/**
 * The module type registration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
 * See OMNeT's user manual.</i>
 */
Define_Module(EthernetTenNodesClient);

#endif
