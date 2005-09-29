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

#ifndef BUSLOGGING_H
#define BUSLOGGING_H
#include <omnetpp.h>
#include "Logging.h"
#include "Util.h"

/*
 * This is a definition file for the BusLogging.
 */
 
 /**
  * The bus logging class.
  * 
  * It logs bus specific data.
  */
class BusLogging : public Logging
{
public:
	/**
	 * The class declaration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
	 * See OMNeT's user manual.</i>
	 */
	Module_Class_Members(BusLogging, Logging, 0);

	/**
	 * The initialization routine for BusLogging. Here we initialize all memeber data.
	 */
	virtual void initialize();

	/**
	 * The BusLogging message handle function. Here we handle messages.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * The BusLogging finish member. Here we write statistics.
	 */
	virtual void finish();
};

/**
 * The module type registration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
 * See OMNeT's user manual.</i>
 */
Define_Module(BusLogging);

#endif
