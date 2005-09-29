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
 
#include <omnetpp.h>
#include "Util.h"

/*
 * A test client for testing bus functionallity.
 */
class EthernetClient : public cSimpleModule
{
private:
	bool debug;
	long messagesSent;
	long messagesReceived;
	
public:
	Module_Class_Members(EthernetClient, cSimpleModule, 0);
	virtual void initialize();
	virtual void handleMessage(cMessage* msg);
	virtual void finish();
};

Define_Module(EthernetClient);

void EthernetClient::initialize()
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

	char* name;
	char* position = strrchr(parentModule()->name(), '_');
	position++;
	name = new char[strlen(position) + 1];
	sprintf(name, "%s", position);
	int kind = atoi(name);
	delete name;
	cMessage* addrMsg = new cMessage("Address", kind);
	send(addrMsg, "out");

	simtime_t sendTime = par("sendTime").doubleValue();
	cMessage* msg = new cMessage("selfMessage");
	scheduleAt(simTime() + sendTime, msg);
}

void EthernetClient::handleMessage(cMessage* msg)
{

	if (msg->isSelfMessage())
	{
		if (debug) ev << MODULENAME << ": isSelfMessage entered\n";

		cMessage* reqMsg = new cMessage("Request", 10);
		cObject* msgObj = new cObject("Data");
		simtime_t sendTime = par("period").doubleValue();
		reqMsg->addObject(msgObj);
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
			<< printTime(simTime()) << " with data? " << (msg->hasObject("Data") ? "yes" : "no")
				<< ", with length = " << msg->length() << endl;
		if (ev.isGUI()) bubble("Message received!");
		messagesReceived++;
		delete msg;
	}
	ev << "\n";
}	

void EthernetClient::finish()
{
//	if (par("writeScalars").boolValue())
//	{
//		double t = simTime();
//		recordScalar("simulated time", t);
//		recordScalar("messages sent", messagesSent);
//		recordScalar("messages received", messagesReceived);
//		if (t > 0)
//			recordScalar("messages/sec", messagesSent/t);
//	}
}
