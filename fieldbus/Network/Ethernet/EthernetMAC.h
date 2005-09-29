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

#ifndef ETHERNETMAC_H
#define ETHERNETMAC_H
#include <omnetpp.h>
#include "Util.h"
#include "MAC.h"
#include "EthernetFrame_m.h"
#include "EthernetMACLogging.h"

/*
 * This is a definition file for the Ethernet MAC.
 */

/**
 * The frame maximum length. This is a bit value.
 */
#define FRAME_MAX	(1526 * 8)

/**
 * The frame minimum length. This is a bit value.
 */
#define FRAME_MIN	(72 * 8)

/**
 * The data field maximum length. This is a bit value.
 */
#define DATA_MAX	(1500 * 8)

/**
 * The pad field maximum length. This is a bit value.
 */
#define PAD_MAX	(46 * 8)

/**
 * The preamble field length. This is a bit value.
 */
#define PREAMBLE	(7 * 8)

/**
 * The start of frame field length. This is a bit value.
 */
#define SOF	(1 * 8)

/**
 * The destination field length. This is a bit value.
 */
#define DST_ADR	(6 * 8)

/**
 * The source field length. This is a bit value.
 */
#define SRC_ADR	(6 * 8)

/**
 * The type field length. This is a bit value.
 */
#define TYPE	(2 * 8)

/**
 * The CRC field length. This is a bit value.
 */
#define CRC	(4 * 8)

/**
 * The IFG field length. This is a bit value.
 */
#define IFG_BITS	96

/**
 * The IFG time. This is a time value.
 */
#define IFG ((double) IFG_BITS / TXRATE)

/**
 * The jamming field length. This is a bit value.
 */
#define JAM_BITS	(4 * 8)

/**
 * The jamming time. This is a time value.
 */
#define JAM ((double) JAM_BITS / TXRATE)

/**
 * The MAC states. This is the definition for the transmit idle state.
 */
#define TX_IDLE_STATE	11

/**
 * The MAC states. This is the definition for the IFG wait state.
 */
#define WAIT_IFG_STATE	12

/**
 * The MAC states. This is the definition for the transmitting state.
 */
#define TRANSMITTING_STATE	13

/**
 * The MAC states. This is the definition for the jamming state.
 */
#define JAMMING_STATE	14

/**
 * The MAC states. This is the definition for the backoff state.
 */
#define BACKOFF_STATE	15

/**
 * The MAC states. This is the definition for the retransmission state.
 */
#define RETRANSMISSION_STATE	16

/**
 * The self-message kinds. This one is used for the end of IFG period.
 */
#define ENDIFG	31

/**
 * The self-message kinds. This one is used for the end of back off period.
 */
#define ENDBACKOFF	33

/**
 * The self-message kinds. This one is used for the end of transmission period.
 */
#define ENDTRANSMISSION	34

/**
 * The self-message kinds. This one is used for the end of jamming period.
 */
#define ENDJAMMING	35

/**
 * The self-message kinds. This one is used for the end of retransmission period.
 */
#define RETRANSMISSION	36

/**
 * Other Ethernet dependend definitionen. That one is the maximum length of the bus
 * medium in m.
 */
#define MAX_LENGTH	(par("maximumLength").doubleValue())

/**
 * Other Ethernet dependend definitionen. That one is the propagation speed of the
 * bus medium in m/sec.
 */
#define PSPEED	(par("propagationSpeed").doubleValue())

/**
 * Other Ethernet dependend definitionen. That one is the transmit rate in bit per sec.
 */
#define TXRATE	(par("bitrate").longValue())

/**
 * Other Ethernet dependend definitionen. That one is the slot time, i.e. the time of the collision
 * window in sec.
 */
#define SLOT_TIME	(512.0 / TXRATE)

/**
 * Other Ethernet dependend definitionen. That one is the number of maximum attempts after
 * a collision to retransmit a frame before we give up.
 */
#define MAX_ATTEMPTS	16

