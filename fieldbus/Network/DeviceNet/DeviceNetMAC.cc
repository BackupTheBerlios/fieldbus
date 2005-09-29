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

#include "DeviceNetMAC.h"

/**
 * This is only the implementation the DeviceNetMAC class.
 */

// static members have to be defined again
simtime_t DeviceNetMAC::intervalSOF[2];
int DeviceNetMAC::loggingCounter;

int compare(cObject* obj1, cObject* obj2)
{
	cMessage* msg1 = check_and_cast<cMessage*>(obj1);
	cMessage* msg2 = check_and_cast<cMessage*>(obj2);
	
	if (msg1->priority() == msg2->priority())
		return 0;
	if (msg1->priority() < msg2->priority())
		return 1;
	if (msg1->priority() > msg2->priority())
		return -1;
	return 0;
}

void DeviceNetMAC::initialize()
{
	// DEBUG?
	if (par("debug").boolValue())
		debug = true;
	else
		debug = false;

	maxQueueSize = (int) par("maxQueueSize").longValue();

	queue.setName("queue");
	CompareFunc compareFunc = compare;
	queue.setup(compareFunc, true);

	endSOFMsg = new cMessage("EndSOF", ENDSOF);
	endARBMsg = new cMessage("EndARB", ENDARB);
	endEOFMsg = new cMessage("EndEOF", ENDEOF);
	endERDELMsg = new cMessage("EndERDEL", ENDERDEL);
	endIFSMsg = new cMessage("EndIFS", ENDIFS);

	transmitState = TX_IDLE_STATE;
	retransmittedReady = false;
	intervalSOF[BEGIN] = intervalSOF[END] = -1.0;

	// measurement
	framesSent = 0L;
	framesReceived = 0L;
	bitsSent = 0L;
	bitsReceived = 0L;

	messagesPassed = 0L;
	messagesReceived = 0L;

	messagesDroppedBO = 0L;
	messagesDroppedBE = 0L;

	framesRetransmitted = 0L;

	waitingTime = 0.0;
	blockingTime = 0.0;
	queueWaitingTime = 0.0;
	transmittingTime = 0.0;
	retransmittingTime = 0.0;
	messageDelay = 0.0;

	// measurement initialization
	devicenetMACLoggingModule = parentModule()->submodule("devicenetMACLogging");
	devicenetMACLogging = check_and_cast<DeviceNetMACLogging*>(devicenetMACLoggingModule);
}

void DeviceNetMAC::handleMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleMessage entered\n";

	// handle self messages
	if (msg->isSelfMessage())
	{
		// process different self-messages (timer signals)
		ev << MODULENAME << ": self-message \"" << msg->name() << "\" received\n";
		switch (msg->kind())
		{
			case ENDSOF:
				handleEndSOF();
				break;
			case ENDARB:
				handleEndARB();
				break;
			case ENDEOF:
				handleEndEOF(msg);
				break;
			case ENDERDEL:
				handleEndERDEL();
				break;
			case ENDIFS:
				handleEndIFS();
				break;
		}
		ev << "\n";
		return;
	}

	// register the own address and init the logging
	if (msg->isName("Number"))
	{
		nodeNumber = msg->kind();
		ev << MODULENAME << ": Node number " << nodeNumber << " registred.\n\n";
		initLogging();
		delete msg;
		return;
	}

	// handle messages comming from application
	if (msg->arrivalGate()->isName("UpperLayerIn"))
	{
		handleUpperLayerMessage(msg);
	}
	
	// handle messages comming from bus
	if (msg->arrivalGate()->isName("LowerLayerIn"))
	{
		// busrequest or frame
		if (msg->isName("Arbitration"))
			handleArbitration(msg);
		else
			handleLowerLayerMessage(msg);
	}
	ev << "\n";
}

