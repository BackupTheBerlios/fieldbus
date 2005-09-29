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

#ifndef BUS_H
#define BUS_H
#include <omnetpp.h>
#include "Utils.h"
#include "Util.h"
#include "BusLogging.h"
#include "EthernetFrame_m.h"

/*
 * This is a definition file for the Bus.
 */

/**
 * Direction of frame travel on bus. It's used for self-message kinds. Upstream means the
 * message is going from taps with higher id to taps with lower id (graphically left direction).
 */
#define UPSTREAM	0

/**
 * Direction of frame travel on bus. It's used for self-message kinds. Downstream means the
 * message is going from taps with lower id to taps with higher id (graphically right direction).
 */
#define DOWNSTREAM	1

/**
 * Direction of frame travel on bus. It's used for self-message kinds. Bothstream means the
 * message is going upstream and downstream from one tap (graphically right and left  direction).
 */
#define BOTHSTREAM	2

/**
 * The protocol definition. For Ethernet zero.
 */
#define ETHERNET	0

/**
 * The protocol definition. For DeviceNet one.
 */
#define DEVICENET	1

/**
 * The protocol definition. For ControlNet two.
 */
#define CONTROLNET	2

/**
 * The protocol definition. For LonTalk three.
 */
#define LONTALK	3

/**
 * The states of a tap. Free means there is no message going through this tap.
 */
#define FREE	0

/**
 * The states of a tap. Busy means there a message going through this tap.
 */
#define BUSY	1

/**
 * The states of a tap. Competing means  there is no message going through this tap
 * but (Ethernet and LonTalk only) we are in a "after collision state" and that it is possible
 * that more collided messages arrive on this tap .
 */
#define COMPETING	2

/**
 * Frame interval flag. This is for the frame interval.
 */
#define BEGIN	0

/**
 * Frame interval flag. This is for the frame interval.
 */
#define END	1

/**
 * This is the abstraction of a bus tap. It implements the physical locations on the bus
 * where each network entity is connected to the bus.
 */
struct BusTap {
	/**
	 * Tap id. Which tap is this.
	 */
	int id;
	
	/**
	 * My position. Physical location of where each entity is connected to on the bus 
	 * (physical location of the tap on the bus).
	 */
	double position;
	
	/**
	 * Propagation delays. The array contains the adjacent tap points on the bus: array[0] 
	 * : upstream, array[1] : downstream time.
	 */
	simtime_t propagationDelay[2];
	
	/**
	 * Tap states. Three states are possible: @see FREE, @see BUSY and @see COMPETING.
	 */
	short frameOnTap;
	
	/**
	 * The frame interval calculated in which the frame will pass this tap.
	 */
	simtime_t frameInterval[2];
	
	/**
	 * One self-message. This message inform when to deliver the complete frame and is
	 * to be unscheduled if a collision occure.
	 */
	cMessage* endTransmission;
	
	/**
	 * The direction of the last message walked through this tap. It can be @see UPSTREAM,
	 * @see DOWNSTREAM and @see BOTHSTREAM.
	 */
	short lastMessageDirection;
	
	/**
	 *  The time of the last message walked through this tap.
	 */
	simtime_t lastMessageTime;
};

/**
 * Implements the bus which connects devices via a bus.
 *
 * The Bus is providing a handling for CSMA/CD based and for non-CSMA/CD based protocols.
 * Every message is sended to every tap on the bus. A Message is brought to the next tap when the
 * propagation + transmisson time is over. It includes collided messages, only in
 * fact it is a CSMA/CD system, and messages with bit errors.
 */
class Bus : public cSimpleModule
{
private:
	/**
	 * Debugging flag. It decides is additional debugging information is printet on output.
	 */
	bool debug;
	
	/**
	 * Propagation speed of signals. Every copper has its own speed which is mostly between.
	 * 0.5 * c and 0.8 * c
	 */
	double propagationSpeed;
	
	/**
	 * Bus taps. They are physical locations of where the hosts is connected to the bus.
	 */
	BusTap* tap;
	
	/**
	 * Bit rate is the bit per sec. It is the speed  the devices are sending on the bus.
	 */
	double bitrate;
	
