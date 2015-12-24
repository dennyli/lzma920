// 7ZipLibConsoleTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "7ZipLib.h"
#include "ProgressCallback.h"
#include "ProgressCallbackTest.h"

#include <iostream>
#include <locale> 

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	vector<wstring> encFiles;
	//encFiles.push_back(L"D:\\GitHub_Projects\\lzma920\\CPP\\7zip\\UI\\7ZipLib\\Debug\\dllmain.obj");
	//encFiles.push_back(L"D:\\GitHub_Projects\\lzma920\\CPP\\7zip\\UI\\7ZipLib\\Debug\\FileDir.obj");
	//encFiles.push_back(L"D:\\GitHub_Projects\\lzma920\\CPP\\7zip\\UI\\7ZipLib\\Debug\\FileFind.obj");
	//encFiles.push_back(L"D:\\GitHub_Projects\\lzma920\\CPP\\7zip\\UI\\7ZipLib\\Debug\\FileName.obj");

	//wstring tarPath = L"D:\\GitHub_Projects\\lzma920\\CPP\\7zip\\UI\\7ZipLib\\Debug\\test.7z";	
	
	encFiles.push_back(L"dllmain.obj");
	encFiles.push_back(L"FileDir.obj");
	encFiles.push_back(L"FileFind.obj");
	encFiles.push_back(L"FileName.obj");

	wstring tarPath = L"test.7z";

	CProgressCallbackTest callback;
	int result;
	//int result = SevenZipOperation(&encFiles, &tarPath, MY7ZIPOP_COMPRESS, (CProgressCallback*)&callback);

	//wcout.imbue(locale("chs"));
	//wcout << L"ZipOperation: " << result << endl;

	vector<wstring> decFiles;
	decFiles.push_back(L"test.7z");

	wstring tarDecPath = L"D:\\GitHub_Projects\\lzma920\\CPP\\7zip\\UI\\7ZipLib\\Debug\\test";

	result = SevenZipOperation(&decFiles, &tarDecPath, MY7ZIPOP_DECOMPRESS, (CProgressCallback*)&callback);

	wcout.imbue(locale("chs"));
	wcout << L"ZipOperation: " << result << endl;

	while(!_gettch());

	return 0;
}

