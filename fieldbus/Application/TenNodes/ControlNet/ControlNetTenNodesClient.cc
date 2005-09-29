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

#include "ControlNetTenNodesClient.h"

/**
 * This is only the implementation the TenNodesClient class.
 */

void ControlNetTenNodesClient::initialize()
{
	// DEBUG?
	if (par("debug").boolValue())
		debug = true;
	else
		debug = false;

	messagesSent = 0;
	messagesReceived = 0;
	WATCH(messagesSent);
	WATCH(messagesReceived);

	// register MAC Address
	char* name;
	char* position = strrchr(parentModule()->name(), '_');
	position++;
	name = new char[strlen(position) + 1];
	sprintf(name, "%s", position);
	int kind = atoi(name);
	delete name;
	cMessage* addrMsg = new cMessage("MACID", kind);
	addrMsg->setPriority(-10);
	send(addrMsg, "out");

	// initialize first event
	simtime_t sendTime = par("begin").doubleValue();
	cMessage* msg = new cMessage("selfMessage");
	msg->setPriority(-1);
	scheduleAt(simTime() + sendTime, msg);
}

void ControlNetTenNodesClient::handleMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleMessage entered\n";

	if (msg->isSelfMessage())
	{
		if (debug) ev << MODULENAME << ": isSelfMessage entered\n";

		// create new message and schedule it for the next period
		cMessage* reqMsg = new cMessage("SData", SCHEDULED);
		reqMsg->setPriority(-1);
		simtime_t sendTime = par("period").doubleValue();
		reqMsg->setLength(par("length").longValue() * 8);
		scheduleAt(simTime() + sendTime, (cMessage*) reqMsg->dup());
		send(reqMsg, "out");
		messagesSent++;
		ev << MODULENAME << ": Message \"" << reqMsg->name() << "\" send\n";
		delete msg;
	}
	else
	{
		if (debug) ev << MODULENAME << ": NOT isSelfMessage entered\n";
		
		ev << MODULENAME << ": Message \"" << msg->name() << "\" received at t = "
			<< printTime(simTime()) << ", with length = " << msg->length() << endl;
		if (ev.isGUI()) bubble("Message received!");
		messagesReceived++;
		delete msg;
	}
	ev << "\n";
}	

void ControlNetTenNodesClient::finish()
{
	if (par("writeScalars").boolValue())
	{
		recordScalar("messages sent", messagesSent);
		recordScalar("messages received", messagesReceived);
	}
}