void DeviceNetMAC::handleUpperLayerMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleUpperLayerMessage entered\n";

	DeviceNetFrame* frame;

	// data or remote frame encapsulation
	if (msg->isName("Data"))
	{
		frame = (DeviceNetFrame*) encapsulateData(msg);
	}
	else
	{
		frame = (DeviceNetFrame*) encapsulateRemote(msg);
	}

	ev << MODULENAME << ": Packet \"" << msg->name() << "\" arrived from higher layers, enqueueing\n";
	if (queue.length() < maxQueueSize)
	{
		frame->setTimestamp(simTime());
		((DeviceNetFrame*) frame)->setTimeStamp(-1.0);
		queue.insert(frame);
	}
	else
	{
		// logging
		messagesDroppedBO++;

		char* name = new char[256];
		if (par("messagesDroppedBOVector").doubleValue() >= 0)
		{
			sprintf(name, "messagesDroppedBO:%i", nodeNumber);
			devicenetMACLogging->setVector(messagesDroppedBO, opp_strdup(name));
		}
		delete name;

		ev << MODULENAME << ": Packet \"" << msg->name() << "\" discarded because the queue is full!\n";
		if (ev.isGUI()) bubble("Packet discarded!");
		delete(frame);
		return;
	}

	// there is no bus activity
	if (transmitState == TX_IDLE_STATE)
	{
		// set SOF interval if this node message is first
		if (intervalSOF[BEGIN] == -1.0 && intervalSOF[END] == -1.0)
		{
			intervalSOF[BEGIN] = simTime();
			intervalSOF[END] = simTime() + (simtime_t) 1 / TXRATE * SOF;
		}

		ev << MODULENAME << ": No incoming carrier signals detected, frame clear to send, wait SOF first\n";
		scheduleEndSOF();
	}
}

void DeviceNetMAC::handleArbitration(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleArbitration entered\n";

	if (transmitState != TX_IDLE_STATE) collection.push_back(msg);

	if (!endARBMsg->isScheduled())
	{
		scheduleAt(simTime() + ARB, endARBMsg);
		ev << MODULENAME << ": Event \"" << endARBMsg->name() << "\" scheduled at t = "
			<< printTime(simTime() + ARB) << endl;
	}
}

void DeviceNetMAC::handleLowerLayerMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleLowerLayerMessage entered\n";

	// shedule events: EOF or ERDEL
	if (!(msg->hasBitError()))
	{
		endEOFMsg->addObject(msg);
		scheduleAt(simTime() + EoF, endEOFMsg);
		ev << MODULENAME << ": Event \"" << endEOFMsg->name() << "\" scheduled at t = "
			<< printTime(simTime() + EoF) << endl;

		if (transmitState != RETRANSMISSION_STATE) transmitState = EOF_STATE;
	}
	else
	{
		delete msg;
		if (ev.isGUI()) bubble("Frame has bit error!");
		scheduleAt(simTime() + ERDEL, endERDELMsg);
		ev << MODULENAME << ": Event \"" << endERDELMsg->name() << "\" scheduled at t = "
			<< printTime(simTime() + ERDEL) << endl;

		if (transmitState != RETRANSMISSION_STATE) transmitState = ERDEL_STATE;
	}
}

cMessage* DeviceNetMAC::encapsulateData(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": encapsulateData entered\n";

	DeviceNetFrame* frame = new DeviceNetFrame("DeviceNetFrame", DATA);
	
	frame->encapsulate(msg);
	frame->setPriority(msg->kind());
	// the length is shorter than the real frame because:
	// * SOF time is used to synchronize send ready nodes
	// * in an error case we have to process before the beginning of the next IFS time
	frame->setLength(RTR + CTRL + msg->length() + CRC + ACK);

	if (debug) frameDetails(frame);
	
	// TODO: set the frame fields with frame->
	frame->setSOF(1);
	frame->setArbitration(msg->kind());
	frame->setRTR(1);
//	setControl(int control_var);
//	setCRC(int CRC_var);

	return (cMessage*) frame;
}

