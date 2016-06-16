#include "trace.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <ctime>

Trace::Trace(const char *bytes)
{
	int ins = 4; 	// Size of int stored if the raw binary string
	int cur = 0; // Cursor walking in the string and getting data
	cur += 12;
	
	int whs;
	memcpy(&whs, bytes+cur, ins); // Storing size of the waveform header
	cur += whs; // Skipping the waveform header
	
	int dhs;
	memcpy(&dhs, bytes+cur, ins); // Storing size of the data header
	cur += dhs; // Skipping the data header
	
	int bfs;
	memcpy(&bfs, bytes+cur-ins, ins); // Storing the data size
	
	length = bfs/ins;
	
	v = new float[length];
	memcpy(v, bytes+cur, bfs);
	
	nTraces = 1;
}

Trace::Trace(int _length) 
{
	length = _length;
	v = new float[length];
	nTraces=1;
}

Trace::Trace(const Trace * t) 
{
	length = t->length;
	nTraces = t->nTraces;
	v = new float[length];
	for (int i=0; i<length; i++)
		v[i] = t->v[i];
}

Trace & Trace::operator+=(const Trace& t) 
{
	for (int i=0; i<length; i++)
		v[i] += t.v[i];
	nTraces+=t.nTraces;
	
	return *this;
}

Trace* Trace::getDifferential(const Trace & t) const
{
	Trace *d= new Trace(length);
	for (int i=0;i<length; i++) {
		d->v[i]=v[i]/nTraces - t.v[i]/t.nTraces;
	}
	return d;
}

float Trace::getMark() const
{
	float mark = 0;
	for (int i=0;i<length; i++) {
		mark = std::max(mark, fabsf(v[i]));
	}
	return mark;
}

const char* Trace::getValues() const
{
	std::stringstream s;
	for (int i=0; i<length; i++) s << v[i] << std::endl;
	std::string st = s.str();
	char *buffer = new char[st.length()];
	std::copy(st.begin(), st.end(), buffer);
	return buffer;
}

Trace::~Trace() 
{
	delete[] v;
}

