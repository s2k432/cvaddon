#ifndef _WINDOWS_FAST_TIMER_H
#define _WINDOWS_FAST_TIMER_H

////////////////////////////////////////////////////////////
//                 Windows Fast Timer Class
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// High accuracy Windows-only timer
// Inspired by the class posted in:
// http://groups.google.com.au/group/comp.lang.c++/browse_thread/thread/5b78dc1667fdc380/6a7a54dc26c2d425%236a7a54dc26c2d425
////////////////////////////////////////////////////////////
// Usage Notes
// ---
// Only use to time periods > 50us, as the 
// timing functions may take considerable 
// processing time
//
// Example
// ---
// while(1)
// { 
//    std::cerr << timer.getLoopTime() << std::endl;
//    <do_processing>
// }
// 
// float oldTime = timer.getTime();
//    
// <do_more_processing>
// float timeDif = timer.getTime() - oldTime;
// std::cerr << timeDif << std::endl;
////////////////////////////////////////////////////////////

// For Query{foo} functions
#include <winbase.h>

// Highly accurate Windows-only timer
// Only use to time periods > 50us, as the 
// timing functions may take considerable 
// processing time
class FastTimer
{
public:
	FastTimer();
	~FastTimer();

	// Restarts timer
	void restart();
	
	// Returns time in ms from last call of restart (or timer creation)
	float getTime() const;

	// Returns time from last call of getLoopTime()
	float getLoopTime();

	// Call this to update the counter frequency
	// May improve accuracy when used every 5-10 timer calls
	// if used on systems with variable CPU clock ratess
	void retuneFrequency();

private:
	LARGE_INTEGER frequency;
	LARGE_INTEGER startTime;
	LARGE_INTEGER lastLoopTime;
};

inline FastTimer::FastTimer()
{
	QueryPerformanceFrequency( &frequency );
	QueryPerformanceCounter( &startTime );
	QueryPerformanceCounter( &lastLoopTime );
}

inline FastTimer::~FastTimer() {}


inline float FastTimer::getTime() const
{
	LARGE_INTEGER curTime;
	QueryPerformanceCounter( &curTime );
	return float( double(curTime.QuadPart - startTime.QuadPart) / double(frequency.QuadPart) * 1e3);
}

inline void FastTimer::restart()
{
	QueryPerformanceCounter( &startTime );
	QueryPerformanceCounter( &lastLoopTime );
} 

inline float FastTimer::getLoopTime()
{
	LARGE_INTEGER delta;
	QueryPerformanceCounter( &delta );
	delta.QuadPart -= lastLoopTime.QuadPart;
	
	QueryPerformanceCounter( &lastLoopTime );
	return float( double(delta.QuadPart) / double(frequency.QuadPart)  * 1e3);
}

inline void FastTimer::retuneFrequency()
{
	QueryPerformanceFrequency( &frequency );
}

#endif