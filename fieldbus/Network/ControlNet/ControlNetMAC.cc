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

#include "ControlNetMAC.h"

/**
 * This is only the implementation the ControlNetMAC class.
 */

void ControlNetMAC::initialize()
{
	// DEBUG?
	if (par("debug").boolValue())
		debug = true;
	else
		debug = false;

	maxScheduledQueueSize = (int) par("maxScheduledQueueSize").longValue();
	maxUnscheduledQueueSize = (int) par("maxUnscheduledQueueSize").longValue();

	scheduledQueue.setName("SQueue");
	unscheduledQueue.setName("UQueue");
	
	endIFGMsg = new cMessage("EndIFG", ENDIFG);
	endIFGMsg->setPriority(10);
	endGBMsg = new cMessage("EndGB", ENDGB);
	endGBMsg->setPriority(10);

	preUToken = 0;
	lastFrameLength = 0;
	scheduleAt(simTime(), endGBMsg);
	
	// measurement
	framesSent = 0L;
	framesReceived = 0L;
	bitsSent = 0L;
	bitsReceived = 0L;

	messagesPassed = 0L;
	messagesReceived = 0L;

	messagesDroppedBO = 0L;
	messagesDroppedBE = 0L;

	waitingTime = 0.0;
	blockingTime = 0.0;
	queueWaitingTime = 0.0;

	// measurement initialization
	controlnetMACLoggingModule = parentModule()->submodule("controlnetMACLogging");
	controlnetMACLogging = check_and_cast<ControlNetMACLogging*>(controlnetMACLoggingModule);
}

void ControlNetMAC::handleMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleMessage entered\n";

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
			case ENDGB:
				handleEndGB();
				break;
		}
		ev << "\n";
		return;
	}

	// register the own address and init the logging
	if (msg->isName("MACID"))
	{
		macID = msg->kind();
		ev << MODULENAME << ": macID " << macID << " registred.\n\n";
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
			sprintf(name, "messagesReceived:%i", macID);
			controlnetMACLogging->setVector(messagesReceived, opp_strdup(name));
		}
		delete name;

		handleUpperLayerMessage(msg);
	}
	
	// handle messages comming from bus
	if (msg->arrivalGate()->isName("LowerLayerIn"))
	{
		handleLowerLayerMessage(msg);
	}
	ev << "\n";
}

void ControlNetMAC::handleUpperLayerMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleUpperLayerMessage entered\n";

	ControlNetFrame* frame = (ControlNetFrame*) encapsulateData(msg);

	// scheduled or unscheduled messages? put it into queues only
	if (msg->kind() == SCHEDULED)
	{
		ev << MODULENAME << ": Packet \"" << msg->name() << "\" arrived from higher layers, enqueueing\n";
		if (scheduledQueue.length() < maxScheduledQueueSize)
		{
			frame->setTimestamp(simTime());
			((ControlNetFrame*) frame)->setTimeStamp(-1.0);
			scheduledQueue.insert(frame);
			updateDisplayString();
			return;
		}
		else
		{
			goto DISCARD;
		}
	}
	else
	{
		ev << MODULENAME << ": Packet \"" << msg->name() << "\" arrived from higher layers, enqueueing\n";
		if (unscheduledQueue.length() < maxUnscheduledQueueSize)
		{
			frame->setTimestamp(simTime());
			((ControlNetFrame*) frame)->setTimeStamp(-1.0);
			unscheduledQueue.insert(frame);
			updateDisplayString();
			return;
		}
		else
		{
			goto DISCARD;
		}
	}

	DISCARD:
		// logging
		messagesDroppedBO++;
	
		char* name = new char[256];
		if (par("messagesDroppedBOVector").doubleValue() >= 0)
		{
			sprintf(name, "messagesDroppedBO:%i", macID);
			controlnetMACLogging->setVector(messagesDroppedBO, opp_strdup(name));
		}
		delete name;
	
		ev << MODULENAME << ": Packet \"" << msg->name() << "\" discarded because the queue is full!\n";
		if (ev.isGUI()) bubble("Packet discarded!");
		delete(frame);
		return;
}

