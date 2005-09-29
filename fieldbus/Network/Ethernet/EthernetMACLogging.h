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

#ifndef ETHERNETMACLOGGING_H
#define ETHERNETMACLOGGING_H
#include <omnetpp.h>
#include "Logging.h"
#include "Util.h"

/*
 * This is a definition file for the EthernetMACLogging.
 */
 
 /**
  * The Ethernet logging class.
  * 
  * It logs protocol specific data.
  */
class EthernetMACLogging : public Logging
{
private:
	/**
	 * Total time delay of messages. For message time delay calculation it's only needful to add
	 * values from all nodes.
	 */
	static simtime_t totalMessageTimeDelay;

	/**
	 * Average time delay of messages. For message time delay calculation it's only needful to add
	 * values from all nodes.
	 */
	static simtime_t averageMessageTimeDelay;

	/**
	 * Transmitting time delay of messages. For message time delay calculation it's only needful to add
	 * values from all nodes.
	 */
	static simtime_t transmittingTimeDelay;

	/**
	 * Retransmitting time delay of messages. For message time delay calculation it's only needful to add
	 * values from all nodes.
	 */
	static simtime_t retransmittingTimeDelay;

	/**
	 * Message time delay flag. This flag is needed to show the value only once.
	 */
	static bool messageTimeDelayFlag;

public:
	/**
	 * The class declaration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
	 * See OMNeT's user manual.</i>
	 */
	Module_Class_Members(EthernetMACLogging, Logging, 0);

	/**
	 * Initialization routine for EthernetMACLogging. Here we initialize all memeber data.
	 */
	virtual void initialize();

	/**
	 * EthernetMACLogging message handle function. Here we handle messages.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * EthernetMACLogging finish member. Here we write statistics.
	 */
	virtual void finish();
	
	/**
	 * Writes statistics. Please see the paper first:<p>
	 * B.F. Lian, J. R. Moyne, D. M. Tilbury: <i>Performance Evaluation of Control Networks:
	 * Ethernet, ControlNet, and DeviceNet.</i> IEEE Control Systems Magazine, Feb. 2001, 
	 * 66-83. (<a href="http://www.eecs.umich.edu/~impact/Publications/LiMoTi0102.pdf">
	 * LiMoTi0102.pdf</a>)<p>
	 * "total time delay" and the "average time delay" are devided in a case where propagation
	 * and posion of the nodes on bus is relevant and the other case where we have a
	 * constant propagation between nodes. Notwithstanding the description in the paper
	 * the calculated values for the first case is:
	 * \f{align*}
	 * T_{delay}^{sum}&=\frac{1}{N-1}\sum_{k=1}^{N-1}\sum_{i=1}^{N}
	 * \sum_{j=1}^{m(i)}T_{delay}^{(i,j)} \\
	 * T_{delay}^{avg}&=\frac{1}{N-1}\sum_{k=1}^{N-1}\left(\frac{1}{m(i)}
	 * \sum_{i=1}^{N}\sum_{j=1}^{m(i)}T_{delay}^{(i,j)}\right)
	 * \f}
	 * The second case is:
	 * \f{align*}
	 * T_{delay}^{sum}&=\sum_{i=1}^{N}\sum_{j=1}^{m(i)}T_{delay}^{(i,j)} \\
	 * T_{delay}^{avg}&=\sum_{i=1}^{N}\left(\frac{1}{m(i)}
	 * \sum_{j=1}^{m(i)}T_{delay}^{(i,j)}\right)
	 * \f}
	 * Common for all equations is:
	 * \f{align*}
	 * N&={Number\ of\ nodes} \\
	 * m(i)&={Number\ of\ sent\ messages\ from\ node\ i} \\
	 * T_{delay}^{(i,j)}&=T_{queue}^{(i,j)}+T_{block}^{(i,j)}+T_{tx}^{(i,j)} \\
	 * m(i)&=M(i)
	 * \f}
	 * What's different:<ul>
	 * <li>\f$m(i)\f$ is a more precisely value for \f$M(i)\f$ because \f$M(i)\f$ is oriented
	 * to periodical messages, we want to have a more common case which depends on
	 * messages arriving policies. Such a policy can be a periodical arriving.
	 * <li>The first case...</ul>
	 * What's equal:<ul>
	 * <li>Needless to say the second case</ul>
	 * @param statistics is the data structure that has collected the informations.
	 */
	virtual void writeStatistics(cStdDev* statistics);

	virtual void write1Statistics(cStdDev* statistics);

	/**
	 * Writes message time delay. It writes calculated values to the statistics file.
	 */
	virtual void writeMessageTimeDelay();

	/**
	 * Destructor. It has to be redefined because in the case of rebuilding the network from
	 * the Tcl/Tk interface the static members have to be reset.
	 */
	~EthernetMACLogging();
};

/**
 * The module type registration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
 * See OMNeT's user manual.</i>
 */
Define_Module(EthernetMACLogging);

#endif
