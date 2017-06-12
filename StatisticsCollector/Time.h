#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#endif

namespace
{

#ifdef WIN32
	inline std::int64_t get_frequency()
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return freq.QuadPart;
	}
#endif
}

uint64_t time_now()
{
#ifdef _WIN32
	const static double freq = static_cast<double>(get_frequency());
	LARGE_INTEGER raw;
	QueryPerformanceCounter(&raw);
	return static_cast<std::uint64_t>(static_cast<double>(raw.QuadPart) * 1000000000.0 / freq);
#else
	timespec cur_time_info;
	clock_gettime(CLOCK_MONOTONIC, &cur_time_info);
	return static_cast<uint64_t>(cur_time_info.tv_sec * 1000000000LL + cur_time_info.tv_nsec);
#endif
}