void ControlNetMAC::handleLowerLayerMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleLowerLayerMessage entered\n";

	if (state == GUARDBAND_STATE) error("Frame received in guardband state");

	// increment counters and check for "my turn to send"
	if (state == SCHEDULED_STATE)
	{
		handleScheduled(msg);
	}
	else
	{
		handleUnscheduled(msg);
	}

	// error handling
	if (msg->hasBitError())
	{
		// logging
		messagesDroppedBE++;

		char* name = new char[256];
		if (par("messagesDroppedBEVector").doubleValue() >= 0)
		{
			sprintf(name, "messagesDroppedBE:%i", macID);
			controlnetMACLogging->setVector(messagesDroppedBE, opp_strdup(name));
		}
		delete name;

		if (ev.isGUI()) bubble("Frame has bit error!");
		delete msg;
		return;
	}

	// maybe it's a NULL frame 
	if (msg->isName("NULL"))
	{
		delete msg;
	}
	else
	{
		// send message to the upper layer
		cMessage* data = (cMessage*) msg->decapsulate()->dup();
	
		// logging
		framesReceived++;
		messagesPassed++;
		bitsReceived += msg->length();

		char* name = new char[256];
		if (par("framesReceivedVector").doubleValue() >= 0)
		{
			sprintf(name, "framesReceived:%i", macID);
			controlnetMACLogging->setVector(framesReceived, opp_strdup(name));
		}
		if (par("messagesPassedVector").doubleValue() >= 0)
		{
			sprintf(name, "messagesPassed:%i", macID);
			controlnetMACLogging->setVector(messagesPassed, opp_strdup(name));
		}
		if (par("bitsReceivedVector").doubleValue() >= 0)
		{
			sprintf(name, "bitsReceived:%i", macID);
			controlnetMACLogging->setVector(bitsReceived, opp_strdup(name));
		}
		if (!par("write1").boolValue())
		{
			if (par("messageDelayVector").doubleValue() >= 0)
			{
				int mac = ((ControlNetFrame*) msg)->getSrcAddress();
				sprintf(name, "messageDelay:%i", mac);
				controlnetMACLogging->setVector(simTime() - msg->timestamp(), opp_strdup(name));
				if (par("dividedStatistics").boolValue())
				{
					sprintf(name, "messageDelay->%i:%i", macID, mac);
					controlnetMACLogging->setVector(simTime() - msg->timestamp(), opp_strdup(name));
				}
				ControlNetMAC* MAC = check_and_cast<ControlNetMAC*>
					(simulation.module(((cMessage*) msg->getObject("MAC"))->kind()));
				MAC->collectMessageDelay(simTime() - msg->timestamp());
			}
			if (par("transmittingTimeVector").doubleValue() >= 0)
			{
				int mac = ((ControlNetFrame*) msg)->getSrcAddress();
				sprintf(name, "transmittingTime:%i", mac);
				controlnetMACLogging->setVector(simTime() - ((ControlNetFrame*) msg)->getTimeStamp(),
					opp_strdup(name));
				if (par("dividedStatistics").boolValue())
				{
					sprintf(name, "transmittingTime->%i:%i", macID, mac);
					controlnetMACLogging->setVector(simTime() - ((ControlNetFrame*) msg)->getTimeStamp(),
						opp_strdup(name));
				}
				ControlNetMAC* MAC = check_and_cast<ControlNetMAC*>
					(simulation.module(((cMessage*) msg->getObject("MAC"))->kind()));
				MAC->collectTransmittingTime(simTime() - ((ControlNetFrame*) msg)->getTimeStamp());
			}
		}

		send(data, "UpperLayerOut");
		ev << MODULENAME << ": Message \"" << data->name() << "\" send at t = "
			<< printTime(simTime()) << "\n";
		delete msg;
	}
}

cMessage* ControlNetMAC::encapsulateData(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": encapsulateData entered\n";

	ControlNetFrame* frame = new ControlNetFrame("ControlNetFrame", msg->kind());
	
	frame->encapsulate(msg);
	frame->setLength(PREAMBLE + SD + SRC_ADR + F_LPACKET_HEADER + msg->length()
		+ CRC + ED);

	if (debug) frameDetails(frame);
	
	// TODO: set the frame fields with frame->
//	setPreamble(int preamble_var);
//	setSD(int SD_var);
	frame->setSrcAddress(macID);
	frame->setLPackets(1);
//	setCRC(int CRC_var);
//	setED(int ED_var);

	return (cMessage*) frame;
}

void ControlNetMAC::frameDetails(cMessage* msg)
{
	ev << MODULENAME << ": Frame name is " << msg->className() << endl;
	ev << MODULENAME << ": Frame length is (in bit):\n";
	ev << MODULENAME << ": PREAMBLE = " << PREAMBLE << endl;
	ev << MODULENAME << ": SD = " << SD << endl;
	ev << MODULENAME << ": SRD_ADR = " << SRC_ADR << endl;
	ev << MODULENAME << ": F_LPACKET_HEADER = " << F_LPACKET_HEADER << endl;
	ev << MODULENAME << ": DATA = " << msg->encapsulatedMsg()->length() << endl;
	ev << MODULENAME << ": CRC = " << CRC << endl;
	ev << MODULENAME << ": ED = " << ED << endl;
	ev << MODULENAME << ": ----------------\n";
	int sum = PREAMBLE + SD + SRC_ADR + F_LPACKET_HEADER +
		msg->encapsulatedMsg()->length() + CRC + ED;
	ev << MODULENAME << ": Sum = " << sum << " ; ok? "
		<< (sum == msg->length() ? "yes" : "no") << endl;
}