cMessage* DeviceNetMAC::encapsulateRemote(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": encapsulateRemote entered\n";

	DeviceNetFrame* frame = new DeviceNetFrame("DeviceNetFrame", REMOTE);
	
	frame->encapsulate(msg);
	frame->setPriority(msg->kind());
	// the length is shorter than the real frame because:
	// * SOF time is used to synchronize send ready nodes
	// * in an error case we have to process before the beginning of the next IFS time
	frame->setLength(RTR + CTRL + msg->length() + CRC + ACK);

	if (debug) frameDetails(frame);
	
	// TODO: set the frame fields with frame->
	frame->setSOF(1);
	frame->setArbitration(msg->kind());
	frame->setRTR(0);
//	setControl(int control_var);
//	setCRC(int CRC_var);

	return (cMessage*) frame;
}

void DeviceNetMAC::frameDetails(cMessage* msg)
{
	ev << MODULENAME << ": Frame name is " << msg->className() << endl;
	ev << MODULENAME << ": Frame length is (in bit):\n";
	ev << MODULENAME << ": SOF = " << SOF << endl;
	ev << MODULENAME << ": ARB = " << ARB_BITS << endl;
	ev << MODULENAME << ": RTR = " << RTR << endl;
	ev << MODULENAME << ": CTRL = " << CTRL << endl;
	ev << MODULENAME << ": DATA = " << msg->encapsulatedMsg()->length() << endl;
	ev << MODULENAME << ": CRC = " << CRC << endl;
	ev << MODULENAME << ": ACK = " << ACK << endl;
	ev << MODULENAME << ": ----------------\n";
	int sum = RTR + CTRL + msg->encapsulatedMsg()->length() + CRC + ACK;
	ev << MODULENAME << ": Sum = " << sum << " ; ok? "
		<< (sum == msg->length() ? "yes" : "no") << endl;
}

void DeviceNetMAC::scheduleEndSOF()
{
	if (debug) ev << MODULENAME << ": scheduleEndSOF entered\n";

	// a message is to be scheduled by arriving frames from the upper layer if it is comming
	// after the end of SOF, i.e. in the next possible message sending period
	if (simTime() <= intervalSOF[END])
	{
		scheduleAt(intervalSOF[END], endSOFMsg);
		ev << MODULENAME << ": Event \"" << endSOFMsg->name() << "\" scheduled at t = "
			<< printTime(intervalSOF[END]) << endl;
	    transmitState = SOF_STATE;
	}
ev << "transmitState = " << transmitState << endl;
}

void DeviceNetMAC::handleEndSOF()
{
	if (debug) ev << MODULENAME << ": handleEndSOF entered\n";

	// send the arbitration field
	sendingCandidate = (DeviceNetFrame*) queue.tail();
	cMessage* arbitration = new cMessage("Arbitration",
		ARBITRATION, sendingCandidate->length(), sendingCandidate->getArbitration());
	arbitration->addObject(new cMessage("RTR", sendingCandidate->getRTR()));

	// for logging
	if (sendingCandidate->getTimeStamp() < 0)
		sendingCandidate->setTimeStamp(simTime());
	
	send(arbitration, "LowerLayerOut");
		ev << MODULENAME << ": Message \"" << arbitration->name() << "\" send at t = "
			<< printTime(simTime()) << "\n";
    transmitState = ARB_STATE;
}

