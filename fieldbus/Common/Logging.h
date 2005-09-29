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

#ifndef LOGGING_H
#define LOGGING_H
#include <omnetpp.h>
#include <list>
#include <vector>
#include <fstream>
#include "../ttl/var/variant.hpp"
#include "Util.h"

/*
 * This is a definition file for the Logging.
 */

/**
 * Variable type definition. A logging value can only be one of this types.
 */
typedef ttl::var::variant<const char*,
	cOutVector*,
	float,
	double,
	int,
	long,
	simtime_t,
	cStdDev*> variable;

 /**
  * The logging base class.
  * 
  * It defines common data members and member functions for all logging classes which
  * schould always be derived from this class. The idea behind this class is to use it in additional
  * to statistics, distribution estimation and recording of simulation results functionality
  * OMNeT++ is offering.
  */
class Logging : public cSimpleModule
{
protected:
	/**
	 * Logging value list. This list contains vector elements (std::vector<variable>) with
	 * the measured value and additional information of the measured value. A vector element
	 * also contains a pointer to a cOutVector object, where we output the measured value.
	 */
	static std::list< std::vector<variable> > vectorList;

	/*
	 * Statistics file name. You have to set it as a parameter of Logging.
	 */
	const char* fileName;

private:
	/**
	 * Debugging flag. It decides is additional debugging information is printet on output.
	 */
	bool debug;

private:
	/**
	 * Scheduled vector handle function. It records value differences to the vector and to
	 * the output.
	 */
	virtual void handleScheduledVector(cMessage* msg);

	/**
	 * Scheduled statistics handle function. It records statistics differences to the vector and to
	 * the output.
	 */
	virtual void handleScheduledStatistics(cMessage* msg);

public:
	/**
	 * The class declaration. <b>Note:</b> <i>This is not a part of the FIELDBUS framework.
	 * See OMNeT's user manual.</i>
	 */
	Module_Class_Members(Logging, cSimpleModule, 0);

	/**
	 * Initialization routine for Logging. Here we initialize all memeber data.
	 */
	virtual void initialize();

	/**
	 * Logging message handle function. Here we handle messages.
	 * @param msg is the message object we get
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * Logging finish member. Here we write statistics.
	 */
	virtual void finish();
	
	/**
	 * Scalar setting function. This member calculates from classes that want to log the
	 * scalar values. It schould be used only in the finish member function of the logging class!
	 * @param vector is the general vector value.
	 */
	template<class VECTOR>
	void calcScalar(VECTOR vector);
	
	/**
	 * Vector initialization function. Here we register every vector and a possible scheduling
	 * period. There are two things you have to watch out:<ol>
	 * <li>You can't use a vector without going through the initialization routine first!
	 * <li>Periodical setting of vector value only works if <i>period</i> is greater than 0!</ol>
	 * @param value is the mesured value.
	 * @param name is the name used to manage the value.
	 * @param description is the description appearing in statistics.
	 * @param period is the scheduling time.
	 */
	template<class TYPE>
	void initVector(const char* name, const char* description, TYPE value, simtime_t period = 0);
	
	/**
	 * Vector setting function. This member sets from classes that want to log the
	 * vector value pairs (value, time). It schould be used in the body of the logging class!
	 * @param value is the general vector value.
	 * @param name is the name used to manage the value.
	 */
	template<class VALUE>
	void setVector(VALUE value, const char* name);
	
	/**
	 * Statistics initialization member. For every vector availible in the list we can calculate
	 * the statistics parameters like mean, variance, standard deviation and so on.
	 * @param name is the name used to manage the value.
	 * @param description is the description appearing in statistics.
	 * @param period is the scheduling time.
	 */
	virtual void initStatistics(const char* name, const char* description, simtime_t period);
	
	/**
	 * Destructor. It has to be redefined because in the case of rebuilding the network from
	 * the Tcl/Tk interface the measurement vector list is to be cleared completly.
	 */
	virtual ~Logging();
};

#endif
