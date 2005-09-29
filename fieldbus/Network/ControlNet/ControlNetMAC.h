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

#ifndef DEVICENETMAC_H
#define DEVICENETMAC_H
#include <omnetpp.h>
#include "Util.h"
#include "MAC.h"
#include "ControlNetFrame_m.h"
#include "ControlNetMACLogging.h"

/*
 * This is a definition file for the ControlNet MAC.
 */

/**
 * The frame maximum length. This is a bit value.
 */
#define FRAME_MAX	(517 * 8)

/**
 * The frame minimum length. This is a bit value.
 */
#define FRAME_MIN	(7 * 8)

/**
 * The data field maximum length. This is a bit value.
 */
#define DATA_MAX	(510 * 8)

/**
 * The preamble field length. This is a bit value.
 */
#define PREAMBLE	(2 * 8)

/**
 * The start delimiter field length. This is a bit value.
 */
#define SD	(1 * 8)

/**
 * The source field length. This is a bit value.
 */
#define SRC_ADR	(1 * 8)

/**
 * The fixed LPacket header field length. This is a bit value.
 */
#define F_LPACKET_HEADER	(4 * 8)

/**
 * The CRC field length. This is a bit value.
 */
#define CRC	(2 * 8)

/**
 * The end delimiter field length. This is a bit value.
 */
#define ED	(1 * 8)

/**
 * The IFG time. This is a time value.
 */
#define IFG	(par("IFG").doubleValue())

/**
 * Other DeviceNet dependend definitionen. That one is the transmit rate in bit per sec.
 */
#define TXRATE	(par("bitrate").longValue())

/**
 * Other DeviceNet dependend definitionen. That one is the guard bnad time length in sec.
 */
#define GB	(par("GB").doubleValue())

/**
 * Other DeviceNet dependend definitionen. That one is the NUT time length in sec.
 */
#define NUT	(par("NUT").doubleValue())

/**
 * Other DeviceNet dependend definitionen. That one is the SMAX number.
 */
#define SMAX	((int) par("SMAX").longValue())

/**
 * Other DeviceNet dependend definitionen. That one is the UMAX number.
 */
#define UMAX	((int) par("UMAX").longValue())

/**
 * Other ControlNet dependend definitionen. That one is the maximum length of the bus
 * medium in m.
 */
#define MAX_LENGTH	(par("maximumLength").doubleValue())

/**
 * Other ControlNet dependend definitionen. That one is the propagation speed of the
 * bus medium in m/sec.
 */
#define PSPEED	(par("propagationSpeed").doubleValue())

/**
 * The MAC states. This is the definition for the scheduled area state.
 */
#define SCHEDULED_STATE	10

/**
 * The MAC states. This is the definition for the unscheduled area state.
 */
#define UNSCHEDULED_STATE	11

/**
 * The MAC states. This is the definition for the guard band area state.
 */
#define GUARDBAND_STATE	12

/**
 * The MAC states. This is the definition for the IFG wait state.
 */
#define WAIT_IFG_STATE	13

/**
 * The self-message kinds. This one is used for the end of IFG period for sending a
 * normal frame.
 */
#define ENDIFG	20

/**
 * The self-message kinds. This one is used for the end of the guardband period.
 */
#define ENDGB	21

/**
 * This is the ControlNet MAC class.
 * 
 * It implements the ControlNet CI (ControlNet International) and IEC61158 standard. It
 * manages two virtual tokens, one for scheduled messages, one for unscheduled
 * messages. The global time, e.g. time synchronization is set implicitly.
 * 
 * Restrictions are:<ul>
 * <li>no moderator frame is send: network management is done through parameters
 * set in configuration files at the beggining of the simulation
 * <li>a node can't fail sending, so there are only message and NULL frames sent on
 * the bus
 * <li>how much "free" time in the unscheduled area is left is calculated based on the
 * propagation time for a specific medium and maximum length
 * <li>data is put into one fixed LPacket</ul>
 */
class ControlNetMAC : public MAC
{
private:
	/**
	 * Scheduled messages queue. This queue holds prior messages that have to be
	 * send in the scheduled period.
	 */
	 cQueue scheduledQueue;

