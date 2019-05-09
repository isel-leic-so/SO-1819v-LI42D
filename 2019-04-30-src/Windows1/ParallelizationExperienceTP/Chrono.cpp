#include "stdafx.h"
#include "chrono.h"

 __int64 Chronometer::getTicksPerSec() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

 const __int64 Chronometer::ticksPerSec = getTicksPerSec();


 void Chronometer::Start() {
	 LARGE_INTEGER now;
	 QueryPerformanceCounter(&now);
	 beginCountHP = now.QuadPart;
	 beginCount = GetTickCount();
 }

 void Chronometer::End() {
	 LARGE_INTEGER now;
	 QueryPerformanceCounter(&now);
	 endCountHP = now.QuadPart;
	 endCount = GetTickCount();
 }