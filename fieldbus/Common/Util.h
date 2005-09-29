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

#ifndef UTIL_H
#define UTIL_H
#include <omnetpp.h>

/**
 * This is a definition file with common definitions of variables, macros
 * and functions.
 */

/**
 * Module name definition. It will write the current module object's name.
 */
#define MODULENAME this->name()

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "yes"
 * is the answear if the bus is free (only Ethernet and LonTalk).
 */
#define YES	2

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "no"
 * is the answear if the bus is busy (only Ethernet and LonTalk).
 */
#define NO	-2

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "normal"
 * is the message kind of a frame.
 */
#define NORMAL	3

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "collision"
 * is the message kind in a collision case (only Ethernet and LonTalk).
 */
#define COLLISION	-1

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "busrequest"
 * is the question if the bus is free (only Ethernet and LonTalk).
 */
#define BUSREQUEST	4

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "data"
 * is the message kind of a data frame (only DeviceNet).
 */
#define DATA	3

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "data"
 * is the message kind of a remote frame (only DeviceNet).
 */
#define REMOTE	2

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "arbitration"
 * is send to arbitrate the bus for sending a frame (only DeviceNet).
 */
#define ARBITRATION	4

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "scheduled"
 * is send is send in the scheduled period (only ControlNet).
 */
#define SCHEDULED	0

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "scheduled"
 * is send is send in the unscheduled period (only ControlNet).
 */
#define UNSCHEDULED	1

/**
 * Definitions for messagekinds and colors. It's used for frame-message kinds. "scheduled"
 * is send is send in the scheduled and unscheduled period (only ControlNet).
 */
#define NUL	-1

/**
 * A print function. \deprecated
 * It's used to print time doubles. Use <tt>ev.printf()</tt> instead.
 * @param sim_t is the simulation time to be written
 * @return (SIM_API char*) is a pointer to the output char array 
 */
SIM_API char* printTime(simtime_t sim_t);

#endif
