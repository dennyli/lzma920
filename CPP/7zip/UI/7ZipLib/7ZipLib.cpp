// 7ZipLib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <typeinfo>
#include "Windows/NtCheck.h"
#include "7ZipLib.h"

#include "Common.h"
#include "ArchiveExtractCallback.h"
#include "ArchiveUpdateCallback.h"

//using NWindows;
//using NWindows::NFile;
//using NWindows::NFile::NDirectory;

static int SevenZipCompress(CObjectVector<CDirItem>* items, 
	UString* pTarget, 
	CProgressCallback* pProgressCallback);
static int SevenZipDecompress(CObjectVector<CDirItem>* items, 
	UString* pTarget, 
	CProgressCallback* pProgressCallback);

static int FindItems(vector<wstring>* pSources, 
	CObjectVector<CDirItem>* pItems, 
	int opType, 
	CProgressCallback* pProgressCallback);

static int CheckTarget(wstring* pPath, int opType, UString* pTarget);

int SevenZipOperation(vector<wstring>* pSources, 
	wstring* pPath, 
	int opType, 
	CProgressCallback* pProgressCallback)
{
	if ((pSources == NULL) || (pPath == NULL))
		return MY7ZIPOP_RES_PARAM_ERROR;

	if (pSources->size() == 0)
		return MY7ZIPOP_RES_SRC_IS_EMPTY;

	if ((opType != MY7ZIPOP_COMPRESS) && (opType != MY7ZIPOP_DECOMPRESS))
		return MY7ZIPOP_RES_UNKOWN_OPTYPE;

	CObjectVector<CDirItem> items;
	int result = FindItems(pSources, &items, opType, pProgressCallback);
	if (result != MY7ZIPOP_RES_OK)
		return result;

	UString targetPath;
	result = CheckTarget(pPath, opType, &targetPath);
	if (result != MY7ZIPOP_RES_OK)
		return result;

	if (opType == MY7ZIPOP_COMPRESS)
		return SevenZipCompress(&items, &targetPath, pProgressCallback);
	else
		return SevenZipDecompress(&items, &targetPath, pProgressCallback);
}

int SevenZipCompress(CObjectVector<CDirItem>* pItems, UString* pTarget, CProgressCallback* pProgressCallback)
{
	COutFileStream *outFileStreamSpec;
	CMyComPtr<IOutArchive> outArchive;
	CArchiveUpdateCallback *updateCallbackSpec;
	HRESULT result;
	int res = MY7ZIPOP_RES_OK;
	NWindows::NDLL::CLibrary lib;
	UString filename;
	wstring szfile;

	if (!lib.Load(TEXT(kDllName)))
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	CreateObjectFunc createObjectFunc = (CreateObjectFunc)lib.GetProc("CreateObject");
	if (createObjectFunc == 0)
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	if (createObjectFunc(&CLSID_CFormat7z, &IID_IOutArchive, (void **)&outArchive) != S_OK)
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	outFileStreamSpec = new COutFileStream;
	CMyComPtr<IOutStream> outFileStream(outFileStreamSpec);
	if (!outFileStreamSpec->Create(*pTarget, false))
		return MY7ZIPOP_RES_TARGET_CANNOT_CREATED;

	updateCallbackSpec = new CArchiveUpdateCallback;
	CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
	updateCallbackSpec->Init(pItems);
	updateCallbackSpec->SetProgressCallback(pProgressCallback);

	if (pProgressCallback)
		pProgressCallback->SetProgressType(CProgressCallback::tCompressing);

	result = outArchive->UpdateItems(outFileStream, pItems->Size(), updateCallback);

	if (pProgressCallback)
		pProgressCallback->SetProgressType(CProgressCallback::tFinilizing);
    updateCallbackSpec->Finilize();

	if (result != S_OK)
		res = MY7ZIPOP_RES_COMPRESS_ERROR;
	else
	{
		if (updateCallbackSpec->FailedFiles.Size() != 0)
		{
			for (int i = 0; i < updateCallbackSpec->FailedFiles.Size(); i++)
			{
				filename = updateCallbackSpec->FailedFiles[i];
				szfile = filename.GetBuffer(filename.Length());
				if (pProgressCallback)
					pProgressCallback->SetFailedPath(szfile);
				filename.ReleaseBuffer();
			}

			res = MY7ZIPOP_RES_OK_WITH_FAILED;
		}
	}

	return res;
}

