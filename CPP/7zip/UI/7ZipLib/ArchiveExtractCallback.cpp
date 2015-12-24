#include "StdAfx.h"
#include "ArchiveExtractCallback.h"

#include "Common.h"

static const wchar_t *kEmptyFileAlias = L"[Content]";
static const wchar_t *kCantDeleteOutputFile = L"ERROR: Can not delete output file ";

static const char *kTestingString    =  "Testing     ";
static const char *kExtractingString =  "Extracting  ";
static const char *kSkippingString   =  "Skipping    ";

static const char *kUnsupportedMethod = "Unsupported Method";
static const char *kCRCFailed = "CRC Failed";
static const char *kDataError = "Data Error";
static const char *kUnknownError = "Unknown Error";

static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
	NCOM::CPropVariant prop;
	RINOK(archive->GetProperty(index, propID, &prop));
	if (prop.vt == VT_BOOL)
		result = VARIANT_BOOLToBool(prop.boolVal);
	else if (prop.vt == VT_EMPTY)
		result = false;
	else
		return E_FAIL;
	return S_OK;
}

static HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result)
{
	return IsArchiveItemProp(archive, index, kpidIsDir, result);
}

void CArchiveExtractCallback::Init(IInArchive *archiveHandler, const UString &directoryPath)
{
	NumErrors = 0;
	_archiveHandler = archiveHandler;
	_directoryPath = directoryPath;
	NFile::NName::NormalizeDirPathPrefix(_directoryPath);

	ProgressCallback = NULL;
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64  size )
{
	if (ProgressCallback)
		ProgressCallback->SetTotal(size);

	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 *  completeValue )
{
	if (ProgressCallback)
		ProgressCallback->SetCompleted(*completeValue);

	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
	ISequentialOutStream **outStream, Int32 askExtractMode)
{
	*outStream = 0;
	_outFileStream.Release();

	{
		// Get Name
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop));

		UString fullPath;
		if (prop.vt == VT_EMPTY)
			fullPath = kEmptyFileAlias;
		else
		{
			if (prop.vt != VT_BSTR)
				return E_FAIL;
			fullPath = prop.bstrVal;
		}
		_filePath = fullPath;

		if (ProgressCallback)
		{
			wstring szfile = _filePath.GetBuffer(_filePath.Length());
			ProgressCallback->SetOperationPath(szfile);
			_filePath.ReleaseBuffer();
		}
	}

	if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
		return S_OK;

	{
		// Get Attrib
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop));
		if (prop.vt == VT_EMPTY)
		{
			_processedFileInfo.Attrib = 0;
			_processedFileInfo.AttribDefined = false;
		}
		else
		{
			if (prop.vt != VT_UI4)
				return E_FAIL;
			_processedFileInfo.Attrib = prop.ulVal;
			_processedFileInfo.AttribDefined = true;
		}
	}

	RINOK(IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.isDir));

	{
		// Get Modified Time
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop));
		_processedFileInfo.MTimeDefined = false;
		switch(prop.vt)
		{
		case VT_EMPTY:
			// _processedFileInfo.MTime = _utcMTimeDefault;
			break;
		case VT_FILETIME:
			_processedFileInfo.MTime = prop.filetime;
			_processedFileInfo.MTimeDefined = true;
			break;
		default:
			return E_FAIL;
		}

	}
	{
		// Get Size
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
		bool newFileSizeDefined = (prop.vt != VT_EMPTY);
		UInt64 newFileSize;
		if (newFileSizeDefined)
			newFileSize = ConvertPropVariantToUInt64(prop);
	}


	{
		// Create folders for file
		int slashPos = _filePath.ReverseFind(WCHAR_PATH_SEPARATOR);
		if (slashPos >= 0)
			NFile::NDirectory::CreateComplexDirectory(_directoryPath + _filePath.Left(slashPos));
	}

	UString fullProcessedPath = _directoryPath + _filePath;
	_diskFilePath = fullProcessedPath;

	if (_processedFileInfo.isDir)
	{
		NFile::NDirectory::CreateComplexDirectory(fullProcessedPath);
	}
	else
	{
		NFile::NFind::CFileInfoW fi;
		if (fi.Find(fullProcessedPath))
		{
			if (!NFile::NDirectory::DeleteFileAlways(fullProcessedPath))
			{
				//PrintString(UString(kCantDeleteOutputFile) + fullProcessedPath);
				return E_ABORT;
			}
		}

		_outFileStreamSpec = new COutFileStream;
		CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
		if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS))
		{
			//PrintString((UString)L"can not open output file " + fullProcessedPath);
			return E_ABORT;
		}
		_outFileStream = outStreamLoc;
		*outStream = outStreamLoc.Detach();
	}
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
	_extractMode = false;
	switch (askExtractMode)
	{
	case NArchive::NExtract::NAskMode::kExtract:  _extractMode = true; break;
	};
	switch (askExtractMode)
	{
	case NArchive::NExtract::NAskMode::kExtract:  
		//PrintString(kExtractingString); 
		break;
	case NArchive::NExtract::NAskMode::kTest:  
		//PrintString(kTestingString); 
		break;
	case NArchive::NExtract::NAskMode::kSkip:  
		//PrintString(kSkippingString); 
		break;
	};
	//PrintString(_filePath);
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
	switch(operationResult)
	{
	case NArchive::NExtract::NOperationResult::kOK:
		break;
	default:
		{
			if (ProgressCallback)
			{
				wstring szfile = _diskFilePath.GetBuffer(_diskFilePath.Length());
				ProgressCallback->SetFailedPath(szfile);
				_diskFilePath.ReleaseBuffer();
			}

			NumErrors++;
			//PrintString("     ");
			switch(operationResult)
			{
			case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
				//PrintString(kUnsupportedMethod);
				break;
			case NArchive::NExtract::NOperationResult::kCRCError:
				//PrintString(kCRCFailed);
				break;
			case NArchive::NExtract::NOperationResult::kDataError:
				//PrintString(kDataError);
				break;
			default:
				//PrintString(kUnknownError);
				break;
			}
		}
	}

	if (_outFileStream != NULL)
	{
		if (_processedFileInfo.MTimeDefined)
			_outFileStreamSpec->SetMTime(&_processedFileInfo.MTime);
		RINOK(_outFileStreamSpec->Close());
	}
	_outFileStream.Release();
	if (_extractMode && _processedFileInfo.AttribDefined)
		NFile::NDirectory::MySetFileAttributes(_diskFilePath, _processedFileInfo.Attrib);
	//PrintNewLine();

	if (ProgressCallback)
		ProgressCallback->SetOperationResult((CProgressCallback::OperationResult)operationResult);

	return S_OK;
}


STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
	if (!PasswordIsDefined)
	{
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		//PrintError("Password is not defined");
		if (ProgressCallback)
		{
			PasswordIsDefined = ProgressCallback->IsPasswordDefined();
			if (PasswordIsDefined)
			{
				wstring _password = ProgressCallback->GetPassword();
				Password = GetUnicodeString(_password.c_str());
			}
			else
				Password = L"";
		}

		//return E_ABORT;
	}
	return StringToBstr(Password, password);
}

HRESULT CArchiveExtractCallback::Finilize()
{
	if (ProgressCallback)
		ProgressCallback->Finilize();

	return S_OK;
}
