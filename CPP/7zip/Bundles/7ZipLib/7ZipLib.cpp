// 7ZipLib.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "7ZipLib.h"


// ���ǵ���������һ��ʾ��
MY7ZIPLIB_API int nMy7ZipLib=0;

// ���ǵ���������һ��ʾ����
MY7ZIPLIB_API int fnMy7ZipLib(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� 7ZipLib.h
CMy7ZipLib::CMy7ZipLib()
{
	return;
}
