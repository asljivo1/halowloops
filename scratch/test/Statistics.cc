#include "Statistics.h"

Statistics::Statistics() {
	this->TimeWhenEverySTAIsAssociated = Time();
	this->EndApplicationTime = Time ();
	TimeMipStarted = Time ();
	TimeMipEnded = Time ();
	NumberOfIneasibleSolutions = 0;
	NumberOfSolutions = 0;
	this->TotalBeaconsSizeInBytes = 0;

}

Statistics::Statistics(int nrOfNodes) : Statistics::Statistics() {
	/*this->TimeWhenEverySTAIsAssociated = Time();
	this->EndApplicationTime = Time ();*/

    for(int i = 0; i < nrOfNodes; i++)
        this->nodeStatistics.push_back(NodeStatistics());    
}
NodeStatistics& Statistics::get(int index) {
    return this->nodeStatistics.at(index);
}

int Statistics::getNumberOfNodes() const {
    return this->nodeStatistics.size();
}