	/**
	 * The name of the MAC protocol running. Possible is @see ETHERNET, @see DEVICENET,
	 * @see CONTROLNET and @see LONTALK.
	 */
	int protocol;
	
	/**
	 * Number of tap points on the bus. It have to always > 1.
	 */
	int taps;
	
	/**
	 * Number of messages handled. This includes all messages sent on the bus.
	 */
	long messages;
	
	/**
	 * Number of bit errors appeared. Every message with a bit errror is counted.
	 */
	long biterrors;
	
	/**
	 * Logging member. It is used to log every value needed for statistics.
	 */
	Logging* busLogging;

	/**
	 * Logging module. It is only needed to instantiate the logging member. @see busLogging
	 */
	cModule* busLoggingModule;
	
private:
	/**
	 * The initialization routine for Ethernet. Here we set tap positions with controling the
	 * tap distance, propagation speed from one tap to his neighbours and initialize the
	 * values of member data.
	 */
	void EthernetInitialize();

	/**
	 * The initialization routine for DeviceNet. We don't set tap positions because all receiving
	 * stations get the message at the same time. Propagation speed is not interresting us,
	 * only the bit rate. Values of member data are initialized.
	 */
	void DeviceNetInitialize();

	/**
	 * The initialization routine for ControlNet. Here we set tap positions with controling the
	 * tap distance, propagation speed from one tap to his neighbours and initialize the
	 * values of member data.
	 */
	void ControlNetInitialize();

	/**
	 * The initialization routine for LanTalk. @see EthernetInitialize()
	 */
	void LonTalkInitialize();
	
	/**
	 * The Ethernet message handle function. This message distribution algorithm is the
	 * most coplicated one. It depends on message length, bit rate, propagation time and
	 * possible collisions that can occur. In a non-collision case the frame is brought to the
	 * tap like it is in ControlNet. But in a collision case a device on bus can see it immediatly
	 * after the "first bit" has been propagate. A bit error flag is set if the message contains
	 * errors from traveling through the bus.
	 * <br><b>Note:</b> <i>The propagation calculation depends on simulation configuration
	 * and can be turned off.</i>
	 * @see ControlNetHandleMessage(cMessage* msg)
	 * @param msg is the message object we get
	 */
	void EthernetHandleMessage(cMessage* msg);

	/**
	 * The DeviceNet message handle function. This message distribution algorithm is the
	 * simplest. When a message arrives on a tap only the message length and the bit rate
	 * are resposible for message delay. The complete message (message copies) is then
	 * send to all tap at the same time, excluding the own tap. A bit error flag is set if the
	 * message contains errors through traveling through the bus.
	 * @param msg is the message object we get
	 */
	void DeviceNetHandleMessage(cMessage* msg);

	/**
	 * The ControlNet message handle function. In this distribution algorithm we have the
	 * length of a message, the bit rate and the propagation time that together are responsible
	 * for the time a complete message (message copies) arrives on a tap. A bit error flag is
	 * set if the message contains errors through traveling through the bus.
	 * <br><b>Note:</b> <i>The propagation calculation depends on simulation configuration
	 * and can be turned off.</i>
	 * @param msg is the message object we get
	 */
	void ControlNetHandleMessage(cMessage* msg);

	/**
	 * The LanTalk message handle function. @see EthernetHandleMessage(cMessage* msg)
	 */
	void LonTalkHandleMessage(cMessage* msg);

	/**
	 * The Ethernet finish member. All statistics are written here.
	 */
	void EthernetFinish();

	/**
	 * The DeviceNet finish member. All statistics are written here.
	 */
	void DeviceNetFinish();

	/**
	 * The ControlNet finish member. All statistics are written here.
	 */
	void ControlNetFinish();

	/**
	 * The LonTalk finish member. All statistics are written here.
	 */
	void LonTalkFinish();

	/**
	 * A helper function. \deprecated
	 * It is used to parse string parameter. See OMNeT's user manual v.3.1.
	 * @param str is the string to be parsed
	 * @param array is the data structure to save the positions in
	 */
	void tokenize(const char* str, std::vector<double>& array);
	