void ControlNetMAC::updateDisplayString()
{
	if (debug) ev << MODULENAME << ": updateDisplayString entered\n";

	char* dspStr = new char[32];
	
	sprintf(dspStr, "q:(%i|%i)", scheduledQueue.length(), unscheduledQueue.length());
	if (ev.isGUI()) displayString().setTagArg("t", 0, opp_strdup(dspStr));
	delete dspStr;
}

void ControlNetMAC::handleScheduled(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleScheduled entered\n";

	// init message?
	if (msg->isName("ScheduleInit"))
	{
		scheduledToken = 1;
		goto TURN;
	}

	// end of scheduled period?
	if (((ControlNetFrame*) msg)->getSrcAddress() == SMAX)
	{
		handleEndScheduled();
		return;
	}

	// set the scheduled token
	scheduledToken = (((ControlNetFrame*) msg)->getSrcAddress() + 1 > SMAX ?
		1 : ((ControlNetFrame*) msg)->getSrcAddress() + 1);

	TURN:
		// if my turn schedule end of IFG
		if (scheduledToken == macID)
		{
			// time checking
			if (unscheduledQueue.empty())
			{
				if (simTime() + IFG + MAX_LENGTH / PSPEED + (double) FRAME_MIN / TXRATE <= uLimit)
				{
					scheduleAt(simTime() + IFG, endIFGMsg);
					ev << MODULENAME << ": Event \"" << endIFGMsg->name() << "\" scheduled at t = "
						<< printTime(simTime() + IFG) << endl;
				}
				else
					error("Scheduled message doesn't fit into NUT");
			}
			else
			{
				if (simTime() + IFG + MAX_LENGTH / PSPEED + 
					(double) ((ControlNetFrame*) unscheduledQueue.tail())->length() / TXRATE <= uLimit)
				{
					scheduleAt(simTime() + IFG, endIFGMsg);
					ev << MODULENAME << ": Event \"" << endIFGMsg->name() << "\" scheduled at t = "
						<< printTime(simTime() + IFG) << endl;
				}
				else
					error("Scheduled message doesn't fit into NUT");
			}
		}
}

void ControlNetMAC::handleUnscheduled(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleUnscheduled entered\n";

	simtime_t diff = 0.0;
	// init message?
	if (msg->isName("UnscheduleInit"))
	{
		unscheduledToken = msg->kind();
		if (macID == UMAX) diff = (double) lastFrameLength / TXRATE;
		goto TURN;
	}

	// set the unscheduled token
	unscheduledToken = (((ControlNetFrame*) msg)->getSrcAddress() + 1 > UMAX ?
		1 : ((ControlNetFrame*) msg)->getSrcAddress() + 1);

	TURN:
		// after "time check" if my turn schedule end of IFG
		if (unscheduledToken == macID)
		{
			// time checking
			if (unscheduledQueue.empty())
			{
				if (simTime() + diff + MAX_LENGTH / PSPEED + (double) FRAME_MIN / TXRATE
					< uLimit)
				{
					scheduleAt(simTime() + diff + IFG, endIFGMsg);
					ev << MODULENAME << ": Event \"" << endIFGMsg->name() << "\" scheduled at t = "
						<< printTime(simTime() + diff + IFG) << endl;
				}
				else
				{
					handleEndUnscheduled();
					ev << MODULENAME << ": End of unscheduled period\n";
				}
			}
			else
			{
				if (simTime() + diff + MAX_LENGTH / PSPEED + 
					(double) ((ControlNetFrame*) unscheduledQueue.tail())->length() / TXRATE < uLimit)
				{
					scheduleAt(simTime() + diff + IFG, endIFGMsg);
					ev << MODULENAME << ": Event \"" << endIFGMsg->name() << "\" scheduled at t = "
						<< printTime(simTime() + diff + IFG) << endl;
				}
				else
				{
					handleEndUnscheduled();
					ev << MODULENAME << ": End of unscheduled period\n";
				}
			}
		}
}

