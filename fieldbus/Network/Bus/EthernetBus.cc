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

void Bus::EthernetInitialize()
{
	messages = 0;
	propagationSpeed = 0;
	bitrate = 0;
	protocol = 0;
	taps = 0;
	WATCH(messages);
	if (par("biterrors").boolValue())
	{
		biterrors = 0;
		WATCH(biterrors);
	}
	propagationSpeed = par("propagationSpeed").doubleValue();
	bitrate = par("bitrate").doubleValue();
	protocol = (int) par("protocol").longValue();

	// initialize the positions where the hosts connects to the bus
	taps = gate("in", 0)->size();
	if (gate("out", 0)->size() != taps)
		error("Bus: the sizes of the in[] and out[] gate vectors must be the same");
	tap = new BusTap[taps];

	// two cases: normal propagation or constant distance between all taps
	if (!par("unsetPosition"))
	{
		// read positions and check if positions are defined in order (we're lazy to sort...)
		std::vector<double> pos;
		tokenize(par("positions").stringValue(), pos);
		int numPos = pos.size();
		if (numPos > taps)
			ev << MODULENAME << ": Note: `positions' parameter contains more values ("
				<< numPos << ") than the number of taps (" << taps
					<< "), ignoring excess values.\n";
		else if (numPos < taps && numPos >= 2)
			ev << MODULENAME << ": Note: `positions' parameter contains less values ("
				<< numPos << ") than the number of taps (" << taps
					<< "), repeating distance between last 2 positions.\n";
		else if (numPos < taps && numPos < 2)
			ev << MODULENAME << ": Note: `positions' parameter contains too few values, "
				"using 5m distances.\n";
	
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
				error("Bus: Tap positions must be ordered in ascending fashion, modify "
					"'positions' parameter and rerun\n");
		}
	
		// Calculate propagation of delays between tap points on the bus
		for (i = 0; i < taps; i++)
		{
			// Propagation delay between adjacent tap points
			if (i == 0)
			{
				tap[i].propagationDelay[UPSTREAM] = 0;
				tap[i].propagationDelay[DOWNSTREAM] = (tap[i + 1].position - tap[i].position) / propagationSpeed;
			}
			else if (i == taps - 1)
			{
				tap[i].propagationDelay[UPSTREAM] = tap[i - 1].propagationDelay[DOWNSTREAM];
				tap[i].propagationDelay[DOWNSTREAM] = 0;
			}
			else
			{
				tap[i].propagationDelay[UPSTREAM] = tap[i - 1].propagationDelay[DOWNSTREAM];
				tap[i].propagationDelay[DOWNSTREAM] = (tap[i + 1].position - tap[i].position) / propagationSpeed;
			}
		}
		
		// reset all flags
		for (int i = 0; i < taps; i++)
		{
			tap[i].frameInterval[BEGIN] = -1.0;
			tap[i].frameInterval[END] = -1.0;
			tap[i].frameOnTap = FREE;
			tap[i].endTransmission = NULL;
			tap[i].lastMessageDirection = -1;
			tap[i].lastMessageTime = -1.0;
		}
	
		// Prints out data of parameters for parameter checking...
		ev << MODULENAME << ": Parameters of (" << className() << ") " << fullPath() << endl;
		ev << MODULENAME << ": propagationSpeed = " << propagationSpeed << endl;
		ev << MODULENAME << ": bitrate = " << bitrate << endl;
		ev << MODULENAME << ": protocol = " << protocol << endl;
		for (i = 0; i < taps; i++)
		{
			ev << MODULENAME << ": tap[" << i << "].position = " << tap[i].position << 
				"\n     tap[" << i << "].propagationDelay[UPSTREAM] =\t"
					<< printTime(tap[i].propagationDelay[UPSTREAM]) <<
					"\n     tap[" << i << "].propagationDelay[DOWNSTREAM] =\t"
						<< printTime(tap[i].propagationDelay[DOWNSTREAM]) << endl;
		}
	}
	else
	{
		// Calculate propagation of delays between tap points on the bus
		for (int i = 0; i < taps; i++)
		{
			tap[i].propagationDelay[UPSTREAM] = par("distance").longValue() / propagationSpeed;
			tap[i].propagationDelay[DOWNSTREAM] = par("distance").longValue() / propagationSpeed;
			tap[i].position = 0;
			tap[i].id = i;
		}			
	
		// reset all flags
		for (int i = 0; i < taps; i++)
		{
			tap[i].frameInterval[BEGIN] = -1.0;
			tap[i].frameInterval[END] = -1.0;
			tap[i].frameOnTap = FREE;
			tap[i].endTransmission = NULL;
			tap[i].lastMessageDirection = -1;
			tap[i].lastMessageTime = -1.0;
		}
	
		// Prints out data of parameters for parameter checking...
		ev << MODULENAME << ": Parameters of (" << className() << ") " << fullPath() << endl;
		ev << MODULENAME << ": propagationSpeed = " << propagationSpeed << endl;
		ev << MODULENAME << ": bitrate = " << bitrate << endl;
		ev << MODULENAME << ": protocol = " << protocol << endl;
		for (int i = 0; i < taps; i++)
		{
			ev << MODULENAME << ": tap[" << i << "].position = " << tap[i].position << 
				"\n     tap[" << i << "].propagationDelay[UPSTREAM] =\t"
					<< printTime(tap[i].propagationDelay[UPSTREAM]) <<
					"\n     tap[" << i << "].propagationDelay[DOWNSTREAM] =\t"
						<< printTime(tap[i].propagationDelay[DOWNSTREAM]) << endl;
		}
	}
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

