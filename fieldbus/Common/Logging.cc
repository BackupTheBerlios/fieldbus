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

#include "Logging.h"

/**
 * This is only the implementation the Logging class.
 */

// Some member prototypes for suppressing linking problems
template void Logging::calcScalar<float>(float vector);
template void Logging::calcScalar<double>(double vector);
template void Logging::calcScalar<int>(int vector);
template void Logging::calcScalar<long>(long vector);
template void Logging::setVector<float>(float value, const char* name);
template void Logging::setVector<double>(double value, const char* name);
template void Logging::setVector<int>(int value, const char* name);
template void Logging::setVector<long>(long value, const char* name);
template void Logging::initVector<float>(const char* name, const char* description, float value, simtime_t period);
template void Logging::initVector<double>(const char* name, const char* description, double value, simtime_t period);
template void Logging::initVector<int>(const char* name, const char* description, int value, simtime_t period);
template void Logging::initVector<long>(const char* name, const char* description, long value, simtime_t period);
std::list< std::vector<variable> > Logging::vectorList;

void Logging::initialize()
{
	debug = par("debug").boolValue();
}

void Logging::handleMessage(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleMessage entered\n";

	ev << MODULENAME << ": Scheduled Logging now.\n";

	char* name = opp_strdup(msg->name());
	char* kind = strtok (name, "_");
	
	// do we have to handle vector or statistics?
	if (!strcmp(kind, "ScheduleVector"))
	{
		delete name;
		handleScheduledVector(msg);
	}
	else
	{
		delete name;
		handleScheduledStatistics(msg);
	}

}

void Logging::finish()
{
	if (debug) ev << MODULENAME << ": finish entered\n";
	
	// write statistics if needed
	std::list< std::vector<variable> >::iterator listIterator;
	std::ofstream statisticsFile(par("fileName").stringValue(), std::ios_base::out | std::ios_base::app);

//	// testing:
//	for (listIterator = vectorList.begin(); listIterator != vectorList.end(); listIterator++)
//	{
//		std::vector<variable> vec(*listIterator);
//		variable value = vec[1];
//		if (value.which() == 0)
//			ev << "0 : " << ttl::var::get<const char*> (value) << endl;
//		else
//			ev << "another value!!!\n";
//	}
//
	for (listIterator = vectorList.begin(); listIterator != vectorList.end(); listIterator++)
	{
		std::vector<variable> vec(*listIterator);
		// 1. case: vector and statistics, 2. case: only statistics
		if (vec.size() - 1 == 11)
		{
			variable outStdDev = vec[8];
			if (typeid(ttl::var::get<cStdDev*>(outStdDev)) != typeid(cStdDev*))
				error("Logging::finish: Something is wrong with the pointer to cStdDev");
			cStdDev* outStat = ttl::var::get<cStdDev*>(outStdDev);
			statisticsFile << "# " << endl;
			statisticsFile << "# " << outStat->name() << endl;
			statisticsFile << "# " << endl;
			statisticsFile << "Samples collected:\t" << outStat->samples() << endl;
			statisticsFile << "Sum weights:\t\t" << outStat->weights() << endl;
			statisticsFile << "Total sum:\t\t" << outStat->sum() << endl;
			statisticsFile << "Square sum:\t\t" << outStat->sqrSum() << endl;
			statisticsFile << "Minimum:\t\t" << outStat->min() << endl;
			statisticsFile << "Maximum:\t\t" << outStat->max() << endl;
			statisticsFile << "Mean:\t\t\t" << outStat->mean() << endl;
			statisticsFile << "Standard deviation:\t" << outStat->stddev() << endl;
			statisticsFile << "Variance:\t\t" << outStat->variance() << endl;
			statisticsFile << "\n " << endl;
		}
		else if (vec.size() - 1 == 8)
		{
			variable outStdDev = vec[5];
			if (typeid(ttl::var::get<cStdDev*>(outStdDev)) != typeid(cStdDev*))
				error("Logging::finish: Something is wrong with the pointer to cStdDev");
			cStdDev* outStat = ttl::var::get<cStdDev*>(outStdDev);
			statisticsFile << "# " << endl;
			statisticsFile << "# " << outStat->name() << endl;
			statisticsFile << "# " << endl;
			statisticsFile << "Samples collected:\t" << outStat->samples() << endl;
			statisticsFile << "Sum weights:\t\t" << outStat->weights() << endl;
			statisticsFile << "Total sum:\t\t" << outStat->sum() << endl;
			statisticsFile << "Square sum:\t\t" << outStat->sqrSum() << endl;
			statisticsFile << "Minimum:\t\t" << outStat->min() << endl;
			statisticsFile << "Maximum:\t\t" << outStat->max() << endl;
			statisticsFile << "Mean:\t\t\t" << outStat->mean() << endl;
			statisticsFile << "Standard deviation:\t" << outStat->stddev() << endl;
			statisticsFile << "Variance:\t\t" << outStat->variance() << endl;
			statisticsFile << "\n " << endl;
		}
	}
	statisticsFile.close();
}