/**
 * Other Ethernet dependend definitionen. That one is the range factor for the backoff calculation.
 */
#define BACKOFF_RANGE	10

/**
 * This is the Ethernet MAC class.
 * 
 * This class represents the Ethernet II standard. Only half duplex mode is supported with
 * two possible bus speed: 10 MBit/s and 100 MBit/s. There also is no distinction between
 * LLC and MAC.
 * 
 * Please note that no delay calculation is taking for:<ul>
 * <li>collision occurring until collision handling, a collision is allway handled at the beginning
 * of the "first collided bit"
 * <li>coding, decoding and other calculating times within the MAC is unaccounted for
 * timing logging</ul>
 * An other restriction is that jamming is handled implicitly and collisions with JAM signals
 * are not modelled.
 */
class EthernetMAC : public MAC
{
private:
	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDTRANSMISSION
	 */
	cMessage* endTxMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDIFG
	 */
	cMessage* endIFGMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see RETRANSMISSION
	 */
	cMessage* retransMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDBACKOFF
	 */
	cMessage* endBackoffMsg;

	/**
	 * A self-message pointer. The kind for this self-messages is defined in: @see ENDJAMMING
	 */
	cMessage* endJammingMsg;

	/**
	 * The actual transmit state. It only can be one of the following: @see TX_IDLE_STATE,
	 * @see WAIT_IFG_STATE, @see TRANSMITTING_STATE, @see JAMMING_STATE,
	 * @see BACKOFF_STATE or @see RETRANSMISSION_STATE.
	 */
	int transmitState;
	
	/**
	 * The capacity of the queue. This parameter must be set in configuration of the simulation.
	 */
    int maxQueueSize;
    
    /**
     * The setting of the promiscuous mode. This parameter must be set in configuration of
     * the simulation.
     */
	bool isPromiscuous;
	
	/**
	 * A helper value. It indicates if a frame is for us in non-promiscuous mode
	 */
	bool forUs;
	
	/**
	 * Back off counter. Controls the frame retransmission.
	 */
	int backoffs;
	
	/**
	 * Collision detection switch. If we want to turn CD off we can do it. A crashed sent frame
	 * won't be retransmitted again.<br>
	 * <b>Note:</b> <i>Logging of time parameters like "message delay" only will work
	 * correctly without this switch being turned off!</i>
	 */
	bool cd;

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
	 * Messages dropped. Counter for dropped messages from lower layer because they
	 * was not for us.<br>
	 * <b>Note:</b> <i>This value is only set in the case of a non-promuscous mode.</i>
	 */
	long messagesDroppedNFU;

	/**
	 * Collisions occured. Counter for occured collisions with our sent frames.
	 */
	long collisions;

	/**
	 * Cumulated back of time. Whole time in back of procedures of all collided frames.
	 */
	simtime_t backoffTime;

	/**
	 * Frames retransmitted. Counter for retransmitted frames in collision cases.
	 */
	long framesRetransmitted;

	/**
	 * Frame retransmission failures. Counts the retransmission failures, viz >16 attepts
	 * without success.
	 */
	long frameRetransmissionFailure;

	/**
	 * Logging member. It is used to log every value needed for statistics.
	 */
	EthernetMACLogging* ethernetMACLogging;

	/**
	 * Logging module. It is only needed to instantiate the logging member. @see ethernetMACLogging
	 */
	cModule* ethernetMACLoggingModule;
	
	/**
	 * MAC adress. This is a unique identifier for the MAC.
	 */
	int MACAddress;
	 
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
	 * Queue wiating  time. A temporal variable who stores the waiting time for the actual
	 * message in the queue.
	 */	
	simtime_t queueWaitingTime;

	/**
	 * A logging helper. We can only log a retransmitted frame after a collision with this
	 * frame and a successful transmitting.
	 */
	bool retransmittedReady;

private:
	/**
	 * Here we handle messages comming from upper layer. The message is encapsulate
	 * and then put in the queue. @see encapsulateMessage(cMessage* msg)
	 * @param msg is the message object we get
	 */
	virtual void handleUpperLayerMessage(cMessage* msg);
	
