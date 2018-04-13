/* $Id$ */
/*
assert宏用于为程序增加诊断功能。当assert(exp)执行时，如果exp为0，则在标准出错输出流stderr输出一条如下所示的信息：
Assertion failed: expression, file filename, line nnn
然后调用abort终止执行。其中的源文件名filename和行号nnn来自于预处理宏__FILE__和__LINE__。
如果<assert.h>被包含时定义了宏NDEBUG，那么宏assert被忽略。
*/
#undef assert
#ifdef NDEBUG
#define assert(e) ((void)0)   //assert(e)等效于空表达式((void)0)
//e可能不被执行，因此不能成为有副效应的计算过程（如赋值）的一个必要部分
#else
#include "except.h"
extern void assert(int e);
#define assert(e) ((void)((e)||(RAISE(Assert_Failed),0)))
//！！！e1||e2表达式，等效与 if(!(e1)) e2 条件语句
//e2为逗号表达式，其结果为一个值，void没有返回值
#endif
