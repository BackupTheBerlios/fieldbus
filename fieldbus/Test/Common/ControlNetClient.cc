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
class ControlNetClient : public cSimpleModule
{
private:
	bool debug;
	static int counter;

public:
	Module_Class_Members(ControlNetClient, cSimpleModule, 0);
	virtual void initialize();
	virtual void handleMessage(cMessage* msg);
	virtual void finish();
	~ControlNetClient();
};

Define_Module(ControlNetClient);

int ControlNetClient::counter;

void ControlNetClient::initialize()
{
	// DEBUG?
	if (par("debug").boolValue())
		debug = true;
	else
		debug = false;

	char* name;
	char* position = strrchr(parentModule()->name(), '_');
	position++;
	name = new char[strlen(position) + 1];
	sprintf(name, "%s", position);
	int kind = atoi(name);
	delete name;
	cMessage* addrMsg = new cMessage("MACID", kind);
	send(addrMsg, "out");

	simtime_t sendTime = par("sendTime").doubleValue();
	scheduleAt(simTime() + sendTime, new cMessage("begin"));
}

void ControlNetClient::handleMessage(cMessage* msg)
{
	if (msg->isSelfMessage())
	{
		if (debug) ev << MODULENAME << ": isSelfMessage entered\n";

		delete msg;
		simtime_t sendTime = par("period").doubleValue();

		cMessage* reqMsg0 = new cMessage("SData", 0);
		cObject* msgObj0 = new cObject("Data");
		reqMsg0->addObject(msgObj0);
		reqMsg0->setLength(10/*par("length").longValue()*/ * 8);
		send(reqMsg0, "out");
		ev << MODULENAME << ": Message \"" << reqMsg0->name() << "\" send\n";

		cMessage* reqMsg1 = new cMessage("UData", 1);
		cObject* msgObj1 = new cObject("Data");
		reqMsg1->addObject(msgObj1);
		reqMsg1->setLength(par("length").longValue() * 8);
		send(reqMsg1, "out");
		ev << MODULENAME << ": Message \"" << reqMsg1->name() << "\" send\n";

		scheduleAt(simTime() + sendTime, new cMessage("NewMessage"));
	}
	else
	{
		if (debug) ev << MODULENAME << ": NOT isSelfMessage entered\n";
		
		ev << MODULENAME << ": Message \"" << msg->name() << "\" received at t = "
			<< printTime(simTime()) << " with data? " << (msg->hasObject("Data") ? "yes" : "no")
				<< ", with length = " << msg->length() << endl;
		if (ev.isGUI()) bubble("Message received!");
		delete msg;
	}
	ev << "\n";
}	

void ControlNetClient::finish()
{
}

ControlNetClient::~ControlNetClient()
{
	counter = 0;
}
