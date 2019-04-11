#include "stdafx.h"
#include "CppUnitTest.h"

#include "../Include/Uthread.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UthreadLabTests
{		
	TEST_MODULE_INITIALIZE(StartTests)
	{
		UtInit();
	}

	TEST_MODULE_CLEANUP(EndTests)
	{
		UtEnd();
	}

	TEST_CLASS(LabTests)
	{
	private:
		static VOID Func1CtxTest(UT_ARGUMENT  arg) {
			UtYield();
		}

		static VOID Func2CtxTest(UT_ARGUMENT  arg) {
			 
		}

		// thread function for test RunningThreadIsAliveTest
		static VOID Func1IsAliveTest(UT_ARGUMENT arg) {
			PBOOL isAlive = (PBOOL)arg;

			*isAlive = UtAlive(UtSelf());
		}

		static VOID Func3IsAliveTest(UT_ARGUMENT arg) {
			printf("Running Thread is %p\n", UtSelf());
		}

		static VOID Func2IsAliveTest(UT_ARGUMENT arg) {
			PBOOL isAlive = (PBOOL)arg;
			HANDLE hThread = UtCreate(Func3IsAliveTest, NULL);
			//Waiting for hThread termination
			UtJoin(hThread);
		
			*isAlive = UtAlive(hThread);
		}
	public:
		
		TEST_METHOD(TestCtxSwitchCnt1)
		{
			DWORD expectedSwitchs = 4;
			UtCreate(Func1CtxTest, NULL);
			UtCreate(Func2CtxTest, NULL);

			UtRun();

			Assert::AreEqual(expectedSwitchs, UtGetSwitchCount());
		}

		TEST_METHOD(RunningThreadIsAliveTest)
		{
			
			BOOL isAlive = FALSE;
			UtCreate(Func1IsAliveTest, &isAlive);
			UtRun();

			Assert::IsTrue(isAlive == TRUE);
		}

		TEST_METHOD(AfterUtExitNotAliveTest)
		{
			
			BOOL isAlive = TRUE;
			UtCreate(Func2IsAliveTest, &isAlive);
			UtRun();

			Assert::IsTrue(isAlive == FALSE);
		
		}



	};
}