	/**
	 * A frame comming from lower layer is handled here. The task is to check if the message
	 * is for us (only in non-promiscuous mode) and if the message is did a collision.
	 * @param msg is the message object we get
	 */
	virtual void handleLowerLayerMessage(cMessage* msg);

	/**
	 * This is the "make frame" function. Here we put all relevant information in the frame
	 * fields including message encapsulation.
	 * @param msg is the message object we get
	 * @return the encapsulated message
	 */
	cMessage* encapsulateMessage(cMessage* msg);

	/**
	 * A helper. For debug option. We can see details about the frame build.
	 * @param msg is the message object we get
	 */
	void frameDetails(cMessage* msg);

	/**
	 * Schedules the end of IFG period for the frame has to be send. This message has been
	 * waiting in the queue and after 96 bit times it can be send.
	 */
	void scheduleEndIFG();
	
	/**
	 * Handles the end of IFG period. If the bus is free we can send our frame.
	 */
	void handleEndIFG();

	/**
	 * Bus request. This is an abstraction of the "listen to the bus" ability.
	 * @param msg is the message object we get
	 */
	void handleBusRequest(cMessage* msg);

	/**
	 * Handles frame transmission. The Bus if free, we send the frame.
	 */
	void transmitFrame();

	/**
	 * Schedules the end of transmission. The end depends on message length and the
	 * actual bit rate. This event is to be unscheduled if a collision occur.
	 * @param msg is the message object we get
	 */
	void scheduleEndTx(cMessage* msg);

	/**
	 * Schedules the end of retransmission. Because in our model we can't really listen to
	 * the bus all the time, we schedule the time point when the bus is idle and this depends
	 * on the actual frame going through this tap.
	 * @param msg is the message object we get
	 */
	void scheduleRetransmission(cMessage* msg);

	/**
	 * Handles the end of transmission. The message can now be removed from the queue
	 * and other messages in the queue can be send.
	 */
	void handleEndTx();
	
	/**
	 * Handles the retransmission. Trying to send a frame again.
	 */
	void handleRetransmission();

	/**
	 * Handles a collision. There are two cases:<ol>
	 * <li>the collision occurred while we are transmitting
	 * <li>we scheduled the last frame because the bus was used</ol>
	 * The 1st case has to be dealed with "back off" and the 2nd case is good for us
	 * because now we can try to send our "delayed" frame again.
	 * <br><b>Note:</b> <i>If collision occur while we don't do anything will not be
	 * handled.</i>
	 */
	void handleCollision();

	/**
	 * Schedules the end of the jamming and the beginning of the back off process. There
	 * is no need to send a JAM signal because every device on the bus is informed "unmistakeably"
	 * about the collision.
	 */
	void scheduleJam_Backoff();

	/**
	 * Handles the back off process. A retransmission is to do in \f$BEB\f$ time. The binary
	 * exponential backoff is:<p>\f$BEB=U(0,r)*SLOT\_TIME\f$ where \f$0\leq{r}<2^k\f$
	 * , \f$k=min(n,10)\f$, \f$n=\#collisions\f$.</p>
	 */
	void handleBackoff();

	/**
	 * Handles the end of back off process. We are ready to send frames again.
	 */
	void handleEndBackoff();
	
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
	Module_Class_Members(EthernetMAC, MAC, 0);

	/**
	 * The initialization routine for Ethernet MAC. We only switch initialize the values of member
	 * data.
	 */
	virtual void initialize();

	/**
	 * The Ethernet MAC message handle function. It is distinguished between self-messages
	 * for the timer handling, messages comming from the upper layer and messages
	 * comming form lower layer.Then we decide to proceed handling by calling other helper
	 * members.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * The Ethernet MAC finish member. All statistics are written here.
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
Define_Module(EthernetMAC);

#endif