	/**
	 * Unscheduled messages queue. This queue holds non-prior messages that can be
	 * send in the unscheduled period.
	 */
	 cQueue unscheduledQueue;

	/**
	 * Capacity of the scheduled queue. This parameter must be set in configuration of the
	 * simulation.
	 */
    int maxScheduledQueueSize;

	/**
	 * Capacity of the unscheduled queue. This parameter must be set in configuration of
	 * the simulation.
	 */
    int maxUnscheduledQueueSize;

	/**
	 * MAC ID. This is a unique identifier for the MAC.
	 */
	int macID;

	/**
	 * Scheduled area token. This implicit token has all the time the same value in all nodes!
	 * This is important to prevent collisions on the bus. A node registering the token value
	 * equal the own MAC ID has right to send a frame.<br>
	 * <b>Note:</b> <i>This value must only be in a range between 0 and SMAX.</i>
	 * @see SMAX
	 */
	 int scheduledToken;

	/**
	 * Uncheduled area token. This implicit token has all the time the same value in all nodes!
	 * This is important to prevent collisions on the bus. A node registering the token value
	 * equal the own MAC ID has right to send a frame.<br>
	 * <b>Note:</b> <i>This value must only be in a range between 0 and UMAX.</i>
	 * @see UMAX
	 */
	 int unscheduledToken;

	/**
	 * MAC's state. The MAC differ in following states: @see SCHEDULED_STATE, @see
	 * UNSCHEDULED_STATE, @see GUARDBAND_STATE or WAIT_IFG_STATE.
	 */
	int state;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDIFG
	 */
	cMessage* endIFGMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDGB
	 */
	cMessage* endGBMsg;

	/**
	 * Logging member. It is used to log every value needed for statistics.
	 */
	ControlNetMACLogging* controlnetMACLogging;

	/**
	 * Logging module. It is only needed to instantiate the logging member. @see controlnetMACLogging
	 */
	cModule* controlnetMACLoggingModule;

	/**
	 * Unscheduled area limit time. This value gives us the limit of the unscheduled area
	 * in sec.
	 */
	simtime_t uLimit;

	/**
	 * Previous unscheduled token. We need this value to increment the unscheduled token
	 * in the next NUT: @see unscheduledToken.
	 */
	int preUToken;

	/**
	 * Length of the last sent frame. This is needed for the first frame sent in the unscheduled
	 * period by the node UMAX. @see
	 */
	long lastFrameLength;

	/**
	 * Frames sent. Counter for sent frames to lower layer.
	 */
	long framesSent;

	/**
	 * Frames received. Counter for received frames from lower layer.
	 */
	long framesReceived;

	/**
	 * Bits sent. Counter for sent bits to lower layer.
	 */
	long bitsSent;

	/**
	 * Bits received. Counter for received bits from lower layer.
	 */
	long bitsReceived;

	/**
	 * Messages passed. Counter for passed messages from lower layer to higher layer.
	 */
	long messagesPassed;

	/**
	 * Messages received. Counter for received messages from higher layer.
	 */
	long messagesReceived;

	/**
	 * Messages dropped. Counter for dropped messages from higher layer because the
	 * buffer was full (buffer overflow).
	 */
	long messagesDroppedBO;

	/**
	 * Messages dropped. Counter for dropped messages from lower layer because they
	 * had bit errors.
	 */
	long messagesDroppedBE;

	/**
	 * Frames retransmitted. Counter for retransmitted frames in collision cases.
	 */
	long framesRetransmitted;

	/**
	 * Message waiting time statistics. Here we collect all about the message waiting time.
	 */
	cStdDev* waitingTimeStatistics;
	
	/**
	 * Queue waiting time statistics. Here we collect all about the queue waiting time for messages.
	 */
	cStdDev* queueWaitingTimeStatistics;
	
	/**
	 * Message blocking time statistics. Here we collect all about the blocking time for messages.
	 */
	cStdDev* blockingTimeStatistics;
	
	/**
	 * Transmitting time statistics. Here we collect all about the transmission time of  messages.
	 */
	cStdDev* transmittingTimeStatistics;

