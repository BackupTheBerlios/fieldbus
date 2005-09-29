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

#include "EthernetMAC.h"

/**
 * This is only the implementation the EthernetMAC class.
 */

void EthernetMAC::initialize()
{
	// DEBUG?
	if (par("debug").boolValue())
		debug = true;
	else
		debug = false;

//	snapshot(this);

//	TODO
//	WATCH();

	maxQueueSize = (int) par("maxQueueSize").longValue();
	isPromiscuous = par("promiscuous").boolValue();
	cd = par("cd").boolValue();

	queue.setName("queue");
    forUs = true;
    backoffs = 0;

	endTxMsg = new cMessage("EndTransmission", ENDTRANSMISSION);
	endJammingMsg = new cMessage("EndJamming", ENDJAMMING);
	retransMsg = new cMessage("Retransmission", RETRANSMISSION);
	endIFGMsg = new cMessage("EndIFG", ENDIFG);
	endBackoffMsg = new cMessage("EndBackoff", ENDBACKOFF);

	transmitState = TX_IDLE_STATE;
    
	// measurement initialization
	framesSent = 0L;
	framesReceived = 0L;
	bitsSent = 0L;
	bitsReceived = 0L;

	messagesPassed = 0L;
	messagesReceived = 0L;

	messagesDroppedBO = 0L;
	messagesDroppedBE = 0L;
	messagesDroppedNFU = 0L;

	collisions = 0L;
	framesRetransmitted = 0L;
	frameRetransmissionFailure = 0L;

	backoffTime = 0.0;
	waitingTime = 0.0;
	blockingTime = 0.0;
	queueWaitingTime = 0.0;

	// measurement initialization
	ethernetMACLoggingModule = parentModule()->submodule("ethernetMACLogging");
	ethernetMACLogging = check_and_cast<EthernetMACLogging*>(ethernetMACLoggingModule);
}

void EthernetMAC::handleMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleMessage entered\n";

//	snapshot(this);

	// handle self messages
	if (msg->isSelfMessage())
	{
		// process different self-messages (timer signals)
		ev << MODULENAME << ": self-message \"" << msg->name() << "\" received\n";
		switch (msg->kind())
		{
			case ENDIFG:
				handleEndIFG();
				break;
			case ENDTRANSMISSION:
				handleEndTx();
				break;
			case RETRANSMISSION:
				handleRetransmission();
				break;
			case ENDJAMMING:
				handleBackoff();
				break;
			case ENDBACKOFF:
				handleEndBackoff();
				break;
		}
		ev << "\n";
		return;
	}

	// register the own address and init the logging
	if (msg->isName("Address"))
	{
		MACAddress = msg->kind();
		ev << MODULENAME << ": MAC address " << MACAddress << " registred.\n\n";
		initLogging();
		delete msg;
		return;
	}

	// handle messages comming from application
	if (msg->arrivalGate()->isName("UpperLayerIn"))
	{
		// logging
		messagesReceived++;

		char* name = new char[256];
		if (par("messagesReceivedVector").doubleValue() >= 0)
		{
			sprintf(name, "messagesReceived:%i", MACAddress);
			ethernetMACLogging->setVector(messagesReceived, opp_strdup(name));
		}
		delete name;

		handleUpperLayerMessage(msg);
	}
	
	// handle messages comming from bus
	if (msg->arrivalGate()->isName("LowerLayerIn"))
	{
		// busrequest or frame
		if (msg->isName("Yes!") || msg->isName("No!"))
			handleBusRequest(msg);
		else
			handleLowerLayerMessage(msg);
	}
	ev << "\n";
}

void EthernetMAC::handleUpperLayerMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleUpperLayerMessage entered\n";

	// encapsulate message and enqueue
	cMessage* frame = encapsulateMessage(msg);
	
	ev << MODULENAME << ": Packet \"" << msg->name() << "\" arrived from higher layers, enqueueing\n";
	if (queue.length() < maxQueueSize)
	{
		frame->setTimestamp(simTime());
		((EthernetFrame*) frame)->setTimeStamp(-1.0);
		queue.insert(frame);
	}
	else
	{
		// logging
		messagesDroppedBO++;

		char* name = new char[256];
		if (par("messagesDroppedBOVector").doubleValue() >= 0)
		{
			sprintf(name, "messagesDroppedBO:%i", MACAddress);
			ethernetMACLogging->setVector(messagesDroppedBO, opp_strdup(name));
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
		ev << MODULENAME << ": No incoming carrier signals detected, frame clear to send, wait IFG first\n";
		scheduleEndIFG();
	}
}

