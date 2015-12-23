
#ifndef __DIRITEM_H__
#define __DIRITEM_H__

struct CDirItem
{
  UInt64 Size;
  FILETIME CTime;
  FILETIME ATime;
  FILETIME MTime;
  UString Name;
  UString FullPath;
  UInt32 Attrib;

  bool isDir() const { return (Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0 ; }
};

#endif // __DIRITEM_H__