	/**
	 * Message delay statistics. Here we collect all about the delaying of  messages.
	 */
	cStdDev* messageDelayStatistics;

	/**
	 * Cumulated queue waiting time. Whole waiting time of every message waiting in the
	 * queue.
	 */
	simtime_t waitingTime;
	
	/**
	 * Message blocking time. A temporal variable who stores the blocking time for the actual
	 * message.
	 */	
	simtime_t blockingTime;

	/**
	 * Queue wiating  time. A temporal variable who stores the waiting time for the actual
	 * message in the queue.
	 */	
	simtime_t queueWaitingTime;

private:
	/**
	 * Here we handle messages comming from upper layer. 
	 * @param msg is the message object we get
	 */
	virtual void handleUpperLayerMessage(cMessage* msg);

	/**
	 * A frame comming from lower layer is handled here. 
	 * @param msg is the message object we get
	 */
	virtual void handleLowerLayerMessage(cMessage* msg);

	/**
	 * This is the encapsulate function. Here we put all relevant information in the frame
	 * fields including message encapsulation.
	 * @param msg is the message object we get
	 * @return the encapsulated message
	 */
	cMessage* encapsulateData(cMessage* msg);

	/**
	 * A helper. For debug option. We can see details about the frame build.
	 * @param msg is the message object we get
	 */
	void frameDetails(cMessage* msg);

	/**
	 * Updates the TCL/TK queue values. The format of the queue display string on the
	 * upper right side of the MAC symbol is <tt>q:(<i>s</i>|<i>u</i>)</tt>  where
	 * <tt><i>s</i></tt> is the actual length of the queue for scheduled area messages
	 * and <tt><i>u</i></tt> the actual length of the queue for unscheduled area messages.
	 */
	void updateDisplayString();

	/**
	 * Handles sending in scheduled area. Either we have a frame to send or we have
	 * to send a NULL frame. But both only if it's our turn and we have an error case if the
	 * time isn't fit into NUT (until ULIMIT)!
	 * @param msg is the message object we get
	 */
	void handleScheduled(cMessage* msg);

	/**
	 * Handles sending in unscheduled area. Either we have a frame to send or we have
	 * to send a NULL frame. But both only if it's our turn! If there is no time space to send
	 * we have to wait until the next unscheduled period.
	 * @param msg is the message object we get
	 */
	void handleUnscheduled(cMessage* msg);

	/**
	 * Handles the end of IFG period for the frame has to be send. Messages that are
	 * waiting in the queue can be send after it.
	 */
	void handleEndIFG();
	
	/**
	 * Handles the end of the guard band period. Everything has to be set for entering
	 * the scheduled state.
	 */
	void handleEndGB();
	
	/**
	 * Handles the end of the scheduled period. Everything has to be set for entering
	 * the unscheduled state.
	 */
	void handleEndScheduled();
	
	/**
	 * Handles the end of the unscheduled period. Everything has to be set for entering
	 * the guard band state.
	 */
	void handleEndUnscheduled();
	
	/**
	 * Inits the logging. We need to do it in a special method because there are some values
	 * that aren't initialized through initialize().
	 */
	void initLogging();

public:
	/**
	 * The class declaration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
	 * See OMNeT's user manual.</i>
	 */
	Module_Class_Members(ControlNetMAC, MAC, 0);

	/**
	 * The initialization routine for ControlNet MAC. We only switch initialize the values of member
	 * data.
	 */
	virtual void initialize();

	/**
	 * The ControlNet MAC message handle function. It is distinguished between self-messages
	 * for the timer handling, messages comming from the upper layer and messages
	 * comming form lower layer.Then we decide to proceed handling by calling other helper
	 * members.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * The ControlNet MAC finish member. All statistics are written here.
	 */
	virtual void finish();

	/**
	 * Collects message delay. A receiving node calls direct this member for logging.
	 */
	virtual void collectMessageDelay(simtime_t time);

	/**
	 * Collects transmission time. A receiving node calls direct this member for logging.
	 */
	virtual void collectTransmittingTime(simtime_t time);
};

/**
 * The module type registration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
 * See OMNeT's user manual.</i>
 */
Define_Module(ControlNetMAC);

#endif