void EthernetMAC::handleLowerLayerMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleLowerLayerMessage entered\n";

	ev << MODULENAME << ": Message \"" << msg->name() << "\" received at t = "
		<< printTime(simTime()) << " with collision? "
			<< (msg->hasObject("collision") ? "yes" : "no") << ", with biterror? "
				<< (msg->hasBitError() ? "yes" : "no") << endl;

	// promiscuous mode? note: collided frames have no readable fields!
	EthernetFrame* frame = (EthernetFrame*) msg;
	if (!isPromiscuous && !msg->hasObject("collision"))
	{
		// check frame's destination
		forUs = (frame->getDstAddress() == MACAddress);
	}
	
	if (forUs)
	{
		// send everything to upper layer
		if (!msg->hasObject("collision"))
		{
			// bit error?
			if (msg->hasBitError())
			{
				// logging
				messagesDroppedBE++;

				char* name = new char[256];
				if (par("messagesDroppedBEVector").doubleValue() >= 0)
				{
					sprintf(name, "messagesDroppedBE:%i", MACAddress);
					ethernetMACLogging->setVector(messagesDroppedBE, opp_strdup(name));
				}
				delete name;

				if (ev.isGUI()) bubble("Frame has bit error!");
			}
			else
			{
				// logging
				framesReceived++;
				messagesPassed++;
				bitsReceived += msg->length();

				char* name = new char[256];
				if (par("framesReceivedVector").doubleValue() >= 0)
				{
					sprintf(name, "framesReceived:%i", MACAddress);
					ethernetMACLogging->setVector(framesReceived, opp_strdup(name));
				}
				if (par("messagesPassedVector").doubleValue() >= 0)
				{
					sprintf(name, "messagesPassed:%i", MACAddress);
					ethernetMACLogging->setVector(messagesPassed, opp_strdup(name));
				}
				if (par("bitsReceivedVector").doubleValue() >= 0)
				{
					sprintf(name, "bitsReceived:%i", MACAddress);
					ethernetMACLogging->setVector(bitsReceived, opp_strdup(name));
				}
				if (!par("write1").boolValue())
				{
					if (par("messageDelayVector").doubleValue() >= 0)
					{
						int mac = ((EthernetFrame*) msg)->getSrcAddress();
						sprintf(name, "messageDelay:%i", mac);
						ethernetMACLogging->setVector(simTime() - msg->timestamp(), opp_strdup(name));
						if (par("dividedStatistics").boolValue())
						{
							sprintf(name, "messageDelay->%i:%i", MACAddress, mac);
							ethernetMACLogging->setVector(simTime() - msg->timestamp(), opp_strdup(name));
						}
						EthernetMAC* MAC = check_and_cast<EthernetMAC*>
							(simulation.module(((cMessage*) msg->getObject("MAC"))->kind()));
						MAC->collectMessageDelay(simTime() - msg->timestamp());
					}
					if (par("transmittingTimeVector").doubleValue() >= 0)
					{
						int mac = ((EthernetFrame*) msg)->getSrcAddress();
						sprintf(name, "transmittingTime:%i", mac);
						ethernetMACLogging->setVector(simTime() - ((EthernetFrame*) msg)->getTimeStamp(),
							opp_strdup(name));
						if (par("dividedStatistics").boolValue())
						{
							sprintf(name, "transmittingTime->%i:%i", MACAddress, mac);
							ethernetMACLogging->setVector(simTime() - ((EthernetFrame*) msg)->getTimeStamp(),
								opp_strdup(name));
						}
						EthernetMAC* MAC = check_and_cast<EthernetMAC*>
							(simulation.module(((cMessage*) msg->getObject("MAC"))->kind()));
						MAC->collectTransmittingTime(simTime() - ((EthernetFrame*) msg)->getTimeStamp());
					}
				}
				delete name;
				
				cMessage* data = (cMessage*) msg->decapsulate();
				send(data, "UpperLayerOut");
				ev << MODULENAME << ": Message \"" << data->name() << "\" send at t = "
					<< printTime(simTime()) << "\n";
			}
			delete msg;
		}
		else
		{
			ev << MODULENAME << ": Frame \"" << msg->name() << "\" is collided!\n";
			if (ev.isGUI()) bubble("Collision!");

			// logging
			collisions++;
			simtime_t timeStamp = msg->par("EthernetFrame").doubleValue();

			char* name = new char[256];
			if (par("collisionsVector").doubleValue() >= 0)
			{
				sprintf(name, "collisions:%i", MACAddress);
				ethernetMACLogging->setVector(collisions, opp_strdup(name));
			}
			if (par("retransmittingTimeVector").doubleValue() >= 0)
			{
				retransmittingTimeStatistics->collect(simTime() - timeStamp);
			}
			delete name;

			delete msg;
			
			// collision handling
			if (transmitState != TRANSMITTING_STATE)
				;
			else
			{
				handleCollision();
			}
		}
	}
	else
	{
		if (!msg->hasObject("collision"))
		{
			// not for us: discard
			framesReceived++;
			messagesDroppedNFU++;
			bitsReceived += msg->length();

			char* name = new char[256];
			if (par("framesReceivedVector").doubleValue() >= 0)
			{
				sprintf(name, "framesReceived:%i", MACAddress);
				ethernetMACLogging->setVector(framesReceived, opp_strdup(name));
			}
			if (par("messagesDroppedNFUVector").doubleValue() >= 0)
			{
				sprintf(name, "messagesDroppedNFU:%i", MACAddress);
				ethernetMACLogging->setVector(messagesDroppedNFU, opp_strdup(name));
			}
			if (par("bitsReceivedVector").doubleValue() >= 0)
			{
				sprintf(name, "bitsReceived:%i", MACAddress);
				ethernetMACLogging->setVector(bitsReceived, opp_strdup(name));
			}
			delete name;
			ev << MODULENAME << ": Frame \"" << msg->name() << "\" is not for us!\n";
			delete msg;
		}
	}

	// in future forUs has to be true
	forUs = true;
}