	/**
	 * The bit error calculation member. Every bit in a message is calculated.
	 * @param msg is the message object we get
	 */
	void setBiterror(cMessage* msg);
	
	/**
	 * Checks if the tap is free. A message is send back to the upper layer to inform whether
	 * the bus is free or not.
	 * @param msg is the message object we get
	 */
	void busFree(cMessage* msg);

	/**
	 * Prints member value of a tap. This can be useful for debbuging. 
	 * @param tapPoint is the actual tap
	 */
	void printState(int tapPoint);

	/**
	 * Handles frames comming from the upper layer. We schedule upstream and downstream
	 * respectively or bothstream events. This stands for the propagation of the "first bit". An
	 * "end transmission" event also is scheduled. In the case everything goes well, we send
	 * the complete frames to the taps.
	 * @param msg is the message object we get
	 */
	void handleFrame(cMessage* msg);
	
	/**
	 * Handle self-messages. It includes timer signals and tap messages.
	 * @param msg is the message object we get
	 */
	void handleSelfMessage(cMessage* msg);
	
	/**
	 * Transmission complete handling. We are ready to propagate the complete frames.
	 * @param msg is the message object we get
	 */
	void endTransmission(cMessage* msg);
	
	/**
	 * Stream handling. We propagate the "first bit". It doesn't really transmit a bit to a device
	 * but we have to set a lot values on the taps, i.e. for collision detection.
	 * @param msg is the message object we get
	 */
	void handleStream(cMessage* msg);
	
	/**
	 * Collision handling. The frames on the bus don't imform us about collision. That's why
	 * we have to calculate them. Let's have a look at the following image:
	 * <br><img src="../../Documentation/collision.png"><br>
	 * A's and B's frames don't do a collision between A's an B's tap. But between B's and
	 * C's tap. That's why B see the collision after A's "first bit" passes the tap. Concusion:
	 * we have to calculate the collision for every tap.
	 * <br><b>Note:</b> <i>Only the propagation of th "first bit" is shown. That is only the
	 * begin of a whole frame. The red arrows symbolize collided frame begin.</i>
	 * @param msg is the message object we get
	 */
	void handleCollision(cMessage* msg);

	/**
	 * Here we handle the competing state. We are not sure if after the arriving of a collided
	 * frame another collided frames won't come. Let's have a look at the following image:
	 * <br><img src="../../Documentation/collision.png"><br>
	 * This state you can see for B. He send his "first bits" at \f$t=5\f$ after a collision with
	 * no idea of what is happen next: his frame gets into a collision because an other frame
	 * had to propagate to his tap!
	 * <br><b>Note:</b> <i>Only the propagation of th "first bit" is shown. That is only the
	 * begin of a whole frame. The red arrows symbolize collided frame begin.</i>
	 * @param msg is the message object we get
	 */
	void handleCompeting(cMessage* msg);

	/**
	 * The busy state is to be handled. A frame arrived on a tap that is busy can only be a
	 * collided message. That's why we send a collided message (original message with a
	 * collision object) on that tap.
	 * @param msg is the message object we get
	 */
	void handleBusy(cMessage* msg);

	/**
	 * Bus arbitration handling. Distributes the arbitration message around all nodes without
	 * a propagation time. This is necessery because we don't know how many nodes send
	 * their arbitration messages while we are collectiong them. At the end of the arbitration
	 * period we can handle them, they all are collected.
	 * @param msg is the message object we get
	 */
	void busArbitration(cMessage* msg);

	void handleConstMessage(cMessage* msg);
public:

	/**
	 * The class declaration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
	 * See OMNeT's user manual.</i>
	 */
	Module_Class_Members(Bus, cSimpleModule, 0);
	
	/**
	 * The initialization routine for Bus. We only switch to other protocol dependent initialization
	 * routines.
	 */
	virtual void initialize();

	/**
	 * The Bus message handle function. We only switch to other protocol dependent handle
	 * functions.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);
	
	/**
	 * The Bus finish member. We only switch to other protocol dependent finish member.
	 */
	virtual void finish();
};

/**
 * The module type registration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
 * See OMNeT's user manual.</i>
 */
Define_Module(Bus);

#endif
