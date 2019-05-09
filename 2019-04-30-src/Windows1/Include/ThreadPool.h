#pragma once

#ifdef THREADPOOL_EXPORTS
#define TP_API _declspec(dllexport)
#else
#define TP_API _declspec(dllimport)
#endif

typedef VOID(*WORK_FUNC)(LPVOID);

#ifdef __cplusplus
extern  "C" {
#endif

	TP_API VOID TP_QueueWorkItem(WORK_FUNC func, LPVOID arg);
	TP_API VOID TP_Init();
	TP_API VOID TP_Destroy();
#ifdef __cplusplus
}
#endif