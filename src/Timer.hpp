// ================================================
// File: Timer.hpp
// Created on: 2025-06-05 12:34:52
// Last modified: 2025-06-05 12:43:55
// Created by: Alwin R Ajeesh
// ================================================

#include <chrono>
#include <iostream>
#include <ratio>
class Timer
{
public:
	Timer()
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	
	~Timer()
	{
		Stop();
	}
	void Stop()
	{
		auto endTimePoint = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		auto stop = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

		auto duration = stop - start;
		double ms = duration * 0.001;
		std::cout << duration << "us (" << ms << "ms)\n"; 
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
};
