#pragma once

#include <chrono>
#include <iostream>
#include <string>

#include "../chess/Common.h"

class ScopedTimer {
public:
	ScopedTimer(const std::string& name)
		: name(name), start(std::chrono::high_resolution_clock::now()) {}

	~ScopedTimer() {
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << name << " took " << elapsed << " μs\n";
	}

private:
	std::string name;
	std::chrono::high_resolution_clock::time_point start;
};

static inline uint64 cntvct() {
	uint64 cval;
	asm volatile("mrs %0, cntvct_el0" : "=r" (cval));
	return cval;
}

static inline uint64 cntfrq() {
	uint64 freq;
	asm volatile("mrs %0, cntfrq_el0" : "=r" (freq));
	return freq;
}

inline uint64 getTimeElapsed(uint64 startTime) {
	return (cntvct() - startTime) * 1000 / cntfrq();
}

inline uint64 getTimeElapsedUS(uint64 startTime) {
	return (cntvct() - startTime) * 1000000 / cntfrq();
}

inline double uint64ToElapsedUS(uint64 t) {
	return static_cast<double>(t) * 1e6 / static_cast<double>(cntfrq());
}

class SearchScopedTimer {
public:
	SearchScopedTimer(uint64& time)
		: time(time), start(cntvct()) {}

	~SearchScopedTimer() {
		time += cntvct() - start;
	}

private:
	uint64& time;
	uint64 start;

};

