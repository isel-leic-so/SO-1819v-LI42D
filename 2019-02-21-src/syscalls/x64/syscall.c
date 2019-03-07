#include <stdio.h>

typedef unsigned long ULONG;

void   NtQueryTimerResolution(  ULONG * MinimumResolution,
								ULONG *MaximumResolution,
							    ULONG *CurrentResolution);




int main()
{
	ULONG MinimumResolution=0;
	ULONG MaximumResolution=0;
	ULONG CurrentResolution=0;

	
	NtQueryTimerResolution( &MinimumResolution,
							&MaximumResolution,
							&CurrentResolution);

	printf("MinimumResolution=%ld, MaximumResolution=%ld, CurrentResolution=%ld\n",
			MinimumResolution, MaximumResolution, CurrentResolution);
  
   
	return 0;
}

