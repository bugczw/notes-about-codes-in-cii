static char rcsid[] = "$Id$";
#include "assert.h"
const Except_T Assert_Failed = { "Assertion failed" };//û�д�������������ʧ�ܵ��³������ִ�У��������Ӧ��Ϣ
void (assert)(int e) {
	assert(e);//�������⺯��������
}
