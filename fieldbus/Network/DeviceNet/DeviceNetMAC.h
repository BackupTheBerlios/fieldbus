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
#include "DeviceNetFrame_m.h"
#include "DeviceNetMACLogging.h"

/*
 * This is a definition file for the DeviceNet MAC.
 */

/**
 * Priority compare function. We need it to sort arriving message priorities in the
 * arriving queue.
 * <b>Note:</b> <i>A message arrived with higher priority is always at the top of the
 * queue. Please see the image:
 * <br><img src="../../Documentation/d-normal.png"><br>
 * From the beginning of the arbitration phase until the end of the transmitting phase
 * we can't transmit a frame with higher priority from the queue. But messages
 * arrived until the end of the SOF period are transmitting sorted by their priority.</i>
 */
int compare(cObject* obj1, cObject* obj2);

/**
 * The frame maximum length. This is a bit value.
 */
#define FRAME_MAX	101

/**
 * The frame minimum length. This is a bit value.
 */
#define FRAME_MIN	37

/**
 * The data field maximum length. This is a bit value.
 */
#define DATA_MAX	(8 * 8)

/**
 * The start of frame field length. This is a bit value.
 */
#define SOF	1

/**
 * The arbitration/priority field length. This is a bit value.
 */
#define ARB_BITS	11

/**
 * The arbitration time. This is a time value.
 */
#define ARB ((double) ARB_BITS / TXRATE)

/**
 * The EOF time. This is a time value.
 */
#define EoF ((double) EoF_BITS / TXRATE)

/**
 * The remote transmission request field length. This is a bit value.
 */
#define RTR	1

/**
 * The type field length. This is a bit value.
 */
#define CTRL	6

/**
 * The CRC field length. This is a bit value.
 */
#define CRC	16

/**
 * The ACK field length. This is a bit value.
 */
#define ACK	2

/**
 * The EOF field length. This is a bit value.
 */
#define EoF_BITS	7

/**
 * The EOF time. This is a time value.
 */
#define EoF ((double) EoF_BITS / TXRATE)

/**
 * The error frame/delimiter field length. This is a bit value.
 */
#define ERDEL_BITS	(7 + 8)

/**
 * The EOF time. This is a time value.
 */
#define ERDEL ((double) ERDEL_BITS / TXRATE)

/**
 * The IFS field length. This is a bit value.
 */
#define IFS_BITS	3

/**
 * The IFS time. This is a time value.
 */
#define IFS ((double) IFS_BITS / TXRATE)

/**
 * The self-message kinds. This one is used for the end of SOF period.
 */
#define ENDSOF	21

/**
 * The self-message kinds. This one is used for the end of the arbitration period.
 */
#define ENDARB	22

/**
 * The self-message kinds. This one is used for the end of EOF period.
 */
#define ENDEOF	23

/**
 * The self-message kinds. This one is used for the end of error frame/delimiter period.
 */
#define ENDERDEL	24

/**
 * The self-message kinds. This one is used for the end of IFS period.
 */
#define ENDIFS	25

/**
 * Other DeviceNet dependend definitionen. That one is the transmit rate in bit per sec.
 */
#define TXRATE	(par("bitrate").longValue())

/**
 * The MAC states. This is the definition for the transmit idle state.
 */
#define TX_IDLE_STATE	11

/**
 * The MAC states. This is the definition for the start of frame state.
 */
#define SOF_STATE	12

/**
 * The MAC states. This is the definition for the arbitration state.
 */
#define ARB_STATE	13

/**
 * The MAC states. This is the definition for the transmitting state.
 */
#define TRANSMITTING_STATE	14

/**
 * The MAC states. This is the definition for the retransmission state.
 */
#define RETRANSMISSION_STATE	15

/**
 * The MAC states. This is the definition for the EOF state.
 */
#define EOF_STATE	16

/**
 * The MAC states. This is the definition for the error frame/delimiter state.
 */
#define ERDEL_STATE	17

/**
 * The MAC states. This is the definition for the IFS wait state.
 */
#define WAIT_IFS_STATE	18

/**
 * SOF interval flag. This is for the begin of SOF interval.
 */
#define BEGIN	0