void DeviceNetMAC::handleEndARB()
{
	if (debug) ev << MODULENAME << ": handleEndARB entered\n";

	if (transmitState == TX_IDLE_STATE)
		return;
	
	// search the winning arbitration message
	cMessage* candidateMessage;
	std::list<cMessage*>::iterator collectionIterator;

	collectionIterator = collection.begin();
	candidateMessage = collection.front();
	collectionIterator++;

	// find the arbitration winner
	while (collectionIterator != collection.end())
	{
		if  (candidateMessage->priority() > ((cMessage*) (*collectionIterator))->priority())
			candidateMessage = (cMessage*) (*collectionIterator);
		else if  (candidateMessage->priority() == ((cMessage*) (*collectionIterator))->priority())
		{
			if (((cMessage*) candidateMessage->getObject("RTR"))->kind() <
				((cMessage*) ((cMessage*) (*collectionIterator))->getObject("RTR"))->kind())
				candidateMessage = (cMessage*) (*collectionIterator);
		}
		collectionIterator++;
	}

	// compare the candidate with our frame in the queue
	if (sendingCandidate->getArbitration() == candidateMessage->priority()
		&& sendingCandidate->getRTR() ==
			((cMessage*) candidateMessage->getObject("RTR"))->kind())
	{
		transmitFrame();
		collection.clear();
	}
	else
	{
		transmitState = RETRANSMISSION_STATE;
		if (debug) ev << MODULENAME << ": Frame delayed\n";
		collection.clear();
	}

	// we have a remote frame which can be deleted
	if (sendingCandidate->getArbitration() == candidateMessage->priority()
		&& sendingCandidate->getRTR() <
			((cMessage*) candidateMessage->getObject("RTR"))->kind())
	{
		delete queue.remove(sendingCandidate);
	}
}

void DeviceNetMAC::transmitFrame()
{
	if (debug) ev << MODULENAME << ": transmitFrame entered\n";

	DeviceNetFrame* origFrame = sendingCandidate;
	DeviceNetFrame* frame = (DeviceNetFrame*) origFrame->dup();

	// logging:
	framesSent++;
	bitsSent += origFrame->length();
	if (retransmittedReady)
	{
		framesRetransmitted++;
	}

	char* name = new char[256];
	if (par("framesSentVector").doubleValue() >= 0)
	{
		sprintf(name, "framesSent:%i", nodeNumber);
		devicenetMACLogging->setVector(framesSent, opp_strdup(name));
	}
	if (par("bitsSentVector").doubleValue() >= 0)
	{
		sprintf(name, "bitsSent:%i", nodeNumber);
		devicenetMACLogging->setVector(bitsSent, opp_strdup(name));
	}
	delete name;

	send(frame, "LowerLayerOut");
	ev << MODULENAME << ": Message \"" << frame->name() << "\" send at t = "
		<< printTime(simTime()) << "\n";

	transmitState = TRANSMITTING_STATE;
}