template<class VECTOR>
void Logging::calcScalar(VECTOR vector)
{
	if (debug) ev << MODULENAME << ": calcScalar entered\n";

//	std::list< std::vector<variable> >::iterator listIterator;
//	// testing:
//	for (listIterator = vectorList.begin(); listIterator != vectorList.end(); listIterator++)
//	{
//		std::vector<variable> vec(*listIterator);
//		for (int i = 1; i <= 5; i++)
//		{
//			variable value = vec[i];
//			switch (value.which())
//			//const char* = 0, cOutVector = 1, float = 2, double = 3, int = 4, long = 5
//			{
//				case 0:
//					ev << "0 : " << ttl::var::get<const char*> (value) << endl;
//					break;
//				case 2:
//					ev << "2 : " << ttl::var::get<float> (value) << endl;
//					break;
//				case 3:
//					ev << "3 : " << ttl::var::get<double> (value) << endl;
//					break;
//				case 4:
//					ev << "4 : " << ttl::var::get<int> (value) << endl;
//					break;
//				case 5:
//					ev << "5 : " << ttl::var::get<long> (value) << endl;
//					break;
//			}
//		}
//	}
}

template<class TYPE>
void Logging::initVector(const char* name, const char* description, TYPE value, simtime_t period)
{
	if (debug) ev << MODULENAME << ": initVector entered\n";

	// schedule first measuring event
	char vectorName[14 + strlen(name) + 1];

	sprintf(vectorName, "ScheduleVector_%s", name);
	if (period > 0)
	{
		cMessage* scheduleVector = new cMessage(vectorName, 0);
		take(scheduleVector);
		scheduleAt(simTime() + period, scheduleVector);
		ev << MODULENAME << ": Event \"" << scheduleVector->name() << " scheduled at t = "
			<< printTime(simTime() + period) << endl;
	}

	std::vector<variable> vector(1);
	vector.push_back(name);
	vector.push_back(description);
	vector.push_back(value);
	vector.push_back(new cOutVector(description));
	if (period > 0)
	{
		char vectorDescription[strlen(name) + 12 + strlen(description) + 1];
		sprintf(vectorDescription, "%s (period = %fs)", description, period);
		vector.push_back(new cOutVector(vectorDescription));
		vector.push_back(value);
		vector.push_back(period);
	}
	vectorList.push_back(vector);
}