/**
 * SOF interval flag. This is for the end of SOF interval.
 */
#define END	1

/**
 * This is the DeviceNet MAC class.
 * 
 * The CAN 2.0 A standard is implemented here with some restrictions. Overload and
 * error frames are not explicitly modelled. Only an implicit error handling in CRC error
 * cases is avaiable. You also have to take care about the "length vs. speed" parameter.
 * 
 * There are some limitations:<ul>
 * <li>no checking of bitstuffing rule within a transmitting frame, an error is registred
 * only at the end of the CRC field
 * <li>ACK is done implicitly and is calculated into the bit error distribution
 * <li>there is no error counting for two reasons:<ol>
 * <li>defective nodes or a defective bus/connection are not modelled yet
 * <li>only a defective bus/connection case would set the nodes in a passive error state
 * after 127 incorrect messages and bus-off state after 255 incorrect messages because
 * without further modelling extensions we are only able to register "receiving errors",
 * i.e. errors that are registered only by receiving nodes</ol></ul>
 */
class DeviceNetMAC : public MAC
{
private:
	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDSOF
	 */
	cMessage* endSOFMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDARB
	 */
	cMessage* endARBMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDEOF
	 */
	cMessage* endEOFMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDERDEL
	 */
	cMessage* endERDELMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDIFS
	 */
	cMessage* endIFSMsg;

	/**
	 * Sending SOF interval. Every node MAC has to know when the arbitration period begins.
	 * In a "bus idle" state the first node sends the SOF bit. All other send ready nodes (all
	 * that became send ready in the SOF period) can compete in the arbitration process
	 * at the and of this period.
	 */
	static simtime_t intervalSOF[2];

	/**
	 * Capacity of the queue. This parameter must be set in configuration of the simulation.
	 */
    int maxQueueSize;

	/**
	 * Error counter. We have to increment and decrement this counter to decide if we are
	 * in an error active or in an error passive state.
	 */
	 int errorCounter;

	/**
	 * The actual transmit state. It only can be one of the following: @see TX_IDLE_STATE,
	 * @see SOF_STATE, @see ARB_STATE, @see TRANSMITTING_STATE, @see
	 * RETRANSMISSION_STATE, @see EOF_STATE, @see ERDEL_STATE or @see
	 * WAIT_IFS_STATE.
	 */
	int transmitState;
	
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
	 * Back off time statistics. Here we collect all about the back off time.
	 */
	cStdDev* backoffTimeStatistics;
	
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
	 * Retransmitting time statistics. Here we collect all about the retransmission time of  messages.
	 */
	cStdDev* retransmittingTimeStatistics;

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
	 * Queue waiting  time. A temporal variable who stores the waiting time for the actual
	 * message in the queue.
	 */	
	simtime_t queueWaitingTime;

	/**
	 * Transmission time. Time needed to transmit a frame.
	 */	
	simtime_t transmittingTime;

	/**
	 * Retransmission time. Time needed to retransmit a frame.
	 */	
	simtime_t retransmittingTime;

	/**
	 * Message delay time. Traveling time needed for a frame comming from arriving source
	 * upper layer to be passed to destination upper layer.
	 */	
	simtime_t messageDelay;

	/**
	 * Logging member. It is used to log every value needed for statistics.
	 */
	DeviceNetMACLogging* devicenetMACLogging;

	/**
	 * Logging module. It is only needed to instantiate the logging member. @see devicenetMACLogging
	 */
	cModule* devicenetMACLoggingModule;

	/**
	 * Received arbitration collection. Every node receives all arbitration messages sent in
	 * the arbitration period. To decide which which node is allowed to send his frame we
	 * have to compare the arbitration priorities.
	 */
	std::list<cMessage*> collection;

	/**
	 * The ready queue. This queue is 
	 */
	 DeviceNetFrame* sendingCandidate;
	
	/**
	 * Node number. This is a unique identifier for the MAC.
	 */
	int nodeNumber;
	 
	/**
	 * A logging helper. We can only log a retransmitted frame after an error in this
	 * frame and a successful transmitting.
	 */
	bool retransmittedReady;

