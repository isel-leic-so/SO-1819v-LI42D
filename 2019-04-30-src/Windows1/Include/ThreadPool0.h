#pragma once

#ifdef THREADPOOL_EXPORTS
#define THREADPOOL_API_ENTRY __declspec(dllexport)
#else
#define THREADPOOL_API_ENTRY __declspec(dllimport)
#endif

#ifdef __cplusplus 
extern "C" {
#endif

typedef VOID (*WORK_FUNC)(LPVOID arg);

THREADPOOL_API_ENTRY
VOID TPCreate();

THREADPOOL_API_ENTRY
VOID TPDestroy();

THREADPOOL_API_ENTRY
VOID TPQueueWorkItem(WORK_FUNC func, LPVOID arg);


#ifdef __cplusplus 
}
#endif