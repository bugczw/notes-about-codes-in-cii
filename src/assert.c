static char rcsid[] = "$Id$";
#include "assert.h"
const Except_T Assert_Failed = { "Assertion failed" };//没有处理这条，断言失败导致程序放弃执行，并输出相应信息
void (assert)(int e) {
	assert(e);//？？？库函数？？？
}
