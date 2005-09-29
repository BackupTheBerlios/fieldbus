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

void Bus::DeviceNetInitialize()
{
	messages = 0;
	bitrate = 0;
	protocol = 0;
	taps = 0;
	WATCH(messages);
	if (par("biterrors").boolValue())
	{
		biterrors = 0;
		WATCH(biterrors);
	}
	bitrate = par("bitrate").doubleValue();
	protocol = par("protocol").longValue();

	// initialize the positions where the hosts connects to the bus
	taps = gate("in", 0)->size();
	if (gate("out", 0)->size() != taps)
		error("the sizes of the in[] and out[] gate vectors must be the same");

	// read positions and check if positions are defined in order (we're lazy to sort...)
	std::vector<double> pos;
	tokenize(par("positions").stringValue(), pos);
	int numPos = pos.size();
	if (numPos > taps)
		ev << MODULENAME << ": Note: `positions' parameter contains more values (" << numPos << ") than "
			"the number of taps (" << taps << "), ignoring excess values.\n";
	else if (numPos < taps && numPos >= 2)
		ev << MODULENAME << ": Note: `positions' parameter contains less values (" << numPos << ") than "
			"the number of taps (" << taps << "), repeating distance between last 2 positions.\n";
	else if (numPos < taps && numPos < 2)
		ev << MODULENAME << ": Note: `positions' parameter contains too few values, using 5m distances.\n";

	tap = new BusTap[taps];

	int i;
	double distance = numPos >= 2 ? pos[numPos - 1] - pos[numPos - 2] : 5;
	for (i = 0; i < taps; i++)
	{
		tap[i].id = i;
		tap[i].position = i < numPos ? pos[i] : i == 0 ? 5 : tap[i - 1].position + distance;
	}
	for (i = 0; i < taps - 1; i++)
	{
		if (tap[i].position > tap[i + 1].position)
			error("Tap positions must be ordered in ascending fashion, modify 'positions' parameter and rerun\n");
	}

	// reset all flags
	for (int i = 0; i < taps; i++)
	{
		tap[i].frameInterval[BEGIN] = -1L;
		tap[i].frameInterval[END] = -1L;
		tap[i].frameOnTap = FREE;
		tap[i].endTransmission = NULL;
		tap[i].lastMessageDirection = -1;
		tap[i].lastMessageTime = -1L;
	}
	
	// Prints out data of parameters for parameter checking...
	ev << MODULENAME << ": Parameters of (" << className() << ") " << fullPath() << "\n";
	ev << MODULENAME << ": bitrate: " << bitrate << "\n";
	ev << MODULENAME << ": protocol: " << protocol << "\n";
	ev << "\n";

	// loginit:
	busLoggingModule = parentModule()->submodule("busLogging");
	busLogging = check_and_cast<BusLogging*>(busLoggingModule);

	if (par("messagesVector").doubleValue() >= 0)
	{
		// message vector
		busLogging->initVector("messages", "'messages on bus'", 0L,
			par("messagesVector").doubleValue());
		if (par("messagesStatistics").doubleValue() >= 0)
			busLogging->initStatistics("messages", "'message statistics'",
				par("messagesStatistics").doubleValue());
	}
}

