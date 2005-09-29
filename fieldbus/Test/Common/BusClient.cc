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
class BusClient : public cSimpleModule
{
private:
	bool debug;
	long messagesSent;
	long messagesReceived;
	
public:
	Module_Class_Members(BusClient, cSimpleModule, 0);
	virtual void initialize();
	virtual void handleMessage(cMessage* msg);
	virtual void finish();
};

Define_Module(BusClient);

void BusClient::initialize()
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
	
	simtime_t sendTime = par("sendTime");
	cMessage* msg = new cMessage("selfMessage");
	scheduleAt(simTime() + sendTime, msg);
}

void BusClient::handleMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleMessage entered\n";

	if (msg->isSelfMessage())
	{
		if (debug) ev << MODULENAME << ": isSelfMessage entered\n";

		// check if bus is busy
		cMessage* checkMsg = new cMessage("Is Bus Free?", BUSREQUEST);
		send(checkMsg, "out");
		ev << MODULENAME << ": Message \"" << checkMsg->name() << "\" send\n";
		delete msg;
	}
	else
	{
		if (debug) ev << MODULENAME << ": NOT isSelfMessage entered\n";
			
		// bus busy answear or getting the frame
		if (msg->isName("Yes!") || msg->isName("No!"))
		{
			// if free send
			if (msg->isName("Yes!"))
			{
				cMessage* message = new cMessage(fullName(), NORMAL);
				message->setLength(1500 * 8);
				send(message, "out");
				ev << MODULENAME << ": Message \"" << message->name() << "\" send at t = "
					<< printTime(simTime()) << "\n";
				if (ev.isGUI()) bubble("Message sent!");
				messagesSent++;
				delete msg;
			}
			else
			{
				ev << MODULENAME << ": Bus is busy and free at: "
					<< printTime(msg->par("Free At").doubleValue()) << endl;
				if (ev.isGUI()) bubble("Bus is busy!");
				delete msg;
			}
		}
		else
		{
			ev << MODULENAME << ": Message \"" << msg->name() << "\" received at t = "
				<< printTime(simTime()) << " with collision? "
					<< (msg->hasObject("collision") ? "yes" : "no") << ", with biterror? "
						<< (msg->hasBitError() ? "yes" : "no") << endl;
			if (ev.isGUI())
				if (msg->hasObject("collision")) bubble("Collision!");
			else
				if (ev.isGUI()) bubble("Message received!");
			messagesReceived++;
			delete msg;
		}
	}
	ev << "\n";
}	

void BusClient::finish()
{
	if (par("writeScalars").boolValue())
	{
		double t = simTime();
		recordScalar("simulated time", t);
		recordScalar("messages sent", messagesSent);
		recordScalar("messages received", messagesReceived);
		if (t > 0)
			recordScalar("messages/sec", messagesSent/t);
	}
}