void Bus::EthernetHandleMessage(cMessage *msg)
{
	if (debug)
		ev << MODULENAME << ": EthernetHandleMessage entered\n";

	// it's an other handling for constante distance
	if (!par("unsetPosition"))
	{
		// answear the question whether the bus is free
		if (msg->isName("Is Bus Free?"))
		{
			busFree(msg);
			ev << "\n";
			return;
		}

		// check for a message from MAC or a self message
		if (!msg->isSelfMessage())
		{
			handleFrame(msg);
		}
		else
		{
			handleSelfMessage(msg);
		}
		ev << "\n";
	}
	else
	{
		// answear the question whether the bus is free
		if (msg->isName("Is Bus Free?"))
		{
			busFree(msg);
			ev << "\n";
			return;
		}

		if (!msg->isSelfMessage())
		{
			handleConstMessage(msg);
		}
		else
		{
			// case endTransmission
			if (msg->isName("endTransmission"))
			{
				endTransmission(msg);
				ev << "\n";
				return;
			}
			// case collision
			if (msg->isName("Collision"))
			{
				for (int i = 0; i < taps; i++)
				{
					// reset the frame interval on the taps
					tap[i].frameInterval[BEGIN] = -1.0;
					tap[i].frameInterval[END] = -1.0;
	
					// erase endTransmission event if exist
					if (tap[i].endTransmission != NULL)
					{
						delete cancelEvent(tap[i].endTransmission);
						tap[i].endTransmission = NULL;
					}

					cMessage* msg2 = (cMessage*) msg->dup();
					send(msg2, "out", i);
					ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
						<< i << " send\n";
				}
			}
		}
	}
	ev << endl;
}

