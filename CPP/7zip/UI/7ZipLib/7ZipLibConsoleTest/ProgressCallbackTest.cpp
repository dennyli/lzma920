#include "StdAfx.h"
#include "ProgressCallbackTest.h"
#include "Types.h"
#include <iostream>
#include <locale> 

using namespace std;

CProgressCallbackTest::CProgressCallbackTest(void)
{
	wcout.imbue(locale("chs"));
}

CProgressCallbackTest::~CProgressCallbackTest(void)
{
}

void CProgressCallbackTest::SetProgressType(OperationType type)
{
	wcout << L"OperationType: " << (int)type << endl;
}

void CProgressCallbackTest::SetTotal(UInt64 size)
{
	wcout << L"Total size: " << size << endl;
}


void CProgressCallbackTest::SetFailedPath(wstring fullPath)
{
	wcout << L"FailedPath: " << fullPath << endl;
}


void CProgressCallbackTest::SetOperationPath(wstring fullPath)
{
	wcout << L"OperationPath: " << fullPath << endl;
}


void CProgressCallbackTest::SetCompleted(UInt64 completeValue)
{
	wcout << L"Completed size: " << completeValue << endl;
}


void CProgressCallbackTest::Finilize()
{
	wcout << L"Finilize" << endl;
}


void CProgressCallbackTest::SetOperationResult(OperationResult result)
{
	wcout << L"OperationResult: " << (int)result << endl;
}


bool CProgressCallbackTest::IsPasswordDefined()
{
	bool IsPasswordDefined = false;
	wcout << L"IsPasswordDefined: " << IsPasswordDefined << endl;

	return IsPasswordDefined;
}


wstring CProgressCallbackTest::GetPassword()
{
	wstring pwd = L"MyPassword";
	wcout << L"Get Password: " << pwd << endl;

	return pwd;
}



