#pragma once

#include <intrin.h>
#include <inttypes.h>
#include <stdio.h>


class InstructionCounter
{
	unsigned __int64  counter;
	unsigned __int64  end_count;
	unsigned __int64 total_count;
public:
	
	void start()
	{
		counter= __rdtsc();
	};

	void end()
	{
		end_count = __rdtsc();
	};

	void print()
	{
		printf("%I64d \n", getCyclesCount());
	}

	unsigned __int64 getCyclesCount()
	{
		end();
		total_count = end_count - counter;
		return  total_count;
	}


};
