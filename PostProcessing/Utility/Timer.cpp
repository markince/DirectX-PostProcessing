//--------------------------------------------------------------------------------------
// Timer class - works like a stopwatch
//--------------------------------------------------------------------------------------

#include "Timer.h"
#include <Windows.h>

// Constructor //

Timer::Timer()
{
	// Try to initialise performance timer, will use low-resolution timer on failure
	LARGE_INTEGER highResFreq;
	mHighRes = (QueryPerformanceFrequency(&highResFreq) != 0);
	if (mHighRes)
	{
		mHighResFreq = highResFreq.QuadPart;
	}

	// Reset and start the timer
	Reset();
	mRunning = true;
}


// Timer control //

// Start the timer running
void Timer::Start()
{
	if (!mRunning)
	{
		mRunning = true;

		// Get restart time - add time passed since stop time to the start and lap times
		// Select high or low-resolution timer
		if (mHighRes)
		{
			LARGE_INTEGER newHighResTime;
			QueryPerformanceCounter( &newHighResTime );
			mHighResStart += (newHighResTime.QuadPart - mHighResStop);
			mHighResLap += (newHighResTime.QuadPart - mHighResStop);
		}
		else
		{
			uint32_t newLowResTime;
			newLowResTime = timeGetTime();
			mLowResStart += (newLowResTime - mLowResStop);
			mLowResLap += (newLowResTime - mLowResStop);
		}
	}
}

// Stop the timer running
void Timer::Stop()
{
	mRunning = false;

	// Get stop time
	// Select high or low-resolution timer
	if (mHighRes)
	{
		LARGE_INTEGER highResStop;
		QueryPerformanceCounter( &highResStop);
		mHighResStop = highResStop.QuadPart;
	}
	else
	{
		mLowResStop = timeGetTime();
	}
}

// Reset the timer to zero
void Timer::Reset()
{
	// Reset start, lap and stop times to current time
	// Select high or low-resolution timer
	if (mHighRes)
	{
		LARGE_INTEGER highResStart;
		QueryPerformanceCounter( &highResStart);
		mHighResStart = highResStart.QuadPart;
		mHighResLap = mHighResStart;
		mHighResStop = mHighResStart;
	}
	else
	{
		mLowResStart = timeGetTime();
		mLowResLap = mLowResStart;
		mLowResStop = mLowResStart;
	}
}


// Timing //

// Get frequency of the timer being used (in counts per second)
float Timer::GetFrequency()
{
	// Select high or low-resolution timer
	if (mHighRes)
	{
		return static_cast<float>(mHighResFreq);
	}
	else
	{
		return 1000.0f;
	}
}

// Get time passed (seconds) since since timer was started or last reset
float Timer::GetTime()
{
	float fTime;
	if (mHighRes)
	{

		LARGE_INTEGER newHighResTime;
		if (mRunning)
		{
			QueryPerformanceCounter( &newHighResTime );
		}
		else
		{
			newHighResTime.QuadPart = mHighResStop;
		}
		double dTime = static_cast<double>(newHighResTime.QuadPart - mHighResStart) / static_cast<double>(mHighResFreq);
		fTime = static_cast<float>(dTime);
	}
	else
	{
		uint32_t newLowResTime;
		if (mRunning)
		{
			newLowResTime = timeGetTime();
		}
		else
		{
			newLowResTime = mLowResStop;
		}
		fTime = static_cast<float>(newLowResTime - mLowResStart) / 1000.0f;
	}

	return fTime;
}

// Get time passed (seconds) since last call to this function. If this is the first call, then
// the time since timer was started or the last reset is returned
float Timer::GetLapTime()
{
	float fTime;
	if (mHighRes)
	{
		LARGE_INTEGER newHighResTime;
		if (mRunning)
		{
			QueryPerformanceCounter( &newHighResTime );
		}
		else
		{
			newHighResTime.QuadPart = mHighResStop;
		}
		double dTime = static_cast<double>(newHighResTime.QuadPart - mHighResLap) / static_cast<double>(mHighResFreq);
		fTime = static_cast<float>(dTime);
		mHighResLap = newHighResTime.QuadPart;
	}
	else
	{
		uint32_t newLowResTime;
		if (mRunning)
		{
			newLowResTime = timeGetTime();
		}
		else
		{
			newLowResTime = mLowResStop;
		}
		fTime = static_cast<float>(newLowResTime - mLowResLap) / 1000.0f;
		mLowResLap = newLowResTime;
	}
	return fTime;
}