void ControlNetMAC::handleEndIFG()
{
	if (debug) ev << MODULENAME << ": handleEndIFG entered\n";

	if (state == GUARDBAND_STATE) error("Frame received in guardband state");

	// if no frame in queue, send NULL frame, else send frame from queue
	if (state == SCHEDULED_STATE)
	{
		if (scheduledQueue.empty())
		{
			ControlNetFrame* frame = new ControlNetFrame("NULL", NUL);
			frame->setSrcAddress(macID);
			frame->setLength(FRAME_MIN);
			lastFrameLength = FRAME_MIN;
			send(frame, "LowerLayerOut");
				ev << MODULENAME << ": Message \"" << frame->name() << "\" send at t = "
					<< printTime(simTime()) << "\n";
		}
		else
		{
			if (simTime() + MAX_LENGTH / PSPEED + (double) ((ControlNetFrame*)
				scheduledQueue.tail())->length() / TXRATE <= uLimit)
			{
				ControlNetFrame* frame = (ControlNetFrame*) scheduledQueue.tail()->dup();
	
				// logging
				framesSent++;
				bitsSent += frame->length();
				// time calculation
				if (par("write1").boolValue())
				{
					if (frame->getTimeStamp() == -1.0)
						blockingTime = 0.0;
					else
					{
						if (fabs(simTime() - frame->getTimeStamp()) < par("accuracyFP").doubleValue())
							blockingTime = 0.0;
						else
							blockingTime = simTime() - frame->getTimeStamp();
					}
					if (fabs(simTime() - frame->timestamp()) < par("accuracyFP").doubleValue())
						queueWaitingTime = 0.0;
					else
						queueWaitingTime = simTime() - frame->timestamp();
					if (fabs(queueWaitingTime + blockingTime) < par("accuracyFP").doubleValue())
						waitingTime = 0.0;
					else
						waitingTime = queueWaitingTime + blockingTime;
				}
				else
				{
					if (frame->getTimeStamp() == -1.0)
						blockingTime = 0.0;
					else
					{
						if (fabs(simTime() - IFG - frame->getTimeStamp()) < par("accuracyFP").doubleValue())
							blockingTime = 0.0;
						else
							blockingTime = simTime() - IFG - frame->getTimeStamp();
					}
					if (fabs(simTime() - IFG - frame->timestamp()) < par("accuracyFP").doubleValue())
						queueWaitingTime = 0.0;
					else
						queueWaitingTime = simTime() - IFG - frame->timestamp();
					if (fabs(queueWaitingTime + blockingTime) < par("accuracyFP").doubleValue())
						waitingTime = 0.0;
					else
						waitingTime = queueWaitingTime + blockingTime;
				}
				// for message travelling
				frame->setTimeStamp(simTime());
	
				char* name = new char[256];
				if (par("framesSentVector").doubleValue() >= 0)
				{
					sprintf(name, "framesSent:%i", macID);
					controlnetMACLogging->setVector(framesSent, opp_strdup(name));
				}
				if (par("bitsSentVector").doubleValue() >= 0)
				{
					sprintf(name, "bitsSent:%i", macID);
					controlnetMACLogging->setVector(bitsSent, opp_strdup(name));
				}
				if (par("waitingTimeVector").doubleValue() >= 0)
				{
					sprintf(name, "waitingTime:%i", macID);
					controlnetMACLogging->setVector(waitingTime, opp_strdup(name));
					waitingTimeStatistics->collect(waitingTime);
				}
				waitingTime = 0.0;
				if (par("blockingTimeVector").doubleValue() >= 0)
				{
					sprintf(name, "blockingTime:%i", macID);
					controlnetMACLogging->setVector(blockingTime, opp_strdup(name));
					blockingTimeStatistics->collect(blockingTime);
				}
				blockingTime = 0.0;
				if (par("queueWaitingTimeVector").doubleValue() >= 0)
				{
					sprintf(name, "queueWaitingTime:%i", macID);
					controlnetMACLogging->setVector(queueWaitingTime, opp_strdup(name));
					queueWaitingTimeStatistics->collect(queueWaitingTime);
				}
				queueWaitingTime = 0.0;
				if (par("write1").boolValue())
				{
					simtime_t transmit_t = MAX_LENGTH / PSPEED + (double) frame->length() / TXRATE;
					if (par("messageDelayVector").doubleValue() >= 0)
					{
						sprintf(name, "messageDelay:%i", macID);
						controlnetMACLogging->setVector(simTime() - frame->timestamp() + transmit_t,
							opp_strdup(name));
						messageDelayStatistics->collect(simTime() - frame->timestamp() + transmit_t);
					}
					if (par("transmittingTimeVector").doubleValue() >= 0)
					{
						sprintf(name, "transmittingTime:%i", macID);
						controlnetMACLogging->setVector(transmit_t, opp_strdup(name));
						transmittingTimeStatistics->collect(transmit_t);
					}
				}
				delete name;
	
				lastFrameLength = frame->length();
				frame->addObject(new cMessage("MAC", id()));
				send(frame, "LowerLayerOut");
					ev << MODULENAME << ": Message \"" << frame->name() << "\" send at t = "
						<< printTime(simTime()) << "\n";
				delete scheduledQueue.pop();
				updateDisplayString();
	
				// logging help
				if (!scheduledQueue.empty())
				{
					ControlNetFrame* candidate = (ControlNetFrame*) scheduledQueue.tail();
					if (candidate->getTimeStamp() < 0)
					{
						candidate->setTimeStamp(simTime());
						if (ev.isGUI()) bubble("TimeStamp set!");
					}
				}
			}
			else
			{
				handleEndScheduled();
				ev << MODULENAME << ": End of scheduled period\n";
			}
		}

		// end of scheduled period?
		if (macID == SMAX)
		{
			handleEndScheduled();
		}
	}
	else
	{
		if (unscheduledQueue.empty())
		{
			ControlNetFrame* frame = new ControlNetFrame("NULL", NUL);
			frame->setSrcAddress(macID);
			frame->setLength(FRAME_MIN);
			send(frame, "LowerLayerOut");
				ev << MODULENAME << ": Message \"" << frame->name() << "\" send at t = "
					<< printTime(simTime()) << "\n";
		}
		else
		{
			if (simTime() + MAX_LENGTH / PSPEED + (double) ((ControlNetFrame*)
				unscheduledQueue.tail())->length() / TXRATE <= uLimit)
			{
				ControlNetFrame* frame = (ControlNetFrame*) unscheduledQueue.tail()->dup();
	
				// logging
				framesSent++;
				bitsSent += frame->length();
				// time calculation
				if (par("write1").boolValue())
				{
					if (frame->getTimeStamp() == -1.0)
						blockingTime = 0.0;
					else
					{
						if (fabs(simTime() - frame->getTimeStamp()) < par("accuracyFP").doubleValue())
							blockingTime = 0.0;
						else
							blockingTime = simTime() - frame->getTimeStamp();
					}
					if (fabs(simTime() - frame->timestamp()) < par("accuracyFP").doubleValue())
						queueWaitingTime = 0.0;
					else
						queueWaitingTime = simTime() - frame->timestamp();
					if (fabs(queueWaitingTime + blockingTime) < par("accuracyFP").doubleValue())
						waitingTime = 0.0;
					else
						waitingTime = queueWaitingTime + blockingTime;
				}
				else
				{
					if (frame->getTimeStamp() == -1.0)
						blockingTime = 0.0;
					else
					{
						if (fabs(simTime() - IFG - frame->getTimeStamp()) < par("accuracyFP").doubleValue())
							blockingTime = 0.0;
						else
							blockingTime = simTime() - IFG - frame->getTimeStamp();
					}
					if (fabs(simTime() - IFG - frame->timestamp()) < par("accuracyFP").doubleValue())
						queueWaitingTime = 0.0;
					else
						queueWaitingTime = simTime() - IFG - frame->timestamp();
					if (fabs(queueWaitingTime + blockingTime) < par("accuracyFP").doubleValue())
						waitingTime = 0.0;
					else
						waitingTime = queueWaitingTime + blockingTime;
				}
				// for message travelling
				frame->setTimeStamp(simTime());
	
				char* name = new char[256];
				if (par("framesSentVector").doubleValue() >= 0)
				{
					sprintf(name, "framesSent:%i", macID);
					controlnetMACLogging->setVector(framesSent, opp_strdup(name));
				}
				if (par("bitsSentVector").doubleValue() >= 0)
				{
					sprintf(name, "bitsSent:%i", macID);
					controlnetMACLogging->setVector(bitsSent, opp_strdup(name));
				}
				if (par("waitingTimeVector").doubleValue() >= 0)
				{
					sprintf(name, "waitingTime:%i", macID);
					controlnetMACLogging->setVector(waitingTime, opp_strdup(name));
					waitingTimeStatistics->collect(waitingTime);
				}
				waitingTime = 0.0;
				if (par("blockingTimeVector").doubleValue() >= 0)
				{
					sprintf(name, "blockingTime:%i", macID);
					controlnetMACLogging->setVector(blockingTime, opp_strdup(name));
					blockingTimeStatistics->collect(blockingTime);
				}
				blockingTime = 0.0;
				if (par("queueWaitingTimeVector").doubleValue() >= 0)
				{
					sprintf(name, "queueWaitingTime:%i", macID);
					controlnetMACLogging->setVector(queueWaitingTime, opp_strdup(name));
					queueWaitingTimeStatistics->collect(queueWaitingTime);
				}
				if (par("write1").boolValue())
				{
					simtime_t transmit_t = MAX_LENGTH / PSPEED + (double) frame->length() / TXRATE;
					if (par("messageDelayVector").doubleValue() >= 0)
					{
						sprintf(name, "messageDelay:%i", macID);
						controlnetMACLogging->setVector(simTime() - frame->timestamp() + transmit_t,
							opp_strdup(name));
						messageDelayStatistics->collect(simTime() - frame->timestamp() + transmit_t);
					}
					if (par("transmittingTimeVector").doubleValue() >= 0)
					{
						sprintf(name, "transmittingTime:%i", macID);
						controlnetMACLogging->setVector(transmit_t, opp_strdup(name));
						transmittingTimeStatistics->collect(transmit_t);
					}
				}
				delete name;
	
				lastFrameLength = frame->length();
				frame->addObject(new cMessage("MAC", id()));
				send(frame, "LowerLayerOut");
					ev << MODULENAME << ": Message \"" << frame->name() << "\" send at t = "
						<< printTime(simTime()) << "\n";
				delete unscheduledQueue.pop();
				updateDisplayString();
	
				// logging help
				if (!unscheduledQueue.empty())
				{
					ControlNetFrame* candidate = (ControlNetFrame*) unscheduledQueue.tail();
					if (candidate->getTimeStamp() < 0)
					{
						candidate->setTimeStamp(simTime());
						if (ev.isGUI()) bubble("TimeStamp set!");
					}
				}
			}
			else
			{
				handleEndUnscheduled();
				ev << MODULENAME << ": End of unscheduled period\n";
			}
		}
	}
}

