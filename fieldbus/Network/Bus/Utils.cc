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

#include "Utils.h"

/**
 * This is one implementation file of the Bus class.
 */

void Bus::tokenize(const char *str, std::vector<double>& array)
{
	char *str2 = opp_strdup(str);
	if (!str2) return;
	char *s = strtok(str2, " ");
	while (s)
	{
		array.push_back(atof(s));
		s = strtok(NULL, " ");
	}
	delete [] str2;
}

void Bus::setBiterror(cMessage* msg)
{
	double dummy;
	if (par("biterrors").boolValue())
	{
		for (int i = 0; i < msg->length(); i++)
		{
			if (!msg->hasBitError())
			{
				if (par("biterrorRate").doubleValue() == 1.0)
				{
					msg->setBitError(true);
					biterrors++;
				}
				else
					msg->setBitError(false);
			}
			else
				 dummy = par("biterrorRate").doubleValue();
		}
	}
}
