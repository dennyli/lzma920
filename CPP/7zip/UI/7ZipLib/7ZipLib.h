#pragma once
// This lib uses 7zra.dll: created by compiling CPP\7zip\Bundles\Format7zR with console command NMAKE in VC2010 console.

#include "exportmacro.h"
#include "Types.h"

// OP Type
#define MY7ZIPOP_COMPRESS					0
#define MY7ZIPOP_DECOMPRESS					1

#define MY7ZIPOP_RES_OK_WITH_FAILED			1
#define MY7ZIPOP_RES_OK						0
#define MY7ZIPOP_RES_UNKOWN_OPTYPE			-1
#define MY7ZIPOP_RES_UNKOWN_CALLBACKTYPE	-2
#define MY7ZIPOP_RES_OP_CALLBACK_MISMATCH	-3
#define MY7ZIPOP_RES_PATH_NOTFOUND			-4
#define MY7ZIPOP_RES_SRC_IS_EMPTY			-5
#define MY7ZIPOP_RES_TARGET_CANNOT_DELETED	-6
#define MY7ZIPOP_RES_TARGET_CANNOT_CREATED	-7
#define MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL	-8
#define MY7ZIPOP_RES_COMPRESS_ERROR			-9
#define MY7ZIPOP_RES_PARAM_ERROR			-10
#define MY7ZIPOP_RES_SRC_CANNOT_OPEN		-11
#define MY7ZIPOP_RES_UNCOMPRESS_ERROR		-12

#include <vector>
#include <string>

#include "ProgressCallback.h"

using namespace std;

EXTERN_C_BEGIN

// 7ZipOperation
// paras:
//		pSources --- collection of full filenames. NOT NULL, For compress, it's source paths; for decompress, the first item is source path.
//		pPath --- target path. NOT NULL, For compress, it's target full 7-Zip path, if path is exists, overwrite it without tips; 
//							for decompress, it's target directory.
//		opType --- OP Type, MY7ZIPOP_COMPRESS or MY7ZIPOP_DECOMPRESS.
//		pCallback --- callback class point, NOT NULL,  type of CArchiveUpdateCallback or CArchiveExtractCallback.
//		pFailedFiles --- collection of failed files filenames, NOT NULL.
// result:
//		OP result type, MY7ZIPOP_RES_xxx
MY7ZIPLIB_API int SevenZipOperation(vector<wstring>* pSources, 
	wstring* pPath, 
	int opType, 
	CProgressCallback* pProgressCallback);

EXTERN_C_END