template<class VALUE>
void Logging::setVector(VALUE value, const char* name)
{
	if (debug) ev << MODULENAME << ": setVector entered\n";

	std::list< std::vector<variable> >::iterator listIterator;

	listIterator = vectorList.begin();
	int valueCounter = 0;
	// search the list for the right entry
	do
	{
		if (listIterator == vectorList.end() && valueCounter == 0)
			error("Logging::setVector: No value with name \"%s\" in the measuring list", name);
		else if (listIterator == vectorList.end() && valueCounter > 0)
			break;

		std::vector<variable> org(*listIterator);
		
		variable vectorName = org[1];		
		if (vectorName.which() != 0)
			error("Logging::setVector: Init of the measuring value \"%s\" is incorrect", name);

		// if value name found refresh and write output
		if (strcmp(ttl::var::get<const char*> (vectorName), name) == 0)
		{
			if (valueCounter > 0)
				error("Logging::setVector: Two equal value names found: \"%s\". Define a measuring name once", name);
			variable outVec = org[4];
			if (typeid(ttl::var::get<cOutVector*>(outVec)) != typeid(cOutVector*))
				error("Logging::setVector: Something is wrong with the pointer to cOutVector");
			cOutVector* outVector;
			try
			{
				outVector = ttl::var::get<cOutVector*>(outVec);
			}
			catch (ttl::var::exception)
			{
				error("Logging::setVector: can't get outVec");
			}
			outVector->record(value);
			std::vector<variable> temp(1);
			// rescue original
			temp.push_back(org[1]);
			temp.push_back(org[2]);
			temp.push_back(value);
			temp.push_back(org[4]);
			// we have a periodical vector measurement
			if (org.size() - 1 == 7)
			{
				temp.push_back(org[5]);
				temp.push_back(org[6]);
				temp.push_back(org[7]);
			}
			// we have a periodical statistics measurement
			if (org.size() - 1 == 8)
			{
				temp.push_back(org[5]);
				temp.push_back(org[6]);
				temp.push_back(org[7]);
				temp.push_back(org[8]);
			}
			if (org.size() - 1 == 11)
			{
				temp.push_back(org[5]);
				temp.push_back(org[6]);
				temp.push_back(org[7]);
				temp.push_back(org[8]);
				temp.push_back(org[9]);
				temp.push_back(org[10]);
				temp.push_back(org[11]);
			}
			vectorList.erase(listIterator);
			vectorList.push_front(temp);
			valueCounter++;
		}
	
		listIterator++;
	}
	while (true);
}

void Logging::initStatistics(const char* name, const char* description, simtime_t period)
{
	if (debug) ev << MODULENAME << ": initStatistics entered\n";

	std::list< std::vector<variable> >::iterator listIterator;

	listIterator = vectorList.begin();
	int valueCounter = 0;
	// search the list for the right entry
	do
	{
		if (listIterator == vectorList.end() && valueCounter == 0)
			error("Logging::initStatistics: No value with name \"%s\" in the measuring list", name);
		else if (listIterator == vectorList.end() && valueCounter > 0)
			break;

		std::vector<variable> org(*listIterator);
		
		variable vectorName = org[1];		
		if (vectorName.which() != 0)
			error("Logging::initStatistics: Init of the measuring value \"%s\" is incorrect", name);

		// if value name found add statistics object
		if (strcmp(ttl::var::get<const char*> (vectorName), name) == 0)
		{
			if (valueCounter > 0)
				error("Logging::initStatistics: Two equal value names found: \"%s\". Define a measuring name once", name);
			std::vector<variable> temp(1);
			// rescue original
			temp.push_back(org[1]);
			temp.push_back(org[2]);
			temp.push_back(org[3]);
			temp.push_back(org[4]);
			if (org.size() - 1 > 4)
			{
				temp.push_back(org[5]);
				temp.push_back(org[6]);
				temp.push_back(org[7]);
			}
			// add statistics
			temp.push_back(new cStdDev(description));
			temp.push_back(org[3]);
			temp.push_back(period);
			temp.push_back(description);
			vectorList.erase(listIterator);
			vectorList.push_front(temp);
			valueCounter++;
		}
	
		listIterator++;
	}
	while (true);

	// schedule first measuring event
	char statisticsName[18 + strlen(name) + 1];

	if (period > 0)
	{
		sprintf(statisticsName, "ScheduleStatistics_%s", name);
		cMessage* scheduleStatistics = new cMessage(statisticsName, 0);
		take(scheduleStatistics);
		scheduleAt(simTime() + period, scheduleStatistics);
		ev << MODULENAME << ": Event \"" << scheduleStatistics->name() << " scheduled at t = "
			<< printTime(simTime() + period) << endl;
	}
}

Logging::~Logging()
{
	// clean the list in "rebuild network" case
	vectorList.clear();
}

