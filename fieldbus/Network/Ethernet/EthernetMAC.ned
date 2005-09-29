//
// Copyright (C) 2005 Lehrstuhl fuer praktische Informatik, Universitaet Dortmund
// Copyright (C) 2005 Dragan Isakovic
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
// The Ethernet MAC model.
//
// Messages arrives on the in gates: on AppIn the messages from the "application"
// layer, on BusIn the messages from the "bus" layer. Please note the dependencies
// between some of the parameters.
//
simple EthernetMAC
    parameters:
        write1: bool, // Switch to log delays from all or just from one node
        writeScalars: bool,  // Enable/disable recording statistics in *.sca
        debug: bool, // Debug option for detailed output in the trace
        bitrate : numeric, // The bitrate on the bus, in bit/s
        maximumLength: numeric, // Maximum length of the bus media
        propagationSpeed: numeric, // Propagation speed on the bus media
        accuracyFP: numeric, // Floating point accuracy
        maxQueueSize: numeric, // The maximum queue size
        promiscuous: bool, // In promiscuous mode all messages from the lower layer
                           // are send to upper layer
        cd: bool, // Turns collision detection on or off<br><br>note: the following values are for logging/measurement with valid values: -1 = off, 0 = only vector/statistics, >0 = vector/statistics with period >0<br>
        framesSentVector: numeric, // Frames sent vector settings
        framesSentStatistics: numeric, // Frames sent statistics settings
        framesReceivedVector: numeric, // Frames received vector settings
        framesReceivedStatistics: numeric, // Frames received statistics settings
        bitsSentVector: numeric, // Bits sent vector settings
        bitsSentStatistics: numeric, // Bits sent statistics settings
        bitsReceivedVector: numeric, // Bits received vector settings
        bitsReceivedStatistics: numeric, // Bits received statistics settings
        messagesPassedVector: numeric, // Messages passed vector settings
        messagesPassedStatistics: numeric, // Messages passed statistics settings
        messagesReceivedVector: numeric, // Messages received vector settings
        messagesReceivedStatistics: numeric, // Messages received statistics settings
        messagesDroppedBOVector: numeric, // Messages dropped - buffer overflow - vector settings
        messagesDroppedBOStatistics: numeric, // Messages dropped - buffer overflow - statistics settings
        messagesDroppedBEVector: numeric, // Messages dropped - bit error - vector settings
        messagesDroppedBEStatistics: numeric, // Messages dropped - bit error - statistics settings
        messagesDroppedNFUVector: numeric, // Messages dropped - not for us - vector settings
        messagesDroppedNFUStatistics: numeric, // Messages dropped - not for us - statistics settings
        collisionsVector: numeric, // Collisions occured vector settings
        collisionsStatistics: numeric, // Collisions occured statistics settings
        framesRetransmittedVector: numeric, // Frames retransmitted vector settings
        framesRetransmittedStatistics: numeric, // Frames retransmitted statistics settings
        backoffTimeVector: numeric, // Back off time vector settings
        backoffTimeStatistics: numeric, // Back off time statistics settings
        waitingTimeVector: numeric, // Waiting time vector settings
        waitingTimeStatistics: numeric, // Waiting time statistics settings
        blockingTimeVector: numeric, // Blocking time vector setting
        blockingTimeStatistics: numeric, // Blocking time statistics setting
        queueWaitingTimeVector: numeric, // Queue waiting time vector settings
        queueWaitingTimeStatistics: numeric, // Queue waiting time statistics settings
        transmittingTimeVector: numeric, // Transmitting time vector settings
        transmittingTimeStatistics: numeric, // Transmitting time statistics settings
        retransmittingTimeVector: numeric, // Retransmitting time vector settings
        retransmittingTimeStatistics: numeric, // Retransmitting time statistics settings
        messageDelayVector: numeric, // Message delay vector settings
        messageDelayStatistics: numeric, // Message delay statistics settings
        hosts: numeric, // Number of hosts in the simulation
        dividedStatistics: bool; // Shall log timings of every message 
    gates:
        in: UpperLayerIn; // In gate from upper layer
        out: UpperLayerOut; // Out gate to upper layer
        in: LowerLayerIn; // In gate from lower layer
        out: LowerLayerOut; // Out gate to lower layer
endsimple
