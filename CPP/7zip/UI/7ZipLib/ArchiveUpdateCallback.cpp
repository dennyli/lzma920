#include "StdAfx.h"
#include "ArchiveUpdateCallback.h"

#include "Common.h"

#include "ProgressCallback.h"

static const wchar_t *kEmptyFileAlias = L"[Content]";

STDMETHODIMP CArchiveUpdateCallback::SetTotal(UInt64  size )
{
	if (ProgressCallback)
		ProgressCallback->SetTotal(size);

	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetCompleted(const UInt64 *  completeValue )
{
	if (ProgressCallback)
		ProgressCallback->SetCompleted(*completeValue);

	return S_OK;
}


STDMETHODIMP CArchiveUpdateCallback::EnumProperties(IEnumSTATPROPSTG ** /* enumerator */)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 /* index */,
	Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
{
	if (newData != NULL)
		*newData = BoolToInt(true);
	if (newProperties != NULL)
		*newProperties = BoolToInt(true);
	if (indexInArchive != NULL)
		*indexInArchive = (UInt32)-1;
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
	NWindows::NCOM::CPropVariant prop;

	if (propID == kpidIsAnti)
	{
		prop = false;
		prop.Detach(value);
		return S_OK;
	}

	{
		const CDirItem &dirItem = (*DirItems)[index];
		switch(propID)
		{
		case kpidPath:  prop = dirItem.Name; break;
		case kpidIsDir:  prop = dirItem.isDir(); break;
		case kpidSize:  prop = dirItem.Size; break;
		case kpidAttrib:  prop = dirItem.Attrib; break;
		case kpidCTime:  prop = dirItem.CTime; break;
		case kpidATime:  prop = dirItem.ATime; break;
		case kpidMTime:  prop = dirItem.MTime; break;
		}
	}
	prop.Detach(value);
	return S_OK;
}

HRESULT CArchiveUpdateCallback::Finilize()
{
	if (m_NeedBeClosed)
	{
		//PrintNewLine();
		m_NeedBeClosed = false;
	}

	if (ProgressCallback)
		ProgressCallback->Finilize();

	return S_OK;
}

static void GetStream2(const wchar_t *name)
{
	//PrintString("Compressing  ");
	if (name[0] == 0)
		name = kEmptyFileAlias;
	//PrintString(name);
}

STDMETHODIMP CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)
{
	RINOK(Finilize());

	const CDirItem &dirItem = (*DirItems)[index];
	GetStream2(dirItem.Name);

	if (dirItem.isDir())
		return S_OK;

	{
		CInFileStream *inStreamSpec = new CInFileStream;
		CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
		UString path = DirPrefix + dirItem.FullPath;
		
		if (ProgressCallback)
		{
			wstring szfile = path.GetBuffer(path.Length());
			ProgressCallback->SetOperationPath(szfile);
			path.ReleaseBuffer();
		}

		if (!inStreamSpec->Open(path))
		{
			DWORD sysError = ::GetLastError();
			FailedCodes.Add(sysError);
			FailedFiles.Add(path);
			// if (systemError == ERROR_SHARING_VIOLATION)
			{
				//PrintNewLine();
				//PrintError("WARNING: can't open file");
				return S_FALSE;
			}
		}
		*inStream = inStreamLoc.Detach();
	}
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetOperationResult(Int32  operationResult )
{
	m_NeedBeClosed = true;

	if (ProgressCallback)
		ProgressCallback->SetOperationResult((CProgressCallback::OperationResult)operationResult);

	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size)
{
	if (VolumesSizes.Size() == 0)
		return S_FALSE;
	if (index >= (UInt32)VolumesSizes.Size())
		index = VolumesSizes.Size() - 1;
	*size = VolumesSizes[index];
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)
{
	wchar_t temp[16];
	ConvertUInt32ToString(index + 1, temp);
	UString res = temp;
	while (res.Length() < 2)
		res = UString(L'0') + res;
	UString fileName = VolName;
	fileName += L'.';
	fileName += res;
	fileName += VolExt;
	COutFileStream *streamSpec = new COutFileStream;
	CMyComPtr<ISequentialOutStream> streamLoc(streamSpec);
	if (!streamSpec->Create(fileName, false))
		return ::GetLastError();
	*volumeStream = streamLoc.Detach();
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
	if (!PasswordIsDefined)
	{
		if (AskPassword)
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
	}
	*passwordIsDefined = BoolToInt(PasswordIsDefined);
	return StringToBstr(Password, password);
}


