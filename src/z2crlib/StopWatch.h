#ifndef __TIMESTOP_HPP__
#define __TIMESTOP_HPP__

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <Core/Core.h>

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

using namespace Upp;

#ifdef PLATFORM_WIN32

class StopWatch : Moveable<StopWatch> {
	LARGE_INTEGER start;
	LARGE_INTEGER stop;
	LARGE_INTEGER freq;

public:
	double  Elapsed() {
		QueryPerformanceCounter(&stop);
		return (stop.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
	}

	double Seconds() {
		return (double)Elapsed() / 1000;
	}
	
	String ToString() {
		double time = Elapsed();
		return Format("%d.%03d", int(time / 1000), int(time) % 1000);
	}
	
	void Reset() {
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start);
	}

	StopWatch() {
		Reset();
	}
};

#endif

#ifdef PLATFORM_POSIX

#include <time.h>

class StopWatch : Moveable<StopWatch> {
	timespec start;

public:
	double  Elapsed() {
		timespec end;
		clock_gettime(CLOCK_REALTIME, &end);
		
		return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
	}

	double Seconds() {
		return (double)Elapsed() / 1000;
	}
	
	String ToString() {
		double time = Elapsed();
		return Format("%d.%03d", int(time / 1000), int(time) % 1000);
	}
	
	void Reset() {
		clock_gettime(CLOCK_REALTIME, &start);
	}

	StopWatch() {
		Reset();
	}
};

#endif

#endif