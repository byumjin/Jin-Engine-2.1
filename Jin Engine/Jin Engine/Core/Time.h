#pragma once
#include <chrono>

static unsigned int currentTime = 0;
static unsigned int deltaTime = 0;
static unsigned int previousTime = 0;

static unsigned int fpsPreviosTime = 0;



using namespace std::chrono;

class Time
{
public:

	Time()
	{
		startTime = high_resolution_clock::now();
	}

	unsigned int getTime()
	{
		time_point<steady_clock> currentTime = high_resolution_clock::now();
		return static_cast<unsigned int>(duration_cast<milliseconds>(currentTime - startTime).count());
	}
private:

	time_point<steady_clock> startTime;

};