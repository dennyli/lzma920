// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MY7ZIPLIB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MY7ZIPLIB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MY7ZIPLIB_EXPORTS
#define MY7ZIPLIB_API __declspec(dllexport)
#else
#define MY7ZIPLIB_API __declspec(dllimport)
#endif

// �����Ǵ� 7ZipLib.dll ������
class MY7ZIPLIB_API CMy7ZipLib {
public:
	CMy7ZipLib(void);
	// TODO: �ڴ�������ķ�����
};

extern MY7ZIPLIB_API int nMy7ZipLib;

MY7ZIPLIB_API int fnMy7ZipLib(void);
