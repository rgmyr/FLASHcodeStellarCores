#include "Data.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>
using std::cout;    using std::endl;
using std::vector;

bool Data::less( uint a, uint b ) 
{
	//use overloaded [] operator to access saddles
	if (compareEqual( (*this)[a],(*this)[b])) return a < b;
	else return compareLess((*this)[a],(*this)[b]);
}

bool Data::greater( uint a, uint b ) 
{
	if (compareEqual((*this)[a],(*this)[b])) return a > b;
	else return compareLess((*this)[b],(*this)[a]);
}

void Data::loadFromVector(const vector<float>& gpot, const vector<float>::size_type num_points) 
{
    cout << "Data::loadFromVector called..." << endl;

    totalSize = num_points;
    data = new DataType[totalSize];

    for(uint i = 0; i < totalSize; ++i)
    {
        data[i] = static_cast<DataType>(gpot[i]);
    }

    maxValue = minValue = data[0];
    for (uint i = 0; i < totalSize; i++) {
        if (data[i] > maxValue) maxValue = data[i];
        if (data[i] < minValue) minValue = data[i];
    }

    size[0] = 512;
    size[1] = 512;
    size[2] = 512;

    cout << "... max value was " << maxValue << " and min value was " << minValue << endl;
}
