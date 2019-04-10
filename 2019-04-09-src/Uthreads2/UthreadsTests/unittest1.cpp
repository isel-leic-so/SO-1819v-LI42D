#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Include/UThread.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UthreadsTests
{		
	TEST_CLASS(UnitTest1)
	{
	private:
		// thread function for test RunningThreadIsAliveTest
		static VOID Func1(UT_ARGUMENT arg) {
			PBOOL isAlive = (PBOOL)arg;

			*isAlive = UtAlive(UtSelf());
		}

		static VOID Func2Aux(UT_ARGUMENT arg) {
			printf("Running Thread is %p\n", UtSelf());
		}

		static VOID Func2(UT_ARGUMENT arg) {
			PBOOL isAlive = (PBOOL)arg;
			HANDLE hThread = UtCreate(Func2Aux, NULL);
			//Waiting for hThread termination
			//UtJoin(hThread);
			UtYield();
			*isAlive = UtAlive( hThread);
		}
	public:
		
		TEST_METHOD(RunningThreadIsAliveTest)
		{
			UtInit();
			BOOL isAlive = FALSE;
			UtCreate(Func1, &isAlive);
			UtRun();

			Assert::IsTrue( isAlive  == TRUE);
			UtEnd();
		}

		TEST_METHOD(AfterUtExitNotAliveTest)
		{
			UtInit();
			BOOL isAlive = TRUE;
			UtCreate(Func2, &isAlive);
			UtRun();

			Assert::IsTrue(isAlive == FALSE);
			UtEnd();
		}

	};
}