cMessage* EthernetMAC::encapsulateMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": encapsulateMessage entered\n";

	EthernetFrame* frame = new EthernetFrame("EthernetFrame", NORMAL);
	
	frame->encapsulate(msg);
	frame->setLength(PREAMBLE + SOF + DST_ADR + SRC_ADR + TYPE + msg->length()
		+ ((msg->length() / PAD_MAX >= 1) ? 0 : PAD_MAX - msg->length()) + CRC);

	if (debug) frameDetails(frame);
	
	// TODO: set the frame fields with frame->
//	setPreamble(int preamble_var);
//	setSOF(int SOF_var);
	frame->setSrcAddress(MACAddress);
	frame->setDstAddress(MACAddress); //TODO!
//	setType(int type_var);
//	setPad(int pad_var);
//	setCRC(int CRC_var);

	return (cMessage*) frame;
}

void EthernetMAC::frameDetails(cMessage* msg)
{
	ev << MODULENAME << ": Frame name is " << msg->className() << endl;
	ev << MODULENAME << ": Frame length is (in bit):\n";
	ev << MODULENAME << ": PREAMBLE = " << PREAMBLE << endl;
	ev << MODULENAME << ": SOF = " << SOF << endl;
	ev << MODULENAME << ": DST_ADR = " << DST_ADR << endl;
	ev << MODULENAME << ": SRC_ADR = " << SRC_ADR << endl;
	ev << MODULENAME << ": TYPE = " << TYPE << endl;
	ev << MODULENAME << ": DATA = " << msg->encapsulatedMsg()->length() << endl;
	ev << MODULENAME << ": PAD = "
		<< ((msg->encapsulatedMsg()->length() / PAD_MAX >= 1)
			? 0 : PAD_MAX - msg->encapsulatedMsg()->length()) << endl;
	ev << MODULENAME << ": CRC = " << CRC << endl;
	ev << MODULENAME << ": ----------------\n";
	int sum = PREAMBLE + SOF + DST_ADR + SRC_ADR + TYPE
		+ msg->encapsulatedMsg()->length() + ((msg->encapsulatedMsg()->length()
			/ PAD_MAX >= 1) ? 0 : PAD_MAX - msg->encapsulatedMsg()->length()) + CRC;
	ev << MODULENAME << ": Sum = " << sum << " ; ok? "
		<< (sum == msg->length() ? "yes" : "no") << endl;
}

void EthernetMAC::scheduleEndIFG()
{
	if (debug) ev << MODULENAME << ": scheduleEndIFG entered\n";

	scheduleAt(simTime() + IFG, endIFGMsg);
	ev << MODULENAME << ": Event \"" << endIFGMsg->name() << "\" scheduled at t = "
		<< printTime(simTime() + IFG) << endl;
    transmitState = WAIT_IFG_STATE;
}

void EthernetMAC::handleEndIFG()
{
	if (debug) ev << MODULENAME << ": handleEndIFG entered\n";

	if (transmitState != WAIT_IFG_STATE) error("Not in WAIT_IFG_STATE at the end of IFG period");

	if (queue.empty()) error("End of IFG and no frame to transmit");

	// end of IFG period, okay to transmit, if bus idle
	cMessage* checkMsg = new cMessage("Is Bus Free?", BUSREQUEST);
	waitingTime += IFG;

	ev << MODULENAME << ": IFG elapsed, now check the bus\n";
	send(checkMsg, "LowerLayerOut");
	ev << MODULENAME << ": Message \"" << checkMsg->name() << "\" send\n";
}

void EthernetMAC::handleBusRequest(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleBusRequest entered\n";

	// if free send
	if (msg->isName("Yes!"))
	{
		// transmit the frame now
		transmitFrame();
		delete msg;
	}
	else
	{
		// the frame has to be delayed
		scheduleRetransmission(msg);
	}
}

