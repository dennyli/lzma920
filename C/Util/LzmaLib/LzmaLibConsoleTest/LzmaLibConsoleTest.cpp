// LzmaLibConsoleTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "LzmaLib.h"

static SRes ShowProgress(void *p, UInt64 inSize, UInt64 outSize)
{
	return SZ_OK;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char *dest = "C:\\1.7z";
	char *src = "C:\\1.wmv";

	ICompressProgress progress;
	progress.Progress = &ShowProgress;

	LzmaCompressFile((unsigned char *)dest, (unsigned char *)src, &progress);

	return 0;
}

