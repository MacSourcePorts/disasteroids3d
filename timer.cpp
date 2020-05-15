
#include "timer.h"

bool bTimerInitialized = false;

timer_t g_timer;

void TimerInit(void)								// Initialize Our Timer (Get It Ready)
{
	memset(&g_timer, 0, sizeof(g_timer));					// Clear Our Timer Structure

#ifdef _WIN32
	// Check To See If A Performance Counter Is Available
	// If One Is Available The Timer Frequency Will Be Updated
	if (!QueryPerformanceFrequency((LARGE_INTEGER *) &g_timer.frequency))
	{
#endif
		// No Performace Counter Available
		g_timer.performance_timer	= false;				// Set Performance Timer To FALSE
		g_timer.mm_timer_start	= SDL_GetTicks();		// Use SDL_GetTicks() To Get Current Time
		g_timer.resolution = 1.0f/1000.0f;				// Set Our Timer Resolution To .001f
		g_timer.frequency = 1000;							// Set Our Timer Frequency To 1000
		g_timer.mm_timer_elapsed	= g_timer.mm_timer_start;		// Set The Elapsed Time To The Current Time
#ifdef _WIN32
	}
	else
	{
		// Performance Counter Is Available, Use It Instead Of The Multimedia Timer
		// Get The Current Time And Store It In performance_timer_start
		QueryPerformanceCounter((LARGE_INTEGER *) &g_timer.performance_timer_start);
		g_timer.performance_timer		= true;				// Set Performance Timer To TRUE
		// Calculate The Timer Resolution Using The Timer Frequency
		g_timer.resolution		= (float) (((double)1.0f)/((double)g_timer.frequency));
		// Set The Elapsed Time To The Current Time
		g_timer.performance_timer_elapsed	= g_timer.performance_timer_start;
	}
#endif

	// Set flag to store that timer was already initialized
	bTimerInitialized = true;

}


// Get Time In Seconds
float TimerGetTime()
{
	__int64 time;									// time Will Hold A 64 Bit Integer

	// Make sure the timer's initialized -- if not initialize it!
	if (!bTimerInitialized)
		TimerInit();

#ifdef _WIN32
	// Are We Using The Performance Timer?
	if (g_timer.performance_timer)
	{
		// Grab The Current Performance Time
		QueryPerformanceCounter((LARGE_INTEGER *) &time);

		// Return The Current Time Minus The Start Time Multiplied By The Resolution 
		return (((float)(time - g_timer.performance_timer_start) * g_timer.resolution));
	}
	else
	{
#endif
		// Return The Current Time Minus The Start Time Multiplied By The Resolution 
		return (((float)(SDL_GetTicks() - g_timer.mm_timer_start) * g_timer.resolution));
#ifdef _WIN32
	}
#endif
}

float TimerGetTimeMS()
{
	return TimerGetTime() * 1000.0f;
}