void ControlNetMAC::handleEndGB()
{
	if (debug) ev << MODULENAME << ": handleEndGB entered\n";

	scheduledToken = 1;
	unscheduledToken = (preUToken + 1 > UMAX ? 1 : preUToken + 1);
	preUToken = unscheduledToken;

	uLimit = simTime() + NUT - GB;// - IFG - MAX_LENGTH / PSPEED;
	// end GB for this NUT
	scheduleAt(uLimit + GB, endGBMsg);
	ev << MODULENAME << ": Event \"" << endGBMsg->name() << "\" scheduled at t = "
		<< printTime(uLimit + GB) << endl;

	if (SMAX > 0)
	{
		// state is scheduled now
		state = SCHEDULED_STATE;

		if (macID == 1)
		{
			// send first scheduled frame to first node
			cMessage* msg = new cMessage("ScheduleInit");
			handleScheduled(msg);
		}
	}
	else
	{
		// state is unscheduled now
		state = UNSCHEDULED_STATE;

		if (macID == unscheduledToken)
		{
			// send first unscheduled frame to first node
			cMessage* msg = new cMessage("UnscheduleInit", unscheduledToken);
			handleUnscheduled(msg);
		}
	}
}

void ControlNetMAC::handleEndScheduled()
{
	if (debug) ev << MODULENAME << ": handleEndScheduled entered\n";

	ev << MODULENAME << ": End of scheduled period\n";

	// state is unscheduled now
	state = UNSCHEDULED_STATE;

	if (macID == unscheduledToken)
	{
		// send first unscheduled frame to first node
		cMessage* msg = new cMessage("UnscheduleInit", unscheduledToken);
		handleUnscheduled(msg);
	}
}