int SevenZipDecompress(CObjectVector<CDirItem>* pItems, UString* pTarget, CProgressCallback* pProgressCallback)
{
	CMyComPtr<IInArchive> inArchive;
	NWindows::NDLL::CLibrary lib;
	CInFileStream *fileSpec;
	UString archiveName;
    CMyComPtr<IInStream> file;
	CArchiveExtractCallback* extractCallbackSpec;
	HRESULT result;
	CreateObjectFunc createObjectFunc;
	int res = MY7ZIPOP_RES_OK;

	if (!lib.Load(TEXT(kDllName)))
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	createObjectFunc = (CreateObjectFunc)lib.GetProc("CreateObject");
	if (createObjectFunc == 0)
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

    if (createObjectFunc(&CLSID_CFormat7z, &IID_IInArchive, (void **)&inArchive) != S_OK)
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	archiveName = pItems->Front().FullPath;
	fileSpec = new CInFileStream;
	file = fileSpec;
	if (!fileSpec->Open(archiveName))
		return MY7ZIPOP_RES_SRC_CANNOT_OPEN;

	if (inArchive->Open(file, 0, NULL) != S_OK)
		return MY7ZIPOP_RES_SRC_CANNOT_OPEN;

	extractCallbackSpec = new CArchiveExtractCallback;
	CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
	extractCallbackSpec->Init(inArchive, *pTarget);
	extractCallbackSpec->SetProgressCallback(pProgressCallback);

	if (pProgressCallback)
		pProgressCallback->SetProgressType(CProgressCallback::tDecompressing);

	result = inArchive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);

	if (pProgressCallback)
		pProgressCallback->SetProgressType(CProgressCallback::tFinilizing);
    extractCallbackSpec->Finilize();

	if (result != S_OK)
		res = MY7ZIPOP_RES_UNCOMPRESS_ERROR;

	return res;
}

int CheckTarget(wstring* pPath, int opType, UString* pTarget)
{
	UString name;
	int result = MY7ZIPOP_RES_OK;

	name = GetUnicodeString((*pPath).c_str());
	if (opType == MY7ZIPOP_COMPRESS)
	{
		if (NFile::NFind::DoesFileExist(name))
		{
			//try
			//{
				if (!NFile::NDirectory::DeleteFileAlways(name))
					result = MY7ZIPOP_RES_TARGET_CANNOT_DELETED;
			//}
			//catch( CFileException, e )
			//{
			//	result = MY7ZIPOP_RES_TARGET_CANNOT_DELETED;
			//}
			
		}
	}
	else
	{
		if (!NFile::NDirectory::MyGetWindowsDirectory(name))
		{
			if (!NFile::NDirectory::MyCreateDirectory(name))
				result = MY7ZIPOP_RES_TARGET_CANNOT_CREATED;
		}
	}

	*pTarget = L"";
	if (result == MY7ZIPOP_RES_OK)
		*pTarget = name;

	return result;
}

int FindItems(vector<wstring>* pSources, CObjectVector<CDirItem>* pItems, int opType, CProgressCallback* pProgressCallback)
{
	CDirItem di;
	UString name;
	NFile::NFind::CFileInfoW fi;

	int result = MY7ZIPOP_RES_OK;
	if (pProgressCallback)
		pProgressCallback->SetProgressType(CProgressCallback::tPreparing);

	if (opType == MY7ZIPOP_COMPRESS)
	{
		// Source files
		for(vector<wstring>::iterator iterSrc = pSources->begin(); iterSrc != pSources->end(); iterSrc++)
		{
			name = GetUnicodeString((*iterSrc).c_str());
			if (!fi.Find(name))
			{
				if (pProgressCallback)
					pProgressCallback->SetFailedPath(*iterSrc);

				result = MY7ZIPOP_RES_PATH_NOTFOUND;
				continue;
			}

			di.Attrib = fi.Attrib;
			di.Size = fi.Size;
			di.CTime = fi.CTime;
			di.ATime = fi.ATime;
			di.MTime = fi.MTime;
			di.Name = name;
			di.FullPath = name;

			pItems->Add(di);
		}
	}
	else
	{
		name = GetUnicodeString(pSources->front().c_str());
		if (!fi.Find(name))
		{
			if (pProgressCallback)
				pProgressCallback->SetFailedPath(pSources->front());

			result = MY7ZIPOP_RES_PATH_NOTFOUND;
		}
		else
		{
			di.Attrib = fi.Attrib;
			di.Size = fi.Size;
			di.CTime = fi.CTime;
			di.ATime = fi.ATime;
			di.MTime = fi.MTime;
			di.Name = name;
			di.FullPath = name;

			pItems->Add(di);
		}
	}

	return result;
}