void DeviceNetMAC::handleEndEOF(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleEndEOF entered\n";

	// send message to the upper layer
	if (!msg->hasObject("DeviceNetFrame")) error("No frame object at end of EOF period");

	DeviceNetFrame* frame = (DeviceNetFrame*) msg->removeObject("DeviceNetFrame");
	int frameLength = frame->length();
	cMessage* data = (cMessage*) frame->decapsulate()->dup();

	// logging
	if (!queue.empty() && transmitState != RETRANSMISSION_STATE && sendingCandidate != NULL)
	{
		if (loggingCounter > 1) error("More than one logging MAC in transmitting state.");

		// time calculation
		if (retransmittedReady && sendingCandidate->hasPar("retransTime"))
		{
			retransmittingTime = simTime() - sendingCandidate->par("retransTime").doubleValue();
			retransmittedReady = false;
			transmittingTime = 0.0;
		}
		else
		{
			retransmittingTime = 0.0;
			transmittingTime = (simtime_t) frameLength / TXRATE;
		}

		if (frame->getTimeStamp() == -1.0)
			blockingTime = 0.0;
		else
		{
			if (fabs(simTime() - EoF - transmittingTime - ARB - /*(simtime_t) SOF / TXRATE -*/
				frame->getTimeStamp())	< par("accuracyFP").doubleValue())
				blockingTime = 0.0;
			else
				blockingTime = simTime() - EoF - transmittingTime - ARB - /*(simtime_t) SOF / TXRATE -*/
					frame->getTimeStamp();
		}
		if (fabs(frame->getTimeStamp() - frame->timestamp()) < par("accuracyFP").doubleValue())
		{
			if (fabs(frame->getTimeStamp() - frame->timestamp()) < par("accuracyFP").doubleValue())
				queueWaitingTime = 0.0;
			else
				queueWaitingTime = frame->getTimeStamp() - frame->timestamp();
		}
		else
		{
			if (fabs(frame->getTimeStamp() - (simtime_t) SOF / TXRATE - 
				frame->timestamp()) < par("accuracyFP").doubleValue())
				queueWaitingTime = 0.0;
			else
				queueWaitingTime = frame->getTimeStamp() - (simtime_t) SOF / TXRATE - 
					frame->timestamp();
		}
		waitingTime = blockingTime + queueWaitingTime;
		simtime_t messageDelay = simTime() + frame->timestamp();

		char* name = new char[256];
		if (par("blockingTimeVector").doubleValue() >= 0)
		{
			sprintf(name, "blockingTime:%i", nodeNumber);
			devicenetMACLogging->setVector(blockingTime, opp_strdup(name));
			blockingTimeStatistics->collect(blockingTime);
		}
		blockingTime = 0.0;
		if (par("queueWaitingTimeVector").doubleValue() >= 0)
		{
			sprintf(name, "queueWaitingTime:%i", nodeNumber);
			devicenetMACLogging->setVector(queueWaitingTime, opp_strdup(name));
			queueWaitingTimeStatistics->collect(queueWaitingTime);
		}
		queueWaitingTime = 0.0;
		if (par("waitingTimeVector").doubleValue() >= 0)
		{
			sprintf(name, "waitingTime:%i", nodeNumber);
			devicenetMACLogging->setVector(waitingTime, opp_strdup(name));
			waitingTimeStatistics->collect(waitingTime);
		}
		waitingTime = 0.0;
		if (par("transmittingTimeVector").doubleValue() >= 0)
		{
			sprintf(name, "transmittingTime:%i", nodeNumber);
			devicenetMACLogging->setVector(transmittingTime +
				(simtime_t) SOF / TXRATE + ARB + EoF, opp_strdup(name));
			transmittingTimeStatistics->collect(transmittingTime +
				(simtime_t) SOF / TXRATE + ARB + EoF);
		}
		transmittingTime = 0.0;
		if (par("retransmittingTimeVector").doubleValue() >= 0)
		{
			sprintf(name, "retransmittingTime:%i", nodeNumber);
			devicenetMACLogging->setVector(retransmittingTime, opp_strdup(name));
			retransmittingTimeStatistics->collect(retransmittingTime);
		}
		retransmittingTime = 0.0;
		if (par("messageDelayVector").doubleValue() >= 0)
		{
			sprintf(name, "messageDelay:%i", nodeNumber);
			devicenetMACLogging->setVector(messageDelay, opp_strdup(name));
			messageDelayStatistics->collect(messageDelay);
		}
		messageDelay = 0.0;
		delete name;
		loggingCounter++;
	}

	// send it only to other nodes upper layer
//queue.empty();
//!queue.empty();
//frame->getArbitration();
//sendingCandidate->getArbitration();
	if (queue.empty() || !queue.empty() && frame->getArbitration() !=
		((sendingCandidate == NULL) ? -1 : sendingCandidate->getArbitration()))
	{
		// logging
		framesReceived++;
		messagesPassed++;
		bitsReceived = bitsReceived + frame->length() + data->length();

		char* name = new char[256];
		if (par("framesReceivedVector").doubleValue() >= 0)
		{
			sprintf(name, "framesReceived:%i", nodeNumber);
			devicenetMACLogging->setVector(framesReceived, opp_strdup(name));
		}
		if (par("messagesPassedVector").doubleValue() >= 0)
		{
			sprintf(name, "messagesPassed:%i", nodeNumber);
			devicenetMACLogging->setVector(messagesPassed, opp_strdup(name));
		}
		if (par("bitsReceivedVector").doubleValue() >= 0)
		{
			sprintf(name, "bitsReceived:%i", nodeNumber);
			devicenetMACLogging->setVector(bitsReceived, opp_strdup(name));
		}
		delete name;

		send(data, "UpperLayerOut");
		ev << MODULENAME << ": Message \"" << data->name() << "\" send at t = "
			<< printTime(simTime()) << "\n";
	}

	delete frame;
	if (!queue.empty() && transmitState != RETRANSMISSION_STATE)
	{
		delete queue.remove(sendingCandidate);
		sendingCandidate = NULL;
	}
	scheduleEndIFS();
}