void EthernetMAC::transmitFrame()
{
	if (debug) ev << MODULENAME << ": transmitFrame entered\n";

	EthernetFrame* origFrame = (EthernetFrame*) queue.tail();
	EthernetFrame* frame = (EthernetFrame*) origFrame->dup();
	frame->addObject(new cMessage("MAC", id()));

	// for logging
	if (origFrame->getTimeStamp() < 0)
	{
		origFrame->setTimeStamp(simTime());
	}
	frame->setTimeStamp(simTime());
	
	send(frame, "LowerLayerOut");
	ev << MODULENAME << ": Message \"" << frame->name() << "\" send at t = "
		<< printTime(simTime()) << "\n";

	// schedule the end of transmission
	scheduleEndTx(origFrame);
}

void EthernetMAC::scheduleRetransmission(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": scheduleRetransmission entered\n";

	// logging:
//	EthernetFrame* actualFrame = (EthernetFrame *) queue.tail();

	// set the timestamp parameter as a logging helper
//	if (actualFrame->getTimeStamp() < 0)
//	{
//		actualFrame->setTimeStamp(simTime());
//	}

	ev << MODULENAME << ": Bus is busy and free at: "
		<< printTime(msg->par("Free At").doubleValue()) << endl;
	if (ev.isGUI()) bubble("Bus is busy!");
	transmitState = RETRANSMISSION_STATE;
	
	// schedule retransmission
	scheduleAt(msg->par("Free At").doubleValue(), retransMsg);
	ev << MODULENAME << ": Event \"" << retransMsg->name() << "\" scheduled at t = "
		<< printTime(msg->par("Free At").doubleValue()) << endl;
	delete msg;
}

void EthernetMAC::scheduleEndTx(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": scheduleEndTx entered\n";

	// this is the message delay for the actual bitrate and message length
	simtime_t messageDelay = (simtime_t) 1 / TXRATE * msg->length();
	
	scheduleAt(simTime() + messageDelay, endTxMsg);
	ev << MODULENAME << ": Event \"" << endTxMsg->name() << "\" scheduled at t = "
		<< printTime(simTime() + messageDelay) << endl;
	transmitState = TRANSMITTING_STATE;
}

void EthernetMAC::handleEndTx()
{
	if (debug) ev << MODULENAME << ": handleEndTx entered\n";
	
	EthernetFrame* origFrame = (EthernetFrame*) queue.tail();
	EthernetFrame* frame = (EthernetFrame*) origFrame->dup();

	// logging:
	framesSent++;
	bitsSent += origFrame->length();
	if (retransmittedReady)
	{
		framesRetransmitted++;
		retransmittedReady = false;
	}
	// time calculation
	simtime_t messageDelay = (simtime_t) 1 / TXRATE * origFrame->length();
	if (par("write1").boolValue())
	{
		if (origFrame->getTimeStamp() == -1.0)
			blockingTime = 0.0;
		else
		{
			if (fabs(simTime() - messageDelay - waitingTime + IFG - origFrame->getTimeStamp())
				< par("accuracyFP").doubleValue())
				blockingTime = 0.0;
			else
				blockingTime = simTime() - messageDelay - waitingTime + IFG - origFrame->getTimeStamp();
		}
		if (fabs(origFrame->getTimeStamp() - IFG - ((cMessage*) queue.tail())->timestamp())
			< par("accuracyFP").doubleValue())
			queueWaitingTime = 0.0;
		else
			queueWaitingTime = origFrame->getTimeStamp() - IFG - ((cMessage*) queue.tail())->timestamp();
		if (fabs(queueWaitingTime + blockingTime) < par("accuracyFP").doubleValue())
			waitingTime = 0.0;
		else
			waitingTime = queueWaitingTime + blockingTime;
	}
	else
	{
		if (origFrame->getTimeStamp() == -1.0)
			blockingTime = 0.0;
		else
		{
			if (fabs(simTime() - messageDelay - waitingTime + IFG - origFrame->getTimeStamp())
				< par("accuracyFP").doubleValue())
				blockingTime = 0.0;
			else
				blockingTime = simTime() - messageDelay - waitingTime + IFG - origFrame->getTimeStamp();
		}
		if (fabs(origFrame->getTimeStamp() - IFG - ((cMessage*) queue.tail())->timestamp())
			< par("accuracyFP").doubleValue())
			queueWaitingTime = 0.0;
		else
			queueWaitingTime = origFrame->getTimeStamp() - IFG - ((cMessage*) queue.tail())->timestamp();
		if (fabs(queueWaitingTime + blockingTime) < par("accuracyFP").doubleValue())
			waitingTime = 0.0;
		else
			waitingTime = queueWaitingTime + blockingTime;
	}
	// for message travelling
	frame->setTimeStamp(simTime());
	origFrame->setTimeStamp(-1.0);

	char* name = new char[256];
	if (par("framesSentVector").doubleValue() >= 0)
	{
		sprintf(name, "framesSent:%i", MACAddress);
		ethernetMACLogging->setVector(framesSent, opp_strdup(name));
	}
	if (par("bitsSentVector").doubleValue() >= 0)
	{
		sprintf(name, "bitsSent:%i", MACAddress);
		ethernetMACLogging->setVector(bitsSent, opp_strdup(name));
	}
	if (par("waitingTimeVector").doubleValue() >= 0)
	{
		sprintf(name, "waitingTime:%i", MACAddress);
		ethernetMACLogging->setVector(waitingTime, opp_strdup(name));
		waitingTimeStatistics->collect(waitingTime);
	}
	waitingTime = 0.0;
	if (par("blockingTimeVector").doubleValue() >= 0)
	{
		sprintf(name, "blockingTime:%i", MACAddress);
		ethernetMACLogging->setVector(blockingTime, opp_strdup(name));
		blockingTimeStatistics->collect(blockingTime);
	}
	blockingTime = 0.0;
	if (par("queueWaitingTimeVector").doubleValue() >= 0)
	{
		sprintf(name, "queueWaitingTime:%i", MACAddress);
		ethernetMACLogging->setVector(queueWaitingTime, opp_strdup(name));
		queueWaitingTimeStatistics->collect(queueWaitingTime);
	}
	queueWaitingTime = 0.0;
	if (par("write1").boolValue())
	{
		simtime_t transmit_t = MAX_LENGTH / PSPEED + (double) frame->length() / TXRATE;
		if (par("messageDelayVector").doubleValue() >= 0)
		{
			sprintf(name, "messageDelay:%i", MACAddress);
			ethernetMACLogging->setVector(simTime() - frame->timestamp() + transmit_t,
				opp_strdup(name));
			messageDelayStatistics->collect(simTime() - frame->timestamp() + transmit_t);
		}
		if (par("transmittingTimeVector").doubleValue() >= 0)
		{
			sprintf(name, "transmittingTime:%i", MACAddress);
			ethernetMACLogging->setVector(transmit_t, opp_strdup(name));
			transmittingTimeStatistics->collect(transmit_t);
		}
	}
	delete name;

	// transmission done: remove frame from queue
	transmitState = TX_IDLE_STATE;
	backoffs = 0;
	delete queue.pop();

	// if the queue is not empty we are ready to send the next object (after the IFG)
	if (!queue.empty()) scheduleEndIFG();
}

