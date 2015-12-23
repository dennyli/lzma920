// Definition for 7Zip Lib
#ifndef __COMMONS_FOR_7ZIPLIB_H__
#define __COMMONS_FOR_7ZIPLIB_H__

#include "Common/IntToString.h"
#include "Common/MyInitGuid.h"
#include "Common/StringConvert.h"

#include "Windows/DLL.h"
#include "Windows/FileIO.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
//#include "Windows/NtCheck.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

#include "../../Common/FileStreams.h"

#include "../../Archive/IArchive.h"

#include "../../IPassword.h"
#include "../../MyVersion.h"

// use another CLSIDs, if you want to support other formats (zip, rar, ...).
// {23170F69-40C1-278A-1000-000110070000}
DEFINE_GUID(CLSID_CFormat7z,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

using namespace NWindows;

// 7zra.dll: created by compiling CPP\7zip\Bundles\Format7zR with console command NMAKE in VC2010 console.
#define kDllName "7zra.dll"

typedef UINT32 (WINAPI * CreateObjectFunc)(
    const GUID *clsID,
    const GUID *interfaceID,
    void **outObject);

#include "DirItem.h"

#endif // __COMMONS_FOR_7ZIPLIB_H__