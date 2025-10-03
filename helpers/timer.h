#pragma once
#include <iostream>
#include <string>

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
