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

#include "EthernetMACLogging.h"

/**
 * This is one implementation file of the EthernetMACLogging class.
 */

// Some static member prototypes to suppress linking problems
simtime_t EthernetMACLogging::totalMessageTimeDelay;
simtime_t EthernetMACLogging::averageMessageTimeDelay;
simtime_t EthernetMACLogging::transmittingTimeDelay;
simtime_t EthernetMACLogging::retransmittingTimeDelay;
bool EthernetMACLogging::messageTimeDelayFlag;

void EthernetMACLogging::initialize()
{
	totalMessageTimeDelay = 0.0;
	averageMessageTimeDelay = 0.0;
	transmittingTimeDelay = 0.0;
	messageTimeDelayFlag =true;
	Logging::initialize();
}

void EthernetMACLogging::handleMessage(cMessage* msg)
{
	Logging::handleMessage(msg);
}

void EthernetMACLogging::finish()
{
}

void EthernetMACLogging::writeStatistics(cStdDev* statistics)
{
	std::ofstream statisticsFile(par("fileName").stringValue(), std::ios_base::out | std::ios_base::app);
	
	statisticsFile << "# " << endl;
	statisticsFile << "# " << statistics->name() << endl;
	statisticsFile << "# " << endl;
	statisticsFile << "Samples collected:\t" << statistics->samples() << endl;
	statisticsFile << "Sum weights:\t\t" << statistics->weights() << endl;
	statisticsFile << "Total sum:\t\t" << statistics->sum() << endl;
	statisticsFile << "Square sum:\t\t" << statistics->sqrSum() << endl;
	statisticsFile << "Minimum:\t\t" << statistics->min() << endl;
	statisticsFile << "Maximum:\t\t" << statistics->max() << endl;
	statisticsFile << "Mean:\t\t\t" << statistics->mean() << endl;
	statisticsFile << "Standard deviation:\t" << statistics->stddev() << endl;
	statisticsFile << "Variance:\t\t" << statistics->variance() << endl;
	statisticsFile << "\n " << endl;
	statisticsFile.close();

	if (!strncmp(statistics->name(), "'transmitting time statistics:", 30))
	{
		transmittingTimeDelay = transmittingTimeDelay + statistics->sum()
			/ par("receivingHosts").longValue();
		totalMessageTimeDelay = totalMessageTimeDelay + statistics->sum()
			/ par("receivingHosts").longValue();
		averageMessageTimeDelay = averageMessageTimeDelay + statistics->mean()
			/ (statistics->samples() / par("receivingHosts").longValue());
	}

	if (!strncmp(statistics->name(), "'retransmitting time statistics:", 32))
	{
		retransmittingTimeDelay = retransmittingTimeDelay + statistics->sum()
			/ par("receivingHosts").longValue();
	}

	if (!strncmp(statistics->name(), "'blocking time statistics:", 26))
	{
		totalMessageTimeDelay = totalMessageTimeDelay + statistics->sum()
			/ par("receivingHosts").longValue();
		averageMessageTimeDelay = averageMessageTimeDelay + statistics->mean()
			/ par("receivingHosts").longValue();
	}

	if (!strncmp(statistics->name(), "'queue waiting time statistics:", 31))
	{
		totalMessageTimeDelay = totalMessageTimeDelay + statistics->sum()
			/ par("receivingHosts").longValue();
		averageMessageTimeDelay = averageMessageTimeDelay + statistics->mean()
			/ par("receivingHosts").longValue();
	}
}

void EthernetMACLogging::write1Statistics(cStdDev* statistics)
{
	std::ofstream statisticsFile(par("fileName").stringValue(), std::ios_base::out | std::ios_base::app);
	
	statisticsFile << "# " << endl;
	statisticsFile << "# " << statistics->name() << endl;
	statisticsFile << "# " << endl;
	statisticsFile << "Samples collected:\t" << statistics->samples() << endl;
	statisticsFile << "Sum weights:\t\t" << statistics->weights() << endl;
	statisticsFile << "Total sum:\t\t" << statistics->sum() << endl;
	statisticsFile << "Square sum:\t\t" << statistics->sqrSum() << endl;
	statisticsFile << "Minimum:\t\t" << statistics->min() << endl;
	statisticsFile << "Maximum:\t\t" << statistics->max() << endl;
	statisticsFile << "Mean:\t\t\t" << statistics->mean() << endl;
	statisticsFile << "Standard deviation:\t" << statistics->stddev() << endl;
	statisticsFile << "Variance:\t\t" << statistics->variance() << endl;
	statisticsFile << "\n " << endl;
	statisticsFile.close();

	if (!strncmp(statistics->name(), "'transmitting time statistics:", 30))
	{
		transmittingTimeDelay = transmittingTimeDelay + statistics->sum();
		totalMessageTimeDelay = totalMessageTimeDelay + statistics->sum();
		averageMessageTimeDelay = averageMessageTimeDelay + statistics->mean();
	}

	if (!strncmp(statistics->name(), "'retransmitting time statistics:", 32))
	{
		retransmittingTimeDelay = retransmittingTimeDelay + statistics->sum();
	}

	if (!strncmp(statistics->name(), "'blocking time statistics:", 26))
	{
		totalMessageTimeDelay = totalMessageTimeDelay + statistics->sum();
		averageMessageTimeDelay = averageMessageTimeDelay + statistics->mean();
	}

	if (!strncmp(statistics->name(), "'queue waiting time statistics:", 31))
	{
		totalMessageTimeDelay = totalMessageTimeDelay + statistics->sum();
		averageMessageTimeDelay = averageMessageTimeDelay + statistics->mean();
	}
}

void EthernetMACLogging::writeMessageTimeDelay()
{
	std::ofstream statisticsFile(par("fileName").stringValue(), std::ios_base::out | std::ios_base::app);
	static char text[4096];
	static int counter;

	counter++;
	if (counter == par("sendingHosts").longValue())
	{
		sprintf(text,
			"#\n" \
			"# 'message time delay'\n" \
			"#\n" \
			"Total:\t\t\t%f\n" \
			"Average:\t\t%f\n" \
			"\n" \
			"#\n" \
			"# 'network'\n" \
			"#\n" \
			"Efficiency:\t\t%f%c\n",
			totalMessageTimeDelay,
			averageMessageTimeDelay / par("sendingHosts").longValue(),
			transmittingTimeDelay / totalMessageTimeDelay *100,
			'%'
		);
	}
	if (counter == par("receivingHosts").longValue() + 1)
	{
		sprintf(text,
			"%sUtilization:\t\t%f%c\n" \
			"\n",
			text,
			(transmittingTimeDelay + retransmittingTimeDelay) / simTime() *100,
			'%'
		);
		statisticsFile << text;
	}
	statisticsFile.close();
}

EthernetMACLogging::~EthernetMACLogging()
{
	totalMessageTimeDelay = 0.0;
	averageMessageTimeDelay = 0.0;
	transmittingTimeDelay = 0.0;
	retransmittingTimeDelay = 0.0;
	messageTimeDelayFlag = true;
}
