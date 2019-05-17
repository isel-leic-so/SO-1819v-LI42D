#pragma once

#ifdef THREADPOOL_EXPORTS
#define UTILS_API __declspec(dllexport)
#else
#define UTILS_API __declspec(dllimport)
#endif

 

UTILS_API VOID TpInit();
UTILS_API VOID TpQueueItem(LPTHREAD_START_ROUTINE func, LPVOID arg);
 