void Logging::handleScheduledVector(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleScheduledVector entered\n";

	char* name;
	char* position = strrchr(msg->name(), '_');
	position++;
	name = new char[strlen(position) + 1];
	sprintf(name, "%s", position);
	
	std::list< std::vector<variable> >::iterator listIterator;
	variable period;

	listIterator = vectorList.begin();
	int valueCounter = 0;
	// search the list for the right entry
	do
	{
		if (listIterator == vectorList.end() && valueCounter == 0)
			error("Logging::handleScheduledVector: No value with name \"%s\" in the measuring list", name);
		else if (listIterator == vectorList.end() && valueCounter > 0)
			break;

		std::vector<variable> org(*listIterator);
		
		variable vectorName = org[1];
		if (vectorName.which() != 0)
			error("Logging::handleScheduledVector: Init of the measuring value \"%s\" is incorrect", name);

		// if value name found refresh and write output
		if (strcmp(ttl::var::get<const char*> (vectorName), name) == 0)
		{
			if (valueCounter > 0)
				error("Logging::handleScheduledVector: Two equal value names found: \"%s\". Define a measuring name once", name);
			variable outVec = org[5];
			if (typeid(ttl::var::get<cOutVector*>(outVec)) != typeid(cOutVector*))
				error("Logging::handleScheduledVector: Something is wrong with the pointer to cOutVector");
			cOutVector* outVector;
			try
			{
				outVector = ttl::var::get<cOutVector*>(outVec);
			}
			catch (ttl::var::exception)
			{
				error("Logging::handleScheduledVector: can't get outVec");
			}
			variable previousValue = org[6];
			variable actualValue = org[3];
			std::vector<variable> temp(1);
			// rescue original
			temp.push_back(org[1]);
			temp.push_back(org[2]);
			temp.push_back(org[3]);
			temp.push_back(org[4]);
			temp.push_back(org[5]);
			switch (previousValue.which())
			//const char* = 0, cOutVector = 1, float = 2, double = 3, int = 4, long = 5
			{
				case 2:
					if (ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue) >= 0)
						outVector->record(ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue));
					else
						outVector->record(-1 * (ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue)));
					break;
				case 3:
					if (ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue) >= 0)
						outVector->record(ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue));
					else
						outVector->record(-1 * (ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue)));
					break;
				case 4:
					if (ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue) >= 0)
						outVector->record(ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue));
					else
						outVector->record(-1 * (ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue)));
					break;
				case 5:
					if (ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue) >= 0)
						outVector->record(ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue));
					else
						outVector->record(-1 * (ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue)));
					break;
				default:
					error("Logging::handleScheduledVector: No type identified for value of periodical writing output");
			}
			temp.push_back(org[3]);
			temp.push_back(org[7]);
			period = org[7];
			// statistics measurement?
			if (org.size() - 1 > 7)
			{
				temp.push_back(org[8]);
				temp.push_back(org[9]);
				temp.push_back(org[10]);
				temp.push_back(org[11]);
			}
			vectorList.erase(listIterator);
			vectorList.push_front(temp);
			valueCounter++;
		}
	
		listIterator++;
	}
	while (true);
	
	// scheduling for the next time
	if (period.which() <= 2 || period.which() >= 5)
		error("Logging::handleScheduledVector: No type identified for timing of periodical writing output");
	take(msg);
	scheduleAt(simTime() + ttl::var::get<simtime_t>(period), msg);
	ev << MODULENAME << ": Event \"" << msg->name() << "\" scheduled at t = "
		<< printTime(simTime() + ttl::var::get<simtime_t>(period)) << endl;
	ev << "\n";
}