void Bus::busFree(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": busFree entered\n";

	int tapPoint = msg->arrivalGate()->index();
	cMessage* msg2;

	if (!par("unsetPosition"))
	{
		// can be free or busy: competing state is a free state!
		if (tap[tapPoint].frameOnTap == FREE || tap[tapPoint].frameOnTap == COMPETING)
		{
		 	delete msg;
			msg2 = new cMessage("Yes!");
			msg2->setKind(YES);
			send(msg2, "out", tapPoint);
			ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
				<< tapPoint << " send\n";
		}
		else
		{
		 	delete msg;
			msg2 = new cMessage("No!");
			msg2->addPar("Free At") = tap[tapPoint].frameInterval[END];
			msg2->setKind(NO);
			send(msg2, "out", tapPoint);
			ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
				<< tapPoint << " send\n";
		}
	}
	else
	{
		if (!(simTime() > tap[tapPoint].frameInterval[BEGIN] && simTime() <
				tap[tapPoint].frameInterval[END]))
		{
		 	delete msg;
			msg2 = new cMessage("Yes!");
			msg2->setKind(YES);
			send(msg2, "out", tapPoint);
			ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
				<< tapPoint << " send\n";
		}
		else
		{
		 	delete msg;
			msg2 = new cMessage("No!");
			msg2->addPar("Free At") = tap[tapPoint].frameInterval[END];
			msg2->setKind(NO);
			send(msg2, "out", tapPoint);
			ev << MODULENAME << ": Message \"" << msg2->name() << "\" on tap "
				<< tapPoint << " send\n";
		}
	}
}

void Bus::printState(int tapPoint)
{
	if (debug) ev << MODULENAME << ": printState entered\n";

	ev << MODULENAME << ": message arrives on tap " << tapPoint << endl;
	ev << MODULENAME << ": frameOnTap = " << tap[tapPoint].frameOnTap << endl;
	ev << MODULENAME << ": frameInterval[BEGIN] = " << printTime(tap[tapPoint].frameInterval[BEGIN])
		<< endl;
	ev << MODULENAME << ": frameInterval[END] = " << printTime(tap[tapPoint].frameInterval[END])
		<< endl;
	ev << MODULENAME << ": endTransmission = "
		<< ((tap[tapPoint].endTransmission == NULL) ? "NULL" : tap[tapPoint].endTransmission->name())
			<< endl;
	ev << MODULENAME << ": lastMessageDirection = " << tap[tapPoint].lastMessageDirection << endl;
	ev << MODULENAME << ": lastMessageTime = " << printTime(tap[tapPoint].lastMessageTime) << endl;
}

void Bus::handleFrame(cMessage* msg)
{
	if (debug)
	{
		ev << MODULENAME << ": handleFrame entered\n";
		int tapPoint = msg->arrivalGate()->index();
		printState(tapPoint);
	}

	// this is the message delay for the actual bitrate and message length
	simtime_t messageDelay = (simtime_t) 1 / bitrate * msg->length();

	// set "frame on tap" and the frame interval for this tap
	int tapPoint = msg->arrivalGate()->index();
	tap[tapPoint].frameOnTap = BUSY;
	ev << MODULENAME << ": Message \"" << msg->name() << "\" arrived on tap "
		<< tapPoint << endl;
	messages++;
	tap[tapPoint].frameInterval[BEGIN] = simTime();
	tap[tapPoint].frameInterval[END] = simTime() + messageDelay;

	// logging:
	if (par("messagesVector").doubleValue() >= 0)
		busLogging->setVector(messages, "messages");

	// create upstream and downstream events
	if (tapPoint > 0)
	{
		// start UPSTREAM travel
		tap[tapPoint].lastMessageDirection = UPSTREAM;
		tap[tapPoint].lastMessageTime = simTime();
		cMessage* event = new cMessage("upstream", UPSTREAM);
		event->setLength(msg->length());
		event->setContextPointer(& tap[tapPoint - 1]);
		event->addPar(msg->name()) = ((EthernetFrame*) msg)->getTimeStamp();
		scheduleAt(simTime() + tap[tapPoint].propagationDelay[UPSTREAM], event);
		ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint - 1
			<< " scheduled at t = " << printTime(simTime() + tap[tapPoint].propagationDelay[UPSTREAM])
				<< endl;
	}
	if (tapPoint < taps - 1)
	{
		// start DOWNSTREAM travel
		(tap[tapPoint].lastMessageDirection == UPSTREAM) ?
			tap[tapPoint].lastMessageDirection = BOTHSTREAM:
				tap[tapPoint].lastMessageDirection = DOWNSTREAM;
		tap[tapPoint].lastMessageTime = simTime();
		cMessage* event = new cMessage("downstream", DOWNSTREAM);
		event->setLength(msg->length());
		event->setContextPointer(& tap[tapPoint + 1]);
		event->addPar(msg->name()) = ((EthernetFrame*) msg)->getTimeStamp();
		scheduleAt(simTime() + tap[tapPoint].propagationDelay[DOWNSTREAM], event);
		ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint +1
			<< " scheduled at t = " << printTime(simTime() + tap[tapPoint].propagationDelay[DOWNSTREAM])
				<< endl;
	}

	// create the endTransmission event for this tap
	cMessage* event = new cMessage("endTransmission");
	event->setContextPointer(& tap[tapPoint]);
	event->encapsulate(msg);

	// a pointer: to remove from the event queue in a collision case
	tap[tapPoint].endTransmission = event;
	scheduleAt(simTime() + messageDelay, event);
	ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint
		<< " scheduled at t = " << printTime(simTime() + messageDelay) << endl;
}

