/*
	We can't profile this file since we use it in the profiler to get the millseconds value for each function. 
	We might actually want to profile this one time, but not really important, as long as it just uses the QueryPerformanceCounter

*/

// #include <timeapi.h>
static u64 GlobalTimeFrequencyDatum;

inline u64 EasyTime_GetTimeCount()
{
    u64 s = SDL_GetPerformanceCounter();
    
    return s;
}

inline u64 EasyTime_GetSecondsCount() {
	return (EasyTime_GetTimeCount() / GlobalTimeFrequencyDatum);
} 

inline float EasyTime_GetSecondsElapsed(u64 CurrentCount, u64 LastCount)
{
    u64 Difference = CurrentCount - LastCount;
    float Seconds = (float)Difference / (float)GlobalTimeFrequencyDatum;
    
    return Seconds;
}

inline float EasyTime_GetMillisecondsElapsed(u64 CurrentCount, u64 LastCount)
{
    u64 Difference = CurrentCount - LastCount;
    assert(Difference >= 0); //user put them in the right order
    double Seconds = (float)Difference / (float)GlobalTimeFrequencyDatum;
	float millseconds = (float)(Seconds * 1000.0);     
    return millseconds;
    
}


inline void EasyTime_setupTimeDatums() {
    GlobalTimeFrequencyDatum = SDL_GetPerformanceFrequency();
	// timeBeginPeriod(1);
}

static inline u64 EasyTime_getTimeStamp() {
    return (u64)time(NULL);
}