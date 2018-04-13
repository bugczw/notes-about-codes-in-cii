/* $Id$ */
/*
assert������Ϊ����������Ϲ��ܡ���assert(exp)ִ��ʱ�����expΪ0�����ڱ�׼���������stderr���һ��������ʾ����Ϣ��
Assertion failed: expression, file filename, line nnn
Ȼ�����abort��ִֹ�С����е�Դ�ļ���filename���к�nnn������Ԥ�����__FILE__��__LINE__��
���<assert.h>������ʱ�����˺�NDEBUG����ô��assert�����ԡ�
*/
#undef assert
#ifdef NDEBUG
#define assert(e) ((void)0)   //assert(e)��Ч�ڿձ��ʽ((void)0)
//e���ܲ���ִ�У���˲��ܳ�Ϊ�и�ЧӦ�ļ�����̣��縳ֵ����һ����Ҫ����
#else
#include "except.h"
extern void assert(int e);
#define assert(e) ((void)((e)||(RAISE(Assert_Failed),0)))
//������e1||e2���ʽ����Ч�� if(!(e1)) e2 �������
//e2Ϊ���ű��ʽ������Ϊһ��ֵ��voidû�з���ֵ
#endif