void Bus::handleSelfMessage(cMessage* msg)
{
	if (debug)
	{
		ev << MODULENAME << ": handleSelfMessage entered\n";
		BusTap* thisTap = (BusTap*) msg->contextPointer();
		int tapPoint = thisTap->id;
		printState(tapPoint);
	}

	// two cases: normal propagation or constant distance between all taps
	BusTap* thisTap = (BusTap*) msg->contextPointer();
	int tapPoint = thisTap->id;

	// case endTransmission
	if (msg->isName("endTransmission"))
	{
		endTransmission(msg);
		ev << "\n";
		return;
	}

	// no frame on tap = no collision handling, frame on tap = "collision" handling
	if (tap[tapPoint].frameOnTap == FREE)
	{
		handleStream(msg);
	}
	else
	{
		handleCollision(msg);
	}
}

void Bus::endTransmission(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": endTransmission entered\n";

	BusTap* thisTap = (BusTap*) msg->contextPointer();
	int tapPoint = thisTap->id;

	// reset "frame on tap" and the tap message parameters
	tap[tapPoint].frameOnTap = FREE;
	tap[tapPoint].lastMessageDirection = -1;
	tap[tapPoint].lastMessageTime = -1.0;

	// reset the frame interval on this tap
	tap[tapPoint].frameInterval[BEGIN] = -1.0;
	tap[tapPoint].frameInterval[END] = -1.0;
		
	// the first or other endMessage events?
	if (tap[tapPoint].endTransmission != NULL)
	{
		if (debug) ev << MODULENAME << ": first endTransmission entered\n";

		// delete message capsule and reset event pointer
		tap[tapPoint].endTransmission = NULL;
		cMessage* orgMsg = msg->decapsulate();
		delete msg;
		
		// set biterror
		setBiterror(orgMsg);

		// create upstream and downstream events
		if (tapPoint > 0)
		{
			// start UPSTREAM travel
			cMessage* event = new cMessage("endTransmission", UPSTREAM);
			event->setContextPointer(& tap[tapPoint - 1]);
			// if goes downstream too, we need to make a copy
			cMessage* orgMsg2= (tapPoint < taps - 1) ? (cMessage*) orgMsg->dup() : orgMsg;
			event->encapsulate(orgMsg2);
			scheduleAt(simTime() + tap[tapPoint].propagationDelay[UPSTREAM], event);
			ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint - 1
				<< " scheduled at t = " << printTime(simTime() + tap[tapPoint].propagationDelay[UPSTREAM])
					<< endl;
		}
		if (tapPoint < taps - 1)
		{
			// start DOWNSTREAM travel
			cMessage* event = new cMessage("endTransmission", DOWNSTREAM);
			event->setContextPointer(& tap[tapPoint + 1]);
			event->encapsulate(orgMsg);
			scheduleAt(simTime() + tap[tapPoint].propagationDelay[DOWNSTREAM], event);
			ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint + 1
				<< " scheduled at t = " << printTime(simTime() + tap[tapPoint].propagationDelay[DOWNSTREAM])
					<< endl;
		}
	}
	else
	{
		if (debug) ev << MODULENAME << ": other endTransmission entered\n";

		// send message on this tap and delete capsule
		cMessage* msg2 = (cMessage*) msg->dup();
		cMessage* orgMsg = (cMessage*) msg2->decapsulate()->dup();
		delete msg2;
		send(orgMsg, "out", tapPoint);
		ev << MODULENAME << ": Message \"" << orgMsg->name() << "\" on tap "
			<< tapPoint << ", sending out comlete frame\n";

		// create an upstream or a downstream event
		int direction = msg->kind();
		bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == taps - 1);
		if (isLast)
		{
			ev << MODULENAME << ": End of bus reached (no collision)." << endl;
			delete msg;
		}
		else
		{
			int nextTap = (direction == UPSTREAM) ? (tapPoint - 1) : (tapPoint + 1);
			msg->setContextPointer(& tap[nextTap]);
			// two cases: normal propagation and "constant propagation"
			if (!par("unsetPosition"))
			{
				scheduleAt(simTime() + tap[tapPoint].propagationDelay[direction], msg);
				ev << MODULENAME << ": Event \"" << msg->name() << "\" for tap " << nextTap
					<< " (no collision) scheduled at t = "
						<< printTime(simTime() + tap[tapPoint].propagationDelay[direction]) << endl;
			}
			else
			{
				scheduleAt(simTime(), msg);
				ev << MODULENAME << ": Event \"" << msg->name() << "\" for tap " << nextTap
					<< " (no collision) scheduled at t = "
						<< printTime(simTime()) << endl;
			}
		}
	}
}

