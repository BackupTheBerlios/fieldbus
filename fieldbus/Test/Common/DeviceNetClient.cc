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
class DeviceNetClient : public cSimpleModule
{
private:
	bool debug;
	static int counter;

public:
	Module_Class_Members(DeviceNetClient, cSimpleModule, 0);
	virtual void initialize();
	virtual void handleMessage(cMessage* msg);
	virtual void finish();
	~DeviceNetClient();
};

Define_Module(DeviceNetClient);

int DeviceNetClient::counter;

void DeviceNetClient::initialize()
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
	cMessage* addrMsg = new cMessage("Number", kind);
	send(addrMsg, "out");

	simtime_t sendTime = par("sendTime").doubleValue();
	scheduleAt(simTime() + sendTime, new cMessage("begin"));
}

void DeviceNetClient::handleMessage(cMessage* msg)
{
	if (msg->isSelfMessage())
	{
		if (debug) ev << MODULENAME << ": isSelfMessage entered\n";

		cMessage* reqMsg;
//		if (counter >= 6)
//			return;
//		if (counter == 0)
			reqMsg = new cMessage("Data");
//		else
//			reqMsg = new cMessage("Remote");
		cObject* msgObj = new cObject("Data");
		simtime_t sendTime = par("period").doubleValue();
		reqMsg->addObject(msgObj);
		reqMsg->setLength(par("length").longValue() * 8);
		reqMsg->setPriority(par("priority").longValue() - counter);
		scheduleAt(simTime() + sendTime, (cMessage*) reqMsg->dup());
		counter++;
		send(reqMsg, "out");
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
		delete msg;
	}
	ev << "\n";
}	

void DeviceNetClient::finish()
{
}

DeviceNetClient::~DeviceNetClient()
{
	counter = 0;
}