void ControlNetMAC::handleEndUnscheduled()
{
	if (debug) ev << MODULENAME << ": handleEndUnscheduled entered\n";

	// state is guard band now
	state = GUARDBAND_STATE;
}

void ControlNetMAC::finish()
{
	if (par("write1").boolValue())
	{
		if (par("waitingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->write1Statistics(waitingTimeStatistics);
		if (par("blockingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->write1Statistics(blockingTimeStatistics);
		if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->write1Statistics(queueWaitingTimeStatistics);
		if (par("transmittingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->write1Statistics(transmittingTimeStatistics);
		if (par("messageDelayStatistics").doubleValue() >= 0)
			controlnetMACLogging->write1Statistics(messageDelayStatistics);
	}
	else
	{
		if (par("waitingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->writeStatistics(waitingTimeStatistics);
		if (par("blockingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->writeStatistics(blockingTimeStatistics);
		if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->writeStatistics(queueWaitingTimeStatistics);
		if (par("transmittingTimeStatistics").doubleValue() >= 0)
			controlnetMACLogging->writeStatistics(transmittingTimeStatistics);
		if (par("messageDelayStatistics").doubleValue() >= 0)
			controlnetMACLogging->writeStatistics(messageDelayStatistics);
	}
	controlnetMACLogging->writeMessageTimeDelay();
	if (par("writeScalars").boolValue())
	{
		char* name = new char[256];
		sprintf(name, "unsentMessages:%i", macID);
		recordScalar(name, scheduledQueue.length() + unscheduledQueue.length());
		delete name;
	}
}

void ControlNetMAC::initLogging()
{
	char* name = new char[256];
	char* description = new char[256];

	if (par("framesSentVector").doubleValue() >= 0)
	{
		// frames sent vector
		sprintf(name, "framesSent:%i", macID);
		sprintf(description, "'frames sent:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesSentVector").doubleValue());
		controlnetMACLogging->setVector(framesSent, opp_strdup(name));
		if (par("framesSentStatistics").doubleValue() >= 0)
		{
			// frames sent statistics
			sprintf(description, "'frames sent statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesSentStatistics").doubleValue());
		}
	}

	if (par("framesReceivedVector").doubleValue() >= 0)
	{
		// frames received vector
		sprintf(name, "framesReceived:%i", macID);
		sprintf(description, "'frames received:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesReceivedVector").doubleValue());
		controlnetMACLogging->setVector(framesReceived, opp_strdup(name));
		if (par("framesReceivedStatistics").doubleValue() >= 0)
		{
			// frames received statistics
			sprintf(description, "'frames received statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesReceivedStatistics").doubleValue());
		}
	}

	if (par("bitsSentVector").doubleValue() >= 0)
	{
		// bits sent vector
		sprintf(name, "bitsSent:%i", macID);
		sprintf(description, "'bits sent:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("bitsSentVector").doubleValue());
		controlnetMACLogging->setVector(bitsSent, opp_strdup(name));
		if (par("bitsSentStatistics").doubleValue() >= 0)
		{
			// bits sent statistics
			sprintf(description, "'bits sent statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("bitsSentStatistics").doubleValue());
		}
	}

	if (par("bitsReceivedVector").doubleValue() >= 0)
	{
		// bits received vector
		sprintf(name, "bitsReceived:%i", macID);
		sprintf(description, "'bits received:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("bitsReceivedVector").doubleValue());
		controlnetMACLogging->setVector(bitsReceived, opp_strdup(name));
		if (par("bitsReceivedStatistics").doubleValue() >= 0)
		{
			// bits received statistics
			sprintf(description, "'bits received statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("bitsReceivedStatistics").doubleValue());
		}
	}

	if (par("messagesPassedVector").doubleValue() >= 0)
	{
		// messages passed vector
		sprintf(name, "messagesPassed:%i", macID);
		sprintf(description, "'messages passed:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesPassedVector").doubleValue());
		controlnetMACLogging->setVector(messagesPassed, opp_strdup(name));
		if (par("messagesPassedStatistics").doubleValue() >= 0)
		{
			// messages passed statistics
			sprintf(description, "'messages passed statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesPassedStatistics").doubleValue());
		}
	}

	if (par("messagesReceivedVector").doubleValue() >= 0)
	{
		// messages received vector
		sprintf(name, "messagesReceived:%i", macID);
		sprintf(description, "'messages received:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesReceivedVector").doubleValue());
		controlnetMACLogging->setVector(messagesReceived, opp_strdup(name));
		if (par("messagesReceivedStatistics").doubleValue() >= 0)
		{
			// messages received statistics
			sprintf(description, "'messages received statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesReceivedStatistics").doubleValue());
		}
	}

	if (par("messagesDroppedBOVector").doubleValue() >= 0)
	{
		// messages dropped - buffer overflow - vector
		sprintf(name, "messagesDroppedBO:%i", macID);
		sprintf(description, "'messages dropped - buffer overflow:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesDroppedBOVector").doubleValue());
		controlnetMACLogging->setVector(messagesDroppedBO, opp_strdup(name));
		if (par("messagesDroppedBOStatistics").doubleValue() >= 0)
		{
			// messages dropped - buffer overflow - statistics
			sprintf(description, "'messages dropped - buffer overflow - statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesDroppedBOStatistics").doubleValue());
		}
	}

	if (par("messagesDroppedBEVector").doubleValue() >= 0)
	{
		// messages dropped - bit error - vector
		sprintf(name, "messagesDroppedBE:%i", macID);
		sprintf(description, "'messages dropped - bit error:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("messagesDroppedBEVector").doubleValue());
		controlnetMACLogging->setVector(messagesDroppedBE, opp_strdup(name));
		if (par("messagesDroppedBEStatistics").doubleValue() >= 0)
		{
			// messages dropped - bit error - statistics
			sprintf(description, "'messages dropped - bit error - statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("messagesDroppedBEStatistics").doubleValue());
		}
	}
	
	if (par("framesRetransmittedVector").doubleValue() >= 0)
	{
		// frames retransmitted vector
		sprintf(name, "framesRetransmitted:%i", macID);
		sprintf(description, "'frames retransmitted:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0L,
			par("framesRetransmittedVector").doubleValue());
		controlnetMACLogging->setVector(framesRetransmitted, opp_strdup(name));
		if (par("framesRetransmittedStatistics").doubleValue() >= 0)
		{
			// frames retransmitted statistics
			sprintf(description, "'frames retransmitted statistics:%i'", macID);
			controlnetMACLogging->initStatistics(opp_strdup(name), opp_strdup(description),
				par("framesRetransmittedStatistics").doubleValue());
		}
	}

	if (par("waitingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "waitingTime:%i", macID);
		sprintf(description, "'waiting time:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("waitingTimeVector").doubleValue());
		controlnetMACLogging->setVector(waitingTime, opp_strdup(name));
		if (par("waitingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'waiting time statistics:%i'", macID);
			waitingTimeStatistics = new cStdDev(opp_strdup(description));
//			waitingTimeStatistics->collect(waitingTime);
		}
	}

	if (par("blockingTimeVector").doubleValue() >= 0)
	{
		// blocking time
		sprintf(name, "blockingTime:%i", macID);
		sprintf(description, "'blocking  time:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("blockingTimeVector").doubleValue());
		controlnetMACLogging->setVector(blockingTime, opp_strdup(name));
		if (par("blockingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'blocking time statistics:%i'", macID);
			blockingTimeStatistics = new cStdDev(opp_strdup(description));
//			blockingTimeStatistics->collect(blockingTime);
		}
	}

	if (par("queueWaitingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "queueWaitingTime:%i", macID);
		sprintf(description, "'queue waiting time:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("queueWaitingTimeVector").doubleValue());
		controlnetMACLogging->setVector(queueWaitingTime, opp_strdup(name));
		if (par("queueWaitingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'queue waiting time statistics:%i'", macID);
			queueWaitingTimeStatistics = new cStdDev(opp_strdup(description));
//			queueWaitingTimeStatistics->collect(queueWaitingTime);
		}
	}

	simtime_t transmittingTime = 0.0;
	if (par("transmittingTimeVector").doubleValue() >= 0)
	{
		// waiting time vector
		sprintf(name, "transmittingTime:%i", macID);
		sprintf(description, "'transmitting time:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("transmittingTimeVector").doubleValue());
		controlnetMACLogging->setVector(transmittingTime, opp_strdup(name));
		if (par("transmittingTimeStatistics").doubleValue() >= 0)
		{
			// waiting time statistics
			sprintf(description, "'transmitting time statistics:%i'", macID);
			transmittingTimeStatistics = new cStdDev(opp_strdup(description));
//			transmittingTimeStatistics->collect(transmittingTime);
		}
		if (par("dividedStatistics").boolValue())
		{
			// message delay statistics for every MAC
			for (int i = 1; i <= par("hosts").longValue(); i++)
			{
				if (i != macID)
				{
					sprintf(name, "transmittingTime->%i:%i", macID, i);
					sprintf(description, "'transmitting time->%i:%i'", macID, i);
					controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description),
						0.0);
//					controlnetMACLogging->setVector(transmittingTime, opp_strdup(name));
				}
			}
		}
	}

	simtime_t messageDelay = 0.0;
	if (par("messageDelayVector").doubleValue() >= 0)
	{
		// message delay vector
		sprintf(name, "messageDelay:%i", macID);
		sprintf(description, "'message delay:%i'", macID);
		controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description), 0.0,
			par("messageDelayVector").doubleValue());
		controlnetMACLogging->setVector(messageDelay, opp_strdup(name));
		if (par("messageDelayStatistics").doubleValue() >= 0)
		{
			// message delay statistics
			sprintf(description, "'message delay statistics:%i'", macID);
			messageDelayStatistics = new cStdDev(opp_strdup(description));
//			messageDelayStatistics->collect(messageDelay);
		}
		if (par("dividedStatistics").boolValue())
		{
			// message delay statistics for every MAC
			for (int i = 1; i <= par("hosts").longValue(); i++)
			{
				if (i != macID)
				{
					sprintf(name, "messageDelay->%i:%i", macID, i);
					sprintf(description, "'message delay->%i:%i'", macID, i);
					controlnetMACLogging->initVector(opp_strdup(name), opp_strdup(description),
						0.0, par("messageDelayVector").doubleValue());
//					controlnetMACLogging->setVector(messageDelay, opp_strdup(name));
				}
			}
		}
	}
	
	delete name;
	delete description;
}

void ControlNetMAC::collectMessageDelay(simtime_t time)
{
	if (debug) ev << MODULENAME << ": collectMessageDelay entered\n";
	
	Enter_Method("collectMessageDelay(simtime_t time)");

	messageDelayStatistics->collect(time);
}

void ControlNetMAC::collectTransmittingTime(simtime_t time)
{
	if (debug) ev << MODULENAME << ": collectTransmittingTime entered\n";

	Enter_Method("collectTransmittingTime(simtime_t time)");

	transmittingTimeStatistics->collect(time);
}