void EthernetMAC::handleRetransmission()
{
	if (debug) ev << MODULENAME << ": handleRetransmission entered\n";
	
	// IFG has to elapse
	scheduleEndIFG();
}

void EthernetMAC::handleCollision()
{
	if (debug) ev << MODULENAME << ": handleCollision entered\n";
	
	if (transmitState == WAIT_IFG_STATE) error("Collision in state WAIT_IFG_STATE occurred");

	// two cases: 
	// 1. the collision occurred while we are transmitting
	// 2. we scheduled the last frame because the bus was used
	switch (transmitState)
	{
		case TRANSMITTING_STATE:
			// discard frame
			if (!cd)
			{
				cancelEvent(endTxMsg);
				handleEndTx();
				break;
			}
			// cancel event endTxMsg and handle backoff
			cancelEvent(endTxMsg);
			scheduleJam_Backoff();
			break;
		case RETRANSMISSION_STATE:
			// discard frame
			if (!cd)
			{
				cancelEvent(retransMsg);
				handleEndTx();
				break;
			}
			// cancel event retransMsg and try to send again (after IFG)
			cancelEvent(retransMsg);
			scheduleEndIFG();
			break;
	}
}

void EthernetMAC::scheduleJam_Backoff()
{
	if (debug) ev << MODULENAME << ": scheduleJam_Backoff entered\n";

	scheduleAt(simTime() + JAM, endJammingMsg);
	ev << MODULENAME << ": Event \"" << endJammingMsg->name() << "\" scheduled at t = "
		<< printTime(simTime() + JAM) << endl;
    transmitState = JAMMING_STATE;
}

void EthernetMAC::handleBackoff()
{
	if (debug) ev << MODULENAME << ": handleBackoff entered\n";

	// TODO: if maximum retries reached we have to inform the upper layer
	if (++backoffs > MAX_ATTEMPTS)
	{
		ev << MODULENAME << ": Number of retransmit attempts of frame exceeds maximum, "
			<< "cancelling transmission of frame " << queue.tail()->name() << endl;
		delete queue.pop();

		transmitState = TX_IDLE_STATE;
		backoffs = 0;
		frameRetransmissionFailure++;
		// TODO
		// error("MAX_ATTEPTS. We have to stop here....");
		return;
	}

	// calculate (backoff) time for retransmission
	ev << MODULENAME << ": Executing backoff procedure\n";
	int backoffrange = (backoffs >= BACKOFF_RANGE) ? 1024 : (1 << backoffs);
    int slotNumber = intuniform(0, backoffrange - 1, 1);
	simtime_t backofftime = slotNumber * SLOT_TIME;

	// logging
	backoffTime = backofftime;

	char* name = new char[256];
	if (par("backoffTimeVector").doubleValue() >= 0)
	{
		sprintf(name, "backoffTime:%i", MACAddress);
		ethernetMACLogging->setVector(backoffTime, opp_strdup(name));
		backoffTimeStatistics->collect(backoffTime);
	}
	backoffTime = 0.0;
	delete name;

	scheduleAt(simTime() + backofftime, endBackoffMsg);
	ev << MODULENAME << ": Event \"" << endBackoffMsg->name() << "\" scheduled at t = "
		<< printTime(simTime() + backofftime) << endl;
	transmitState = BACKOFF_STATE;
}