void Bus::DeviceNetHandleMessage(cMessage* msg)
{
	if (debug)
		ev << MODULENAME << ": DeviceNetHandleMessage entered\n";

	// answear the question whether the bus is free
	if (msg->isName("Arbitration"))
	{
		busArbitration(msg);
		ev << "\n";
		return;
	}

	// this is the message delay for the actual bitrate and message length
	simtime_t messageDelay = (simtime_t) 1 / bitrate * msg->length();

	if (!msg->isSelfMessage())
	{
		int tapPoint = msg->arrivalGate()->index();

		if (debug)
		{
			ev << MODULENAME << ": NOT SelfMessage entered\n";
//			printState(tapPoint);
		}

		// Handle frame sent down from the network entity
		ev << MODULENAME << ": Message \"" << msg->name() << "\" arrived on tap "
			<< tapPoint << endl;
		messages++;
		
		// logging:
		if (par("messagesVector").doubleValue() >= 0)
			busLogging->setVector(messages, "messages");

		// the taps are busy now
		for (int i = 0; i < taps; i++)
		{
			tap[i].frameOnTap = BUSY;
			tap[i].frameInterval[BEGIN] = simTime();
			tap[i].frameInterval[END] = simTime() + messageDelay;
		}
			
		// set biterror
		setBiterror(msg);

		// create upstream and downstream events
		if (tapPoint > 0)
		{
			// start UPSTREAM travel
			cMessage* event = new cMessage("upstream", UPSTREAM);
			event->setContextPointer(& tap[tapPoint - 1]);
			// if goes downstream too, we need to make a copy
			cMessage* msg2 = (tapPoint < taps - 1) ? (cMessage*) msg->dup() : msg;
			event->encapsulate(msg2);
			scheduleAt(simTime() + messageDelay, event);
			ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint - 1
				<< " scheduled at t = " << printTime(simTime() + messageDelay) << endl;
		}
		if (tapPoint < taps - 1)
		{
			// start DOWNSTREAM travel
			cMessage* event = new cMessage("downstream", DOWNSTREAM);
			event->setContextPointer(& tap[tapPoint + 1]);
			event->encapsulate(msg);
			scheduleAt(simTime() + messageDelay, event);
			ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint +1
				<< " scheduled at t = " << printTime(simTime() + messageDelay) << endl;
		}

		// one frame is for this tap
		cMessage* event = new cMessage("messageCopy", msg->kind());
		event->setContextPointer(& tap[tapPoint]);
		cMessage* msg0 = (cMessage*) msg->dup();
		event->encapsulate(msg0);
		scheduleAt(simTime() + messageDelay, event);
		ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint
			<< " scheduled at t = " << printTime(simTime() + messageDelay) << endl;
	}
	else
	{
		// handle upstream and downstream events
		int direction = msg->kind();
		BusTap* thistap = (BusTap*) msg->contextPointer();
		int tapPoint = thistap->id;

		if (debug)
		{
			ev << MODULENAME << ": isSelfMessage entered\n";
//			printState(tapPoint);
		}

		// handle the message copy for this tap
		if (msg->isName("messageCopy"))
		{
			cMessage* msg2 = msg->decapsulate();
			delete msg;
			send(msg2, "out", tapPoint);
			ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
				<< tapPoint << ", sending out comlete frame\n\n";
			return;
		}

		ev << MODULENAME << ": message arrives on tap " << tapPoint << endl;

		// send out on gate
		bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == taps - 1);
		cMessage* msg2 = isLast ? msg->decapsulate() : (cMessage*) msg->encapsulatedMsg()->dup();
		send(msg2, "out", tapPoint);
		ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
			<< tapPoint << ", sending out comlete frame\n";

		// if not end of the bus, schedule for next tap
		if (isLast)
		{
			ev << "End of bus reached\n";
			for (int i = 0; i < taps; i++)
			{
				// the taps are free now
				tap[i].frameOnTap = FREE;
				tap[i].frameInterval[BEGIN] = -1L;;
				tap[i].frameInterval[END] = -1L;
			}
			delete msg;
		}
		else
		{
			int nextTap = (direction == UPSTREAM) ? (tapPoint - 1) : (tapPoint + 1);
			msg->setContextPointer(& tap[nextTap]);
			scheduleAt(simTime(), msg);
			ev << MODULENAME << ": Event \"" << msg->name() << "\" for tap " << nextTap
				<< " scheduled at t = " << printTime(simTime()) << endl;
		}
	}
	ev << endl;
}

void Bus::busArbitration(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": busArbitration entered\n";

	for (int i = 0; i < taps; i++)
	{
		cMessage* msg2 = (cMessage*) msg->dup();
		send(msg2, "out", i);
		ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
			<< i << ", sending out arbitration message\n";
	}
	delete msg;
}

void Bus::DeviceNetFinish()
{
}