	/**
	 * A logging helper. It's only for control functionallity.
	 */
	static int loggingCounter;

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
	 * Handles arbitration of the bus. The message priority and arriving time (in a bus idle
	 * case) only affect the arbitration. Every node has to check if his message has won the
	 * competition for the sending right and reschedule if necessery.
	 * @param msg is the message object we get
	 */
	void handleArbitration(cMessage* msg);

	/**
	 * This is the "make data frame" function. Here we put all relevant information in the frame
	 * fields including message encapsulation.
	 * @param msg is the message object we get
	 * @return the encapsulated message
	 */
	cMessage* encapsulateData(cMessage* msg);

	/**
	 * This is the "make remote frame" function. Here we put all relevant information in the frame
	 * fields including message encapsulation.
	 * @param msg is the message object we get
	 * @return the encapsulated message
	 */
	cMessage*  encapsulateRemote(cMessage* msg);

	/**
	 * A helper. For debug option. We can see details about the frame build.
	 * @param msg is the message object we get
	 */
	void frameDetails(cMessage* msg);

	/**
	 * Schedules the end of SOF period for the frame that is to be send. All send ready nodes
	 * can prepare to send their messages after this period of 1 bit time. After this period
	 * stations who receive "to send" messages from upper layer have to wait in the queue
	 * and are transmitted in the next period.
	 */
	void scheduleEndSOF();
	
	/**
	 * Handles the end of SOF period. We can send our arbitration field after it to compete
	 * for the exclusive right to transmit the frame.
	 */
	void handleEndSOF();

	/**
	 * Handles arbitration of the bus. The message priority and arriving time (in a bus idle
	 * case) only affect the arbitration. Every node has to check if his message has won the
	 * competition for the sending right and reschedule if necessery.
	 */
	void handleEndARB();

	/**
	 * Handles frame transmission. After the SOF period and a won bus competition we
	 * transmit our frame.
	 */
	void transmitFrame();

	/**
	 * Handles the end of EOF period. The frame including the acknowledgement is transmitted
	 * and we can pass it now to upper layer if it's not our own frame. Look at the following
	 * image:
	 * <br><img src="../../Documentation/d-normal.png"><br>
	 * You can see the "normal" flow. Every node acknowledges the sended frame and we
	 * enter the EOF period for 7 bit times. After that IFS follows with 3 bits.
	 * @param msg is the message object we get
	 */
	void handleEndEOF(cMessage* msg);

	/**
	 * Handles the end of error frame/delimiter period. This case is only the "CRC error"
	 * case: only receivers see the error! The error handling is done implicitly because we
	 * don't really need to send an error message: the sender has one copy of the frame
	 * where the error flag is set. Look at the following
	 * image:
	 * <br><img src="../../Documentation/d-error.png"><br>
	 * You can see the error case. One MAC is registering the error and don't send the ACK
	 * but produces a one bit time frame rule violation after the ACK. Other nodes send the
	 * error frame (6 bits). Now we enter the delimiter period of 8 bits followed by the IFS
	 * of 3 bit times.
	 */
	void handleEndERDEL();

	/**
	 * Schedules the end of IFS period for the frame has to be send. Messages that are
	 * waiting in the queue can be send after 3 bit times.
	 */
	void scheduleEndIFS();
	
	/**
	 * Handles the end of IFS period. Only internal members are set. We enter a new SOF
	 * period only if we have a frame to transmit. 
	 */
	void handleEndIFS();

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
	Module_Class_Members(DeviceNetMAC, MAC, 0);

	/**
	 * The initialization routine for DeviceNet MAC. We only switch initialize the values of member
	 * data.
	 */
	virtual void initialize();

	/**
	 * The DeviceNet MAC message handle function. It is distinguished between self-messages
	 * for the timer handling, messages comming from the upper layer and messages
	 * comming form lower layer.Then we decide to proceed handling by calling other helper
	 * members.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * The DeviceNet MAC finish member. All statistics are written here.
	 */
	virtual void finish();

	/**
	 * Destructor. It has to be redefined because in the case of rebuilding the network from
	 * the Tcl/Tk interface the static members have to be reset.
	 */
	virtual ~DeviceNetMAC();
};

/**
 * The module type registration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
 * See OMNeT's user manual.</i>
 */
Define_Module(DeviceNetMAC);

#endif
