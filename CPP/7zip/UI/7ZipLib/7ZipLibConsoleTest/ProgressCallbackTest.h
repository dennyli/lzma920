#pragma once

#include "ProgressCallback.h"
#include "Types.h"

class CProgressCallbackTest :
	public CProgressCallback
{
public:
	CProgressCallbackTest(void);
	~CProgressCallbackTest(void);

public:
	void SetProgressType(OperationType type);
	void SetTotal(UInt64 size);
	void SetFailedPath(wstring fullPath);
	void SetOperationPath(wstring fullPath);
	void SetCompleted(UInt64 completeValue);
	void Finilize();
	void SetOperationResult(OperationResult result);
	bool IsPasswordDefined();
	wstring GetPassword();
};