void DeviceNetMAC::handleEndERDEL()
{
	if (debug) ev << MODULENAME << ": handleEndEOF entered\n";

	// logging
	messagesDroppedBE++;

	// logging helper
	retransmittedReady = true;
	if (sendingCandidate != NULL)
		if (!(sendingCandidate->hasPar("retransTime")));
			sendingCandidate->addPar("retransTime") = simTime() - ERDEL;

	char* name = new char[256];
	if (par("messagesDroppedBEVector").doubleValue() >= 0)
	{
		sprintf(name, "messagesDroppedBE:%i", nodeNumber);
		devicenetMACLogging->setVector(messagesDroppedBE, opp_strdup(name));
	}
	delete name;

	scheduleEndIFS();
}

void DeviceNetMAC::scheduleEndIFS()
{
	if (debug) ev << MODULENAME << ": scheduleEndIFS entered\n";

	scheduleAt(simTime() + IFS, endIFSMsg);
	ev << MODULENAME << ": Event \"" << endIFSMsg->name() << "\" scheduled at t = "
		<< printTime(simTime() + IFS) << endl;
	transmitState = WAIT_IFS_STATE;
	loggingCounter = 0;
}

void DeviceNetMAC::handleEndIFS()
{
	if (debug) ev << MODULENAME << ": handleEndIFS entered\n";

	if (transmitState != WAIT_IFS_STATE) error("Not in WAIT_IFS_STATE at the end of IFS period");

	if (queue.empty())
	{
		transmitState = TX_IDLE_STATE;
		// reset SOF interval
		if (intervalSOF[END] < simTime())
		{
			intervalSOF[BEGIN] = -1.0;
			intervalSOF[END] = -1.0;
		}
	}
	else
	{
		// set SOF interval
		if (intervalSOF[END] < simTime())
		{
			intervalSOF[BEGIN] = simTime();
			intervalSOF[END] = simTime() + (simtime_t) 1 / TXRATE * SOF;
		}
	
		scheduleEndSOF();
	}
}

void DeviceNetMAC::finish()
{
	if (par("waitingTimeStatistics").doubleValue() >= 0)
		devicenetMACLogging->writeStatistics(waitingTimeStatistics);
	if (par("blockingTimeStatistics").doubleValue() >= 0)
		devicenetMACLogging->writeStatistics(blockingTimeStatistics);
	if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
		devicenetMACLogging->writeStatistics(queueWaitingTimeStatistics);
	if (par("transmittingTimeStatistics").doubleValue() >= 0)
		devicenetMACLogging->writeStatistics(transmittingTimeStatistics);
	if (par("retransmittingTimeStatistics").doubleValue() >= 0)
		devicenetMACLogging->writeStatistics(retransmittingTimeStatistics);
	if (par("messageDelayStatistics").doubleValue() >= 0)
		devicenetMACLogging->writeStatistics(messageDelayStatistics);
	devicenetMACLogging->writeMessageTimeDelay();
	if (par("writeScalars").boolValue())
	{
		char* name = new char[256];
		sprintf(name, "unsentMessages:%i", nodeNumber);
		recordScalar(name, queue.length());
		sprintf(name, "framesSent:%i", nodeNumber);
		recordScalar(name, framesSent);
		delete name;
	}
}

DeviceNetMAC::~DeviceNetMAC()
{
	intervalSOF[BEGIN] = intervalSOF[END] = -1.0;
}

