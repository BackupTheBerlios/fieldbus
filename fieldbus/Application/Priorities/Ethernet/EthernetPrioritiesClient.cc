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

#include "EthernetPrioritiesClient.h"

/**
 * This is only the implementation the PrioritiesClient class.
 */

void EthernetPrioritiesClient::initialize()
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
	loggingTime = -1.0;
	onePrior = par("onePrior").longValue();

	// register MAC Address
	char* name;
	char* position = strrchr(parentModule()->name(), '_');
	position++;
	name = new char[strlen(position) + 1];
	sprintf(name, "%s", position);
	int kind = atoi(name);
	delete name;
	cMessage* addrMsg = new cMessage("Address", kind);
	addrMsg->setPriority(-10);
	send(addrMsg, "out");

	// create the logging vector
	name = new char[32];
	sprintf(name, "'priorDelay:%i'", kind);
	outVector = new cOutVector(name);
	stdDev = new cStdDev(name);
	delete name;

	// initialize first events
	if (onePrior == 0)
	{
		scheduleAt(simTime() + par("arriving").doubleValue(), new cMessage("Normal"));
		// additionally priority messages on node 1
		if (kind == 1)
			scheduleAt(simTime() + par("priority").doubleValue(), new cMessage("Prior"));
	}
	else
	{
		// prior node has only prior messages
		if (kind == onePrior)
			scheduleAt(simTime() + par("priority").doubleValue(), new cMessage("Prior"));
		else
			scheduleAt(simTime() + par("arriving").doubleValue(), new cMessage("Normal"));
	}
}

void EthernetPrioritiesClient::handleMessage(cMessage* msg)
{

	if (msg->isSelfMessage())
	{
		if (debug) ev << MODULENAME << ": isSelfMessage entered\n";

		if (msg->isName("Normal"))
		{
			cMessage* reqMsg = new cMessage("Data", 10);
			reqMsg->setPriority(-1);
			reqMsg->setLength(par("length").longValue() * 8);
			reqMsg->setTimestamp();
			scheduleAt(simTime() + par("arriving").doubleValue(), new cMessage("Normal"));
			send(reqMsg, "out");
			messagesSent++;
			ev << MODULENAME << ": Message \"" << reqMsg->name() << "\" send\n";
			delete msg;
		}
		else
		{
			cMessage* reqMsg = new cMessage("Data", -1);
			reqMsg->addObject(new cMessage("prior", this->id()));
			reqMsg->setPriority(-1);
			reqMsg->setLength(par("length").longValue() * 8);
			reqMsg->setTimestamp();
			scheduleAt(simTime() + par("priority").doubleValue(), new cMessage("Prior"));
			send(reqMsg, "out");
			messagesSent++;
			ev << MODULENAME << ": Message \"" << reqMsg->name() << "\" send\n";
			delete msg;
		}
	}
	else
	{
		if (debug) ev << MODULENAME << ": NOT isSelfMessage entered\n";
		
		ev << MODULENAME << ": Message \"" << msg->name() << "\" received at t = "
			<< printTime(simTime()) << ", with length = " << msg->length()
				 << ", with priority = " << (msg->hasObject("prior") ? "yes" : "no") << endl;
		if (ev.isGUI()) bubble("Message received!");
		messagesReceived++;
		// logging in sender module
		if (msg->hasObject("prior"))
		{
			EthernetPrioritiesClient* client = check_and_cast<EthernetPrioritiesClient*>
				(simulation.module(((cMessage*)msg->getObject("prior"))->kind()));
			client->record(simTime() - msg->timestamp());
		}
		delete msg;
	}
	ev << "\n";
}	

void EthernetPrioritiesClient::record(simtime_t time)
{
	if (debug) ev << MODULENAME << ": record entered\n";

	Enter_Method("record(simtime_t time)");

	if (fabs(simTime() - loggingTime) >= par("accuracyFP").doubleValue())
	{
		outVector->record(time);
		stdDev->collect(time);
		loggingTime = simTime();
	}
}

void EthernetPrioritiesClient::finish()
{
	std::ofstream statisticsFile(par("fileName").stringValue(), std::ios_base::out | std::ios_base::app);
	
	statisticsFile << "# " << endl;
	statisticsFile << "# " << stdDev->name() << endl;
	statisticsFile << "# " << endl;
	statisticsFile << "Samples collected:\t" << stdDev->samples() << endl;
	statisticsFile << "Sum weights:\t\t" << stdDev->weights() << endl;
	statisticsFile << "Total sum:\t\t" << stdDev->sum() << endl;
	statisticsFile << "Square sum:\t\t" << stdDev->sqrSum() << endl;
	statisticsFile << "Minimum:\t\t" << stdDev->min() << endl;
	statisticsFile << "Maximum:\t\t" << stdDev->max() << endl;
	statisticsFile << "Mean:\t\t\t" << stdDev->mean() << endl;
	statisticsFile << "Standard deviation:\t" << stdDev->stddev() << endl;
	statisticsFile << "Variance:\t\t" << stdDev->variance() << endl;
	statisticsFile << "\n " << endl;
	statisticsFile.close();

	if (par("writeScalars").boolValue())
	{
		recordScalar("messages sent", messagesSent);
		recordScalar("messages received", messagesReceived);
	}
}
