// SimpleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "usynch.h"
#include "List.h"

/////////////////////////////////////////////
//
// CCISEL 
// 2007-2011
//
// UThread    Library First    Test
//
// Jorge Martins, 2011
////////////////////////////////////////////
#define DEBUG

#define MAX_THREADS 10

#include <crtdbg.h>
#include <stdio.h>

#include "..\Include\Uthread.h"


///////////////////////////////////////////////////////////////
//															 //
// Test 1: N threads, each one printing its number M times //
//															 //
///////////////////////////////////////////////////////////////

ULONG Test1_Count;


VOID Test1_Thread (UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR) Argument;	
	 
	for (Index = 0; Index < 100; ++Index) {
	    putchar(Char);
		
	    if ((rand() % 4) == 0) {
		    UtYield();
	    }	 
    }
	++Test1_Count;
}

VOID Test1 ()  {
	ULONG Index;

	Test1_Count = 0; 

	printf("\n :: Test 1 - BEGIN :: \n\n");

	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Test1_Thread, (UT_ARGUMENT) ('0' + Index));
	}   

	UtRun();

	_ASSERTE(Test1_Count == MAX_THREADS);
	printf("\n\n :: Test 1 - END :: \n");
}

VOID Func1(UT_ARGUMENT arg) {
	printf("Start Func1\n");
	UtYield();
	printf("End Func1\n");
}

VOID Func2(UT_ARGUMENT arg) {
	printf("Func2\n");
}

VOID Test2() {
	printf("\n :: Test 2 - BEGIN :: \n\n");
	UtCreate(Func1, NULL);
	UtCreate(Func2, NULL);
	UtRun();
	printf("\n\n :: Test 2 - END :: \n");
}




int main () {
	UtInit();
 
	Test2();
	getchar();
	
	 
	UtEnd();
	return 0;
}