void EthernetMAC::handleEndBackoff()
{
	if (debug) ev << MODULENAME << ": handleEndBackoff entered\n";

	// logging helper
	retransmittedReady = true;

	char* name = new char[256];
	if (par("framesRetransmittedVector").doubleValue() >= 0)
	{
		sprintf(name, "framesRetransmitted:%i", MACAddress);
		ethernetMACLogging->setVector(framesRetransmitted, opp_strdup(name));
	}
	delete name;

	// we start at the beginning
	scheduleEndIFG();
}

void EthernetMAC::finish()
{
	if (par("write1").boolValue())
	{
		if (par("backoffTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->write1Statistics(backoffTimeStatistics);
		if (par("waitingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->write1Statistics(waitingTimeStatistics);
		if (par("blockingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->write1Statistics(blockingTimeStatistics);
		if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->write1Statistics(queueWaitingTimeStatistics);
		if (par("transmittingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->write1Statistics(transmittingTimeStatistics);
		if (par("retransmittingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->write1Statistics(retransmittingTimeStatistics);
		if (par("messageDelayStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(messageDelayStatistics);
	}
	else
	{
		if (par("backoffTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(backoffTimeStatistics);
		if (par("waitingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(waitingTimeStatistics);
		if (par("blockingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(blockingTimeStatistics);
		if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(queueWaitingTimeStatistics);
		if (par("transmittingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(transmittingTimeStatistics);
		if (par("retransmittingTimeStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(retransmittingTimeStatistics);
		if (par("messageDelayStatistics").doubleValue() >= 0)
			ethernetMACLogging->writeStatistics(messageDelayStatistics);
	}
	ethernetMACLogging->writeMessageTimeDelay();
	if (par("writeScalars").boolValue())
	{
		char* name = new char[256];
		sprintf(name, "retransmissionFailures:%i", MACAddress);
		recordScalar(name, frameRetransmissionFailure);
		sprintf(name, "unsentMessages:%i", MACAddress);
		recordScalar(name, queue.length());
		delete name;
	}
}

void EthernetMAC::initLogging()
{
	char* name = new char[256];
	char* description = new char[256];

	if (par("framesSentVector").doubleValue() >= 0)
	{
		// frames sent vector
		sprintf(name, "framesSent:%i", MACAddress);
		sprintf(description, "'frames sent:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesSentVector").doubleValue());
		ethernetMACLogging->setVector(framesSent, opp_strdup(name));
		if (par("framesSentStatistics").doubleValue() >= 0)
		{
			// frames sent statistics
			sprintf(description, "'frames sent statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesSentStatistics").doubleValue());
		}
	}

	if (par("framesReceivedVector").doubleValue() >= 0)
	{
		// frames received vector
		sprintf(name, "framesReceived:%i", MACAddress);
		sprintf(description, "'frames received:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesReceivedVector").doubleValue());
		ethernetMACLogging->setVector(framesReceived, opp_strdup(name));
		if (par("framesReceivedStatistics").doubleValue() >= 0)
		{
			// frames received statistics
			sprintf(description, "'frames received statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesReceivedStatistics").doubleValue());
		}
	}

	if (par("bitsSentVector").doubleValue() >= 0)
	{
		// bits sent vector
		sprintf(name, "bitsSent:%i", MACAddress);
		sprintf(description, "'bits sent:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("bitsSentVector").doubleValue());
		ethernetMACLogging->setVector(bitsSent, opp_strdup(name));
		if (par("bitsSentStatistics").doubleValue() >= 0)
		{
			// bits sent statistics
			sprintf(description, "'bits sent statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("bitsSentStatistics").doubleValue());
		}
	}

	if (par("bitsReceivedVector").doubleValue() >= 0)
	{
		// bits received vector
		sprintf(name, "bitsReceived:%i", MACAddress);
		sprintf(description, "'bits received:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("bitsReceivedVector").doubleValue());
		ethernetMACLogging->setVector(bitsReceived, opp_strdup(name));
		if (par("bitsReceivedStatistics").doubleValue() >= 0)
		{
			// bits received statistics
			sprintf(description, "'bits received statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("bitsReceivedStatistics").doubleValue());
		}
	}

	if (par("messagesPassedVector").doubleValue() >= 0)
	{
		// messages passed vector
		sprintf(name, "messagesPassed:%i", MACAddress);
		sprintf(description, "'messages passed:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesPassedVector").doubleValue());
		ethernetMACLogging->setVector(messagesPassed, opp_strdup(name));
		if (par("messagesPassedStatistics").doubleValue() >= 0)
		{
			// messages passed statistics
			sprintf(description, "'messages passed statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesPassedStatistics").doubleValue());
		}
	}

	if (par("messagesReceivedVector").doubleValue() >= 0)
	{
		// messages received vector
		sprintf(name, "messagesReceived:%i", MACAddress);
		sprintf(description, "'messages received:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesReceivedVector").doubleValue());
		ethernetMACLogging->setVector(messagesReceived, opp_strdup(name));
		if (par("messagesReceivedStatistics").doubleValue() >= 0)
		{
			// messages received statistics
			sprintf(description, "'messages received statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesReceivedStatistics").doubleValue());
		}
	}

	if (par("messagesDroppedBOVector").doubleValue() >= 0)
	{
		// messages dropped - buffer overflow - vector
		sprintf(name, "messagesDroppedBO:%i", MACAddress);
		sprintf(description, "'messages dropped - buffer overflow:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesDroppedBOVector").doubleValue());
		ethernetMACLogging->setVector(messagesDroppedBO, opp_strdup(name));
		if (par("messagesDroppedBOStatistics").doubleValue() >= 0)
		{
			// messages dropped - buffer overflow - statistics
			sprintf(description, "'messages dropped - buffer overflow - statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesDroppedBOStatistics").doubleValue());
		}
	}

	if (par("messagesDroppedBEVector").doubleValue() >= 0)
	{
		// messages dropped - bit error - vector
		sprintf(name, "messagesDroppedBE:%i", MACAddress);
		sprintf(description, "'messages dropped - bit error:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesDroppedBEVector").doubleValue());
		ethernetMACLogging->setVector(messagesDroppedBE, opp_strdup(name));
		if (par("messagesDroppedBEStatistics").doubleValue() >= 0)
		{
			// messages dropped - bit error - statistics
			sprintf(description, "'messages dropped - bit error - statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesDroppedBEStatistics").doubleValue());
		}
	}
	
	if (par("messagesDroppedNFUVector").doubleValue() >= 0)
	{
		// messages dropped - not for us - vector
		sprintf(name, "messagesDroppedNFU:%i", MACAddress);
		sprintf(description, "'messages dropped - not for us:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesDroppedNFUVector").doubleValue());
		ethernetMACLogging->setVector(messagesDroppedNFU, opp_strdup(name));
		if (par("messagesDroppedNFUStatistics").doubleValue() >= 0)
		{
			// messages dropped - not for us - statistics
			sprintf(description, "'messages dropped - not for us - statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesDroppedNFUStatistics").doubleValue());
		}
	}

	if (par("collisionsVector").doubleValue() >= 0)
	{
		// collisions occured vector
		sprintf(name, "collisions:%i", MACAddress);
		sprintf(description, "'collisions occured:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("collisionsVector").doubleValue());
		ethernetMACLogging->setVector(collisions, opp_strdup(name));
		if (par("collisionsStatistics").doubleValue() >= 0)
		{
			// collisions occured statistics
			sprintf(description, "'collisions occured statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("collisionsStatistics").doubleValue());
		}
	}

	if (par("framesRetransmittedVector").doubleValue() >= 0)
	{
		// frames retransmitted vector
		sprintf(name, "framesRetransmitted:%i", MACAddress);
		sprintf(description, "'frames retransmitted:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesRetransmittedVector").doubleValue());
		ethernetMACLogging->setVector(framesRetransmitted, opp_strdup(name));
		if (par("framesRetransmittedStatistics").doubleValue() >= 0)
		{
			// frames retransmitted statistics
			sprintf(description, "'frames retransmitted statistics:%i'", MACAddress);
			ethernetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesRetransmittedStatistics").doubleValue());
		}
	}

	if (par("backoffTimeVector").doubleValue() >= 0)
	{
		// back off time vector
		sprintf(name, "backoffTime:%i", MACAddress);
		sprintf(description, "'back off time:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("backoffTimeVector").doubleValue());
		ethernetMACLogging->setVector(backoffTime, opp_strdup(name));
		if (par("backoffTimeStatistics").doubleValue() >= 0)
		{
			// back off time statistics
			sprintf(description, "'back off time statistics:%i'", MACAddress);
			backoffTimeStatistics = new cStdDev(opp_strdup(description));
//			backoffTimeStatistics->collect(backoffTime);
		}
	}

	if (par("waitingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "waitingTime:%i", MACAddress);
		sprintf(description, "'waiting time:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("waitingTimeVector").doubleValue());
		ethernetMACLogging->setVector(waitingTime, opp_strdup(name));
		if (par("waitingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'waiting time statistics:%i'", MACAddress);
			waitingTimeStatistics = new cStdDev(opp_strdup(description));
//			waitingTimeStatistics->collect(waitingTime);
		}
	}

	if (par("blockingTimeVector").doubleValue() >= 0)
	{
		// blocking time
		sprintf(name, "blockingTime:%i", MACAddress);
		sprintf(description, "'blocking  time:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("blockingTimeVector").doubleValue());
		ethernetMACLogging->setVector(blockingTime, opp_strdup(name));
		if (par("blockingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'blocking time statistics:%i'", MACAddress);
			blockingTimeStatistics = new cStdDev(opp_strdup(description));
//			blockingTimeStatistics->collect(blockingTime);
		}
	}

	if (par("queueWaitingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "queueWaitingTime:%i", MACAddress);
		sprintf(description, "'queue waiting time:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("queueWaitingTimeVector").doubleValue());
		ethernetMACLogging->setVector(queueWaitingTime, opp_strdup(name));
		if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'queue waiting time statistics:%i'", MACAddress);
			queueWaitingTimeStatistics = new cStdDev(opp_strdup(description));
//			queueWaitingTimeStatistics->collect(queueWaitingTime);
		}
	}

	simtime_t transmittingTime = 0.0;
	if (par("transmittingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "transmittingTime:%i", MACAddress);
		sprintf(description, "'transmitting time:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("transmittingTimeVector").doubleValue());
		ethernetMACLogging->setVector(transmittingTime, opp_strdup(name));
		if (par("transmittingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'transmitting time statistics:%i'", MACAddress);
			transmittingTimeStatistics = new cStdDev(opp_strdup(description));
//			transmittingTimeStatistics->collect(transmittingTime);
		}
		if (par("dividedStatistics").boolValue())
		{
			// message delay statistics for every MAC
			for (int i = 1; i <= par("hosts").longValue(); i++)
			{
				if (i != MACAddress)
				{
					sprintf(name, "transmittingTime->%i:%i", MACAddress, i);
					sprintf(description, "'transmitting time->%i:%i'", MACAddress, i);
					ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description),
						0.0, par("transmittingTimeVector").doubleValue());
//					ethernetMACLogging->setVector(transmittingTime, opp_strdup(name));
				}
			}
		}
	}

	simtime_t retransmittingTime = 0.0;
	if (par("retransmittingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "retransmittingTime:%i", MACAddress);
		sprintf(description, "'retransmitting time:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("retransmittingTimeVector").doubleValue());
		ethernetMACLogging->setVector(retransmittingTime, opp_strdup(name));
		if (par("retransmittingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'retransmitting time statistics:%i'", MACAddress);
			retransmittingTimeStatistics = new cStdDev(opp_strdup(description));
			retransmittingTimeStatistics->collect(retransmittingTime);
		}
	}

	simtime_t messageDelay = 0.0;
	if (par("messageDelayVector").doubleValue() >= 0)
	{
		// message delay vector
		sprintf(name, "messageDelay:%i", MACAddress);
		sprintf(description, "'message delay:%i'", MACAddress);
		ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("messageDelayVector").doubleValue());
		ethernetMACLogging->setVector(messageDelay, opp_strdup(name));
		if (par("messageDelayStatistics").doubleValue() >= 0)
		{
			// message delay statistics
			sprintf(description, "'message delay statistics:%i'", MACAddress);
			messageDelayStatistics = new cStdDev(opp_strdup(description));
			messageDelayStatistics->collect(messageDelay);
		}
		if (par("dividedStatistics").boolValue())
		{
			// message delay statistics for every MAC
			for (int i = 1; i <= par("hosts").longValue(); i++)
			{
				if (i != MACAddress)
				{
					sprintf(name, "messageDelay->%i:%i", MACAddress, i);
					sprintf(description, "'message delay->%i:%i'", MACAddress, i);
					ethernetMACLogging->initVector(opp_strdup(name), opp_strdup(description),
						0.0, par("messageDelayVector").doubleValue());
					ethernetMACLogging->setVector(messageDelay, opp_strdup(name));
				}
			}
		}
	}

	delete name;
	delete description;
}

void EthernetMAC::collectMessageDelay(simtime_t time)
{
	if (debug) ev << MODULENAME << ": collectMessageDelay entered\n";

	Enter_Method("collectMessageDelay(simtime_t time)");

	messageDelayStatistics->collect(time);
}

void EthernetMAC::collectTransmittingTime(simtime_t time)
{
	if (debug) ev << MODULENAME << ": collectTransmittingTime entered\n";

	Enter_Method("collectTransmittingTime(simtime_t time)");

	transmittingTimeStatistics->collect(time);
}