void Logging::handleScheduledStatistics(cMessage* msg)
{
	if (debug) ev << MODULENAME << ": handleScheduledStatistics entered\n";

	char* name;
	char* position = strrchr(msg->name(), '_');
	position++;
	name = new char[strlen(position) + 1];
	sprintf(name, "%s", position);
	
	std::list< std::vector<variable> >::iterator listIterator;
	variable period;

	listIterator = vectorList.begin();
	int valueCounter = 0;
	// search the list for the right entry
	do
	{
		if (listIterator == vectorList.end() && valueCounter == 0)
			error("Logging::handleScheduledStatistics: No value with name \"%s\" in the measuring list", name);
		else if (listIterator == vectorList.end() && valueCounter > 0)
			break;

		std::vector<variable> org(*listIterator);
		
		variable vectorName = org[1];
		if (vectorName.which() != 0)
			error("Logging::handleScheduledStatistics: Init of the measuring value \"%s\" is incorrect", name);

		// if value name found refresh and write output
		if (strcmp(ttl::var::get<const char*> (vectorName), name) == 0)
		{
			if (valueCounter > 0)
				error("Logging::handleScheduledStatistics: Two equal value names found: \"%s\". Define a measuring name once", name);
			// do we have statistics and/or/only vector?
			if (org.size() - 1 > 7 && org.size() - 1 == 11)
			{
				variable outStdDev = org[8];
				if (typeid(ttl::var::get<cStdDev*>(outStdDev)) != typeid(cStdDev*))
					error("Logging::handleScheduledStatistics: Something is wrong with the pointer to cStdDev");
				cStdDev* outStat;
				try
				{
					outStat = ttl::var::get<cStdDev*>(outStdDev);
				}
				catch (ttl::var::exception)
				{
					error("Logging::handleScheduledStatistics: can't get outStdDev");
				}
				variable previousValue = org[9];
				variable actualValue = org[3];
				period = org[10];
				std::vector<variable> temp(1);
				// rescue original
				temp.push_back(org[1]);
				temp.push_back(org[2]);
				temp.push_back(org[3]);
				temp.push_back(org[4]);
				temp.push_back(org[5]);
				switch (previousValue.which())
				//const char* = 0, cOutVector = 1, float = 2, double = 3, int = 4, long = 5
				{
					case 2:
						if (ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue) >= 0)
							outStat->collect(ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue)));
						break;
					case 3:
						if (ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue) >= 0)
							outStat->collect(ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue)));
						break;
					case 4:
						if (ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue) >= 0)
							outStat->collect(ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue)));
						break;
					case 5:
						if (ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue) >= 0)
							outStat->collect(ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue)));
						break;
					default:
						error("Logging::handleScheduledStatistics: No type identified for value of periodical writing output");
				}
				temp.push_back(org[6]);
				temp.push_back(org[7]);
				temp.push_back(org[8]);
				temp.push_back(org[3]);
				temp.push_back(org[10]);
				temp.push_back(org[11]);
				vectorList.erase(listIterator);
				vectorList.push_front(temp);
			}
			else
			{
				variable outStdDev = org[5];
				if (typeid(ttl::var::get<cStdDev*>(outStdDev)) != typeid(cStdDev*))
					error("Logging::handleScheduledStatistics: Something is wrong with the pointer to cStdDev");
				cStdDev* outStat;
				try
				{
					outStat = ttl::var::get<cStdDev*>(outStdDev);
				}
				catch (ttl::var::exception)
				{
					error("Logging::handleScheduledStatistics: can't get outStdDev");
				}
				variable previousValue = org[6];
				variable actualValue = org[3];
				period = org[7];
				std::vector<variable> temp(1);
				temp.push_back(org[1]);
				temp.push_back(org[2]);
				temp.push_back(org[3]);
				temp.push_back(org[4]);
				switch (previousValue.which())
				//const char* = 0, cOutVector = 1, float = 2, double = 3, int = 4, long = 5
				{
					case 2:
						if (ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue) >= 0)
							outStat->collect(ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<float>(actualValue) - ttl::var::get<float>(previousValue)));
						break;
					case 3:
						if (ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue) >= 0)
							outStat->collect(ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<double>(actualValue) - ttl::var::get<double>(previousValue)));
						break;
					case 4:
						if (ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue) >= 0)
							outStat->collect(ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<int>(actualValue) - ttl::var::get<int>(previousValue)));
						break;
					case 5:
						if (ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue) >= 0)
							outStat->collect(ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue));
						else
							outStat->collect(-1 * (ttl::var::get<long>(actualValue) - ttl::var::get<long>(previousValue)));
						break;
					default:
						error("Logging::handleScheduledStatistics: No type identified for value of periodical writing output");
				}
				temp.push_back(org[5]);
				temp.push_back(org[3]);
				temp.push_back(org[7]);
				temp.push_back(org[8]);
				vectorList.erase(listIterator);
				vectorList.push_front(temp);
			}
			valueCounter++;
		}
	
		listIterator++;
	}
	while (true);
	
	// scheduling for the next time
	if (period.which() <= 2 || period.which() >= 5)
		error("Logging::handleScheduledStatistics: No type identified for timing of periodical writing output");
	take(msg);
	scheduleAt(simTime() + ttl::var::get<simtime_t>(period), msg);
	ev << MODULENAME << ": Event \"" << msg->name() << "\" scheduled at t = "
		<< printTime(simTime() + ttl::var::get<simtime_t>(period)) << endl;
	ev << "\n";
}
