#pragma once

class Chronometer {
private:
	static __int64 getTicksPerSec();
		 
	static  const __int64 ticksPerSec;

	DWORD beginCount, endCount;
	__int64 beginCountHP, endCountHP;

public:
	void Start();
		 

	void End();
		 
	DWORD GetMilis() { return endCount - beginCount; }

	__int64 GetMicros() {
		return ((endCountHP - beginCountHP) * 1000000) / ticksPerSec;
	}
};
