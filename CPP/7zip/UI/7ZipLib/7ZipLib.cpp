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

static int SevenZipCompress(CObjectVector<CDirItem>* items, UString* pTarget, CArchiveUpdateCallback* pCallback, vector<wstring>* pFailedFiles);
static int SevenZipDecompress(CObjectVector<CDirItem>* items, UString* pTarget, CArchiveExtractCallback* pCallback, vector<wstring>* pFailedFiles);

static int FindItems(vector<wstring>* pSources, CObjectVector<CDirItem>* pItems, int opType, vector<wstring>* pFailedFiles);
static int CheckTarget(wstring* pPath, int opType, UString* pTarget);

int SevenZipOperation(vector<wstring>* pSources, 
	wstring* pPath, 
	int opType, 
	CMyUnknownImp* pCallback, 
	vector<wstring>* pFailedFiles)
{
	if ((pSources == NULL) || (pPath == NULL) || (pCallback == NULL) || (pFailedFiles == NULL))
		return MY7ZIPOP_RES_PARAM_ERROR;

	if (pSources->size() == 0)
		return MY7ZIPOP_RES_SRC_IS_EMPTY;

	pFailedFiles->clear();

	if ((opType != MY7ZIPOP_COMPRESS) && (opType != MY7ZIPOP_DECOMPRESS))
		return MY7ZIPOP_RES_UNKOWN_OPTYPE;

	if ((typeid(*pCallback) != typeid(CArchiveUpdateCallback)) &&
		(typeid(*pCallback) != typeid(CArchiveExtractCallback)))
		return MY7ZIPOP_RES_UNKOWN_CALLBACKTYPE;

	if (!(((opType == MY7ZIPOP_COMPRESS) && (typeid(*pCallback) == typeid(CArchiveUpdateCallback))) || 
		((opType == MY7ZIPOP_DECOMPRESS) && (typeid(*pCallback) != typeid(CArchiveExtractCallback)))))
		return MY7ZIPOP_RES_OP_CALLBACK_MISMATCH;

	CObjectVector<CDirItem> items;
	int result = FindItems(pSources, &items, opType, pFailedFiles);
	if (result != MY7ZIPOP_RES_OK)
		return result;

	UString targetPath;
	result = CheckTarget(pPath, opType, &targetPath);
	if (result != MY7ZIPOP_RES_OK)
		return result;

	if (opType == MY7ZIPOP_COMPRESS)
		return SevenZipCompress(&items, &targetPath, (CArchiveUpdateCallback*)pCallback, pFailedFiles);
	else
		return SevenZipDecompress(&items, &targetPath, (CArchiveExtractCallback*)pCallback, pFailedFiles);
}

int SevenZipCompress(CObjectVector<CDirItem>* pItems, UString* pTarget, CArchiveUpdateCallback* pCallback, vector<wstring>* pFailedFiles)
{
	COutFileStream *outFileStreamSpec;
    CMyComPtr<IOutStream> outFileStream;
	CMyComPtr<IOutArchive> outArchive;
	CMyComPtr<IArchiveUpdateCallback2> updateCallback(pCallback);
	HRESULT result;
	int res = MY7ZIPOP_RES_OK;
	CreateObjectFunc createObjectFunc;
	NWindows::NDLL::CLibrary lib;
	UString filename;
	wstring szfile;

	if (!lib.Load(TEXT(kDllName)))
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	createObjectFunc = (CreateObjectFunc)lib.GetProc("CreateObject");
	if (createObjectFunc == 0)
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	if (createObjectFunc(&CLSID_CFormat7z, &IID_IOutArchive, (void **)&outArchive) != S_OK)
		return MY7ZIPOP_RES_CANNOT_LOAD_7ZIP_DLL;

	outFileStreamSpec = new COutFileStream;
	outFileStream = outFileStreamSpec;
	if (!outFileStreamSpec->Create(*pTarget, false))
		return MY7ZIPOP_RES_TARGET_CANNOT_CREATED;

	pCallback->Init(pItems);

	result = outArchive->UpdateItems(outFileStream, pItems->Size(), updateCallback);
    pCallback->Finilize();

	if (result != S_OK)
		res = MY7ZIPOP_RES_COMPRESS_ERROR;
	else
	{
		if (pCallback->FailedFiles.Size() != 0)
		{
			for (int i = 0; i < pCallback->FailedFiles.Size(); i++)
			{
				filename = pCallback->FailedFiles[i];
				szfile = filename.GetBuffer(filename.Length());
				pFailedFiles->push_back(szfile);
			}

			res = MY7ZIPOP_RES_OK_WITH_FAILED;
		}
	}

	delete updateCallback;

	return res;
}

int SevenZipDecompress(CObjectVector<CDirItem>* pItems, UString* pTarget, CArchiveExtractCallback* pCallback, vector<wstring>* pFailedFiles)
{
	CMyComPtr<IInArchive> inArchive;
	NWindows::NDLL::CLibrary lib;
	CInFileStream *fileSpec;
	UString archiveName;
    CMyComPtr<IInStream> file;
	CMyComPtr<IArchiveExtractCallback> extractCallback(pCallback);
	HRESULT result;
	CreateObjectFunc createObjectFunc;

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

	pCallback->Init(inArchive, *pTarget);
	result = inArchive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);
	if (result != S_OK)
		return MY7ZIPOP_RES_UNCOMPRESS_ERROR;

	return MY7ZIPOP_RES_OK;
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

int FindItems(vector<wstring>* pSources, CObjectVector<CDirItem>* pItems, int opType, vector<wstring>* pIgores)
{
	CDirItem di;
	UString name;
	NFile::NFind::CFileInfoW fi;

	int result = MY7ZIPOP_RES_OK;
	if (opType == MY7ZIPOP_COMPRESS)
	{
		// Source files
		for(vector<wstring>::iterator iterSrc = pSources->begin(); iterSrc == pSources->end(); iterSrc++)
		{
			name = GetUnicodeString((*iterSrc).c_str());
			if (!fi.Find(name))
			{
				pIgores->push_back(*iterSrc);

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
			pIgores->push_back(pSources->front());

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