void Bus::handleStream(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleStream entered\n";

	BusTap* thisTap = (BusTap*) msg->contextPointer();
	int tapPoint = thisTap->id;

	// this is the message delay for the actual bitrate and message length
	simtime_t messageDelay = (simtime_t) 1 / bitrate * msg->length();

	// set "frame on tap" and the frame interval for this tap
	tap[tapPoint].frameOnTap = BUSY;
	ev << MODULENAME << ": Message \"" << msg->name() << "\" arrived on tap "
		<< tapPoint << endl;
	
	// set the frame interval on this tap
	tap[tapPoint].frameInterval[BEGIN] = simTime();
	tap[tapPoint].frameInterval[END] = simTime() + messageDelay;

	// set the tap message parameters
	int direction = msg->kind();
	bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == taps - 1);
	tap[tapPoint].lastMessageDirection = direction;
	tap[tapPoint].lastMessageTime = simTime();

	// create an upstream or a downstream event
	if (isLast)
	{
		ev << MODULENAME << ": End of bus reached (no collision)." << endl;
		delete msg;
	}
	else
	{
		int nextTap = (direction == UPSTREAM) ? (tapPoint - 1) : (tapPoint + 1);
		msg->setContextPointer(& tap[nextTap]);
		// two cases: normal propagation and "constant propagation"
		if (!par("unsetPosition"))
		{
			scheduleAt(simTime() + tap[tapPoint].propagationDelay[direction], msg);
			ev << MODULENAME << ": Event \"" << msg->name() << "\" for tap " << nextTap
				<< " (no collision) scheduled at t = "
					<< printTime(simTime() + tap[tapPoint].propagationDelay[direction]) << endl;
		}
		else
		{
			scheduleAt(simTime(), msg);
			ev << MODULENAME << ": Event \"" << msg->name() << "\" for tap " << nextTap
				<< " (no collision) scheduled at t = "
					<< printTime(simTime()) << endl;
		}
	}
}

void Bus::handleCollision(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleCollision entered (collision?)\n";

	BusTap* thisTap = (BusTap*) msg->contextPointer();
	int tapPoint = thisTap->id;
	int direction = msg->kind();
	bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == taps - 1);
	
	// if tap state is COMPETING we have to check the direction of the last message
	if (tap[tapPoint].frameOnTap == COMPETING)
	{
		handleCompeting(msg);
	}
	else
	{
		handleBusy(msg);
	}
		
	// set the direction for this message on this tap
	tap[tapPoint].lastMessageDirection = direction;
	tap[tapPoint].lastMessageTime = simTime();

	// create an upstream or a downstream event
	if (isLast)
	{
		ev << MODULENAME << ": End of bus reached" << endl;
	}
	else
	{
		int nextTap = (direction == UPSTREAM) ? (tapPoint - 1) : (tapPoint + 1);
		msg->setContextPointer(& tap[nextTap]);
		// two cases: normal propagation and "constant propagation"
		if (!par("unsetPosition"))
		{
			scheduleAt(simTime() + tap[tapPoint].propagationDelay[direction], msg);
			ev << MODULENAME << ": Event \"" << msg->name() << "\" for tap " << nextTap
				<< " scheduled at t = "
					<< printTime(simTime() + tap[tapPoint].propagationDelay[direction]) << endl;
		}
		else
		{
			scheduleAt(simTime(), msg);
			ev << MODULENAME << ": Event \"" << msg->name() << "\" for tap " << nextTap
				<< " scheduled at t = "
					<< printTime(simTime()) << endl;
		}
	}
}

void Bus::handleCompeting(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleCompeting entered (collision?)\n";
	
	BusTap* thisTap = (BusTap*) msg->contextPointer();
	int tapPoint = thisTap->id;
	int direction = msg->kind();
	bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == taps - 1);

	// are we doing a collision with the last message send fom this tap?
	if (direction != tap[tapPoint].lastMessageDirection
		&& (simTime() <= (tap[tapPoint].lastMessageTime
			+ 2 * tap[tapPoint].propagationDelay[!direction])))
	{
		// reset "frame on tap" and the frame interval for this tap
		tap[tapPoint].frameOnTap = COMPETING;
		
		// reset the frame interval on this tap
		tap[tapPoint].frameInterval[BEGIN] = -1.0;
		tap[tapPoint].frameInterval[END] = -1.0;
			
		// erase endTransmission event if exist
		if (tap[tapPoint].endTransmission != NULL)
		{
			delete cancelEvent(tap[tapPoint].endTransmission);
			tap[tapPoint].endTransmission = NULL;
		}
		
		// send a collision message on this tap
		if (!msg->hasObject("collision"))
		{
			cObject* collision = new cObject("collision");
			msg->addObject(collision);
		}
		cMessage* collMsg = isLast ? msg : (cMessage*) msg->dup();
		cArray array = collMsg->parList();
		collMsg->setName(array.get(0)->name());
		collMsg->setKind(COLLISION);
		send(collMsg, "out", tapPoint);
		ev << MODULENAME << ": Message \"" << collMsg->name() << "\" on tap "
			<< tapPoint << ", sending out collided frame\n";
	}
	else
	{
		// check for collision object and send if there is one
		if (msg->hasObject("collision"))
		{
			cMessage* collMsg = isLast ? msg : (cMessage*) msg->dup();
			cArray array = collMsg->parList();
			collMsg->setName(array.get(0)->name());
			collMsg->setKind(COLLISION);
			send(collMsg, "out", tapPoint);
			ev << MODULENAME << ": Message \"" << collMsg->name() << "\" on tap "
				<< tapPoint << ", sending out collided frame\n";
		}
		else
		{
			tap[tapPoint].frameOnTap = BUSY;

			// this is the message delay for the actual bitrate and message length
			simtime_t messageDelay = (simtime_t) 1 / bitrate * msg->length();

			// set the frame interval on this tap
			tap[tapPoint].frameInterval[BEGIN] = simTime();
			tap[tapPoint].frameInterval[END] = simTime() + messageDelay;
		}
	}
}