void DeviceNetMAC::initLogging()
{
	char* name = new char[256];
	char* description = new char[256];

	if (par("framesSentVector").doubleValue() >= 0)
	{
		// frames sent vector
		sprintf(name, "framesSent:%i", nodeNumber);
		sprintf(description, "'frames sent:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesSentVector").doubleValue());
		devicenetMACLogging->setVector(framesSent, opp_strdup(name));
		if (par("framesSentStatistics").doubleValue() >= 0)
		{
			// frames sent statistics
			sprintf(description, "'frames sent statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesSentStatistics").doubleValue());
		}
	}

	if (par("framesReceivedVector").doubleValue() >= 0)
	{
		// frames received vector
		sprintf(name, "framesReceived:%i", nodeNumber);
		sprintf(description, "'frames received:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesReceivedVector").doubleValue());
		devicenetMACLogging->setVector(framesReceived, opp_strdup(name));
		if (par("framesReceivedStatistics").doubleValue() >= 0)
		{
			// frames received statistics
			sprintf(description, "'frames received statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesReceivedStatistics").doubleValue());
		}
	}

	if (par("bitsSentVector").doubleValue() >= 0)
	{
		// bits sent vector
		sprintf(name, "bitsSent:%i", nodeNumber);
		sprintf(description, "'bits sent:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("bitsSentVector").doubleValue());
		devicenetMACLogging->setVector(bitsSent, opp_strdup(name));
		if (par("bitsSentStatistics").doubleValue() >= 0)
		{
			// bits sent statistics
			sprintf(description, "'bits sent statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("bitsSentStatistics").doubleValue());
		}
	}

	if (par("bitsReceivedVector").doubleValue() >= 0)
	{
		// bits received vector
		sprintf(name, "bitsReceived:%i", nodeNumber);
		sprintf(description, "'bits received:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("bitsReceivedVector").doubleValue());
		devicenetMACLogging->setVector(bitsReceived, opp_strdup(name));
		if (par("bitsReceivedStatistics").doubleValue() >= 0)
		{
			// bits received statistics
			sprintf(description, "'bits received statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("bitsReceivedStatistics").doubleValue());
		}
	}

	if (par("messagesPassedVector").doubleValue() >= 0)
	{
		// messages passed vector
		sprintf(name, "messagesPassed:%i", nodeNumber);
		sprintf(description, "'messages passed:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesPassedVector").doubleValue());
		devicenetMACLogging->setVector(messagesPassed, opp_strdup(name));
		if (par("messagesPassedStatistics").doubleValue() >= 0)
		{
			// messages passed statistics
			sprintf(description, "'messages passed statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesPassedStatistics").doubleValue());
		}
	}

	if (par("messagesReceivedVector").doubleValue() >= 0)
	{
		// messages received vector
		sprintf(name, "messagesReceived:%i", nodeNumber);
		sprintf(description, "'messages received:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesReceivedVector").doubleValue());
		devicenetMACLogging->setVector(messagesReceived, opp_strdup(name));
		if (par("messagesReceivedStatistics").doubleValue() >= 0)
		{
			// messages received statistics
			sprintf(description, "'messages received statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesReceivedStatistics").doubleValue());
		}
	}

	if (par("messagesDroppedBOVector").doubleValue() >= 0)
	{
		// messages dropped - buffer overflow - vector
		sprintf(name, "messagesDroppedBO:%i", nodeNumber);
		sprintf(description, "'messages dropped - buffer overflow:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesDroppedBOVector").doubleValue());
		devicenetMACLogging->setVector(messagesDroppedBO, opp_strdup(name));
		if (par("messagesDroppedBOStatistics").doubleValue() >= 0)
		{
			// messages dropped - buffer overflow - statistics
			sprintf(description, "'messages dropped - buffer overflow - statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesDroppedBOStatistics").doubleValue());
		}
	}

	if (par("messagesDroppedBEVector").doubleValue() >= 0)
	{
		// messages dropped - bit error - vector
		sprintf(name, "messagesDroppedBE:%i", nodeNumber);
		sprintf(description, "'messages dropped - bit error:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesDroppedBEVector").doubleValue());
		devicenetMACLogging->setVector(messagesDroppedBE, opp_strdup(name));
		if (par("messagesDroppedBEStatistics").doubleValue() >= 0)
		{
			// messages dropped - bit error - statistics
			sprintf(description, "'messages dropped - bit error - statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesDroppedBEStatistics").doubleValue());
		}
	}
	
	if (par("framesRetransmittedVector").doubleValue() >= 0)
	{
		// frames retransmitted vector
		sprintf(name, "framesRetransmitted:%i", nodeNumber);
		sprintf(description, "'frames retransmitted:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesRetransmittedVector").doubleValue());
		devicenetMACLogging->setVector(framesRetransmitted, opp_strdup(name));
		if (par("framesRetransmittedStatistics").doubleValue() >= 0)
		{
			// frames retransmitted statistics
			sprintf(description, "'frames retransmitted statistics:%i'", nodeNumber);
			devicenetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesRetransmittedStatistics").doubleValue());
		}
	}

	if (par("waitingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "waitingTime:%i", nodeNumber);
		sprintf(description, "'waiting time:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("waitingTimeVector").doubleValue());
		devicenetMACLogging->setVector(waitingTime, opp_strdup(name));
		if (par("waitingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'waiting time statistics:%i'", nodeNumber);
			waitingTimeStatistics = new cStdDev(opp_strdup(description));
//			waitingTimeStatistics->collect(waitingTime);
		}
	}

	if (par("blockingTimeVector").doubleValue() >= 0)
	{
		// blocking time
		sprintf(name, "blockingTime:%i", nodeNumber);
		sprintf(description, "'blocking  time:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("blockingTimeVector").doubleValue());
		devicenetMACLogging->setVector(blockingTime, opp_strdup(name));
		if (par("blockingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'blocking time statistics:%i'", nodeNumber);
			blockingTimeStatistics = new cStdDev(opp_strdup(description));
//			blockingTimeStatistics->collect(blockingTime);
		}
	}

	if (par("queueWaitingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "queueWaitingTime:%i", nodeNumber);
		sprintf(description, "'queue waiting time:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("queueWaitingTimeVector").doubleValue());
		devicenetMACLogging->setVector(queueWaitingTime, opp_strdup(name));
		if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'queue waiting time statistics:%i'", nodeNumber);
			queueWaitingTimeStatistics = new cStdDev(opp_strdup(description));
//			queueWaitingTimeStatistics->collect(queueWaitingTime);
		}
	}

	if (par("transmittingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "transmittingTime:%i", nodeNumber);
		sprintf(description, "'transmitting time:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("transmittingTimeVector").doubleValue());
		devicenetMACLogging->setVector(transmittingTime, opp_strdup(name));
		if (par("transmittingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'transmitting time statistics:%i'", nodeNumber);
			transmittingTimeStatistics = new cStdDev(opp_strdup(description));
//			transmittingTimeStatistics->collect(transmittingTime);
		}
	}

	if (par("retransmittingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "retransmittingTime:%i", nodeNumber);
		sprintf(description, "'retransmitting time:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("retransmittingTimeVector").doubleValue());
		devicenetMACLogging->setVector(retransmittingTime, opp_strdup(name));
		if (par("retransmittingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'retransmitting time statistics:%i'", nodeNumber);
			retransmittingTimeStatistics = new cStdDev(opp_strdup(description));
//			retransmittingTimeStatistics->collect(retransmittingTime);
		}
	}

	if (par("messageDelayVector").doubleValue() >= 0)
	{
		// message delay vector
		sprintf(name, "messageDelay:%i", nodeNumber);
		sprintf(description, "'message delay:%i'", nodeNumber);
		devicenetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("messageDelayVector").doubleValue());
		devicenetMACLogging->setVector(messageDelay, opp_strdup(name));
		if (par("messageDelayStatistics").doubleValue() >= 0)
		{
			// message delay statistics
			sprintf(description, "'message delay statistics:%i'", nodeNumber);
			messageDelayStatistics = new cStdDev(opp_strdup(description));
//			messageDelayStatistics->collect(messageDelay);
		}
	}

	delete name;
	delete description;
}