void Bus::handleBusy(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleBusy entered (collision)\n";

	BusTap* thisTap = (BusTap*) msg->contextPointer();
	int tapPoint = thisTap->id;
	int direction = msg->kind();
	bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == taps - 1);

	// reset "frame on tap" and the frame interval for this tap
	tap[tapPoint].frameOnTap = COMPETING;

	// reset the frame interval on this tap
	tap[tapPoint].frameInterval[BEGIN] = -1.0;
	tap[tapPoint].frameInterval[END] = -1.0;
		
	// erase endTransmission event if exist
	if (tap[tapPoint].endTransmission != NULL)
	{
		delete cancelEvent(tap[tapPoint].endTransmission);
		tap[tapPoint].endTransmission = NULL;
	}
	
	// send a collision message on this tap
	if (!msg->hasObject("collision"))
	{
		cObject* collision = new cObject("collision");
		msg->addObject(collision);
	}
	cMessage* collMsg = isLast ? msg : (cMessage*) msg->dup();
	cArray array = collMsg->parList();
	collMsg->setName(array.get(0)->name());
	collMsg->setKind(COLLISION);
	send(collMsg, "out", tapPoint);
	ev << MODULENAME << ": Message \"" << collMsg->name() << "\" on tap "
		<< tapPoint << ", sending out collided frame\n";
}

void Bus::handleConstMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleConstMessage entered\n";

	int tapPoint = msg->arrivalGate()->index();
	simtime_t messageDelay = (simtime_t) 1 / bitrate * msg->length();

	// if interval is set the message is colliding
	if (tap[tapPoint].frameInterval[BEGIN] < 0 && tap[tapPoint].frameInterval[END]  < 0)
	{
		// set intervals of all taps
		for (int i = 0; i < taps; i++)
		{
			if (i == tapPoint)
			{
				tap[i].frameInterval[BEGIN] = simTime();
				tap[i].frameInterval[END] = simTime() + messageDelay;
			}
			else
			{
				tap[i].frameInterval[BEGIN] = simTime() +
					tap[i].propagationDelay[UPSTREAM];
				tap[i].frameInterval[END] = simTime() +
					tap[i].propagationDelay[DOWNSTREAM] + messageDelay;
			}
		}

		// create the endTransmission event for this tap
		cMessage* event = new cMessage("endTransmission");
		event->setContextPointer(& tap[tapPoint]);
		event->encapsulate(msg);

		// a pointer: to remove from the event queue in a collision case
		tap[tapPoint].endTransmission = event;
		scheduleAt(simTime() + messageDelay, event);
		ev << MODULENAME << ": Event \"" << event->name() << "\" for tap " << tapPoint
			<< " scheduled at t = " << printTime(simTime() + messageDelay) << endl;
	}
	else
	{
		cMessage* collMsg = new cMessage("Collision");
		cObject* collision = new cObject("collision");
		collMsg->addPar(msg->name()) = ((EthernetFrame*) msg)->getTimeStamp();
		collMsg->addObject(collision);
		collMsg->setKind(COLLISION);
		scheduleAt(simTime() + tap[tapPoint].propagationDelay[UPSTREAM], collMsg);
		ev << MODULENAME << ": Event \"" << collMsg->name() << " scheduled at t = "
			<< printTime(simTime() +  tap[tapPoint].propagationDelay[UPSTREAM]) << endl;
	}
}

void Bus::EthernetFinish()
{
//	if (par("writeScalars").boolValue())
//	{
//		double t = simTime();
//		recordScalar("simulated time", t);
//		recordScalar("messages handled", messages);
//		if (t > 0)
//			recordScalar("messages/sec", messages/t);
//		if (par("biterrors").boolValue())
//			recordScalar("biterrors occurred", biterrors);
//	}
}
