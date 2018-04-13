//了解函数TlsGetValue()功能


static char rcsid[] = "$Id$" "\n$Id$";
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "except.h"
#define T Except_T
Except_Frame *Except_stack = NULL;
void Except_raise(const T *e, const char *file,
	int line) {
	//由RAISE，RERAISE调用。该函数会在栈顶的异常帧中填写exception、file、line字段，将栈顶的Except_Frame弹出栈，并调用longjmp
#ifdef WIN32
	Except_Frame *p; 

	if (Except_index == -1)
		Except_init();
	p = TlsGetValue(Except_index);//？？？？TlsGetValue？？？
	//没有的话建立一个新的栈顶
#else
	Except_Frame *p = Except_stack;// 设置p为异常的栈顶
#endif
	assert(e);
	if (p == NULL) {
		// 如果p为NULL，则直接退出，此时应该是异常被re-raised,且无相应的解决方案，所以系统选择退出
		fprintf(stderr, "Uncaught exception");
		if (e->reason)
			fprintf(stderr, " %s", e->reason);
		else
			fprintf(stderr, " at 0x%p", e);
		if (file && line > 0)
			fprintf(stderr, " raised at %s:%d\n", file, line);
		fprintf(stderr, "aborting...\n");
		fflush(stderr);
		abort();
	}
	p->exception = e;// 对异常进行赋值操作
	p->file = file;
	p->line = line;
#ifdef WIN32
	Except_pop();
#else
	Except_stack = Except_stack->prev;// 与TRY配合使用，如果出现异常需要处理，弹出栈顶
#endif
	longjmp(p->env, Except_raised);// 跳转指令，同时设置setjmp返回的值为Except_raised
}
#ifdef WIN32
_CRTIMP void __cdecl _assert(void *, void *, unsigned);
#undef assert
#define assert(e) ((e) || (_assert(#e, __FILE__, __LINE__), 0))

int Except_index = -1;
void Except_init(void) {//初始化帧
	BOOL cond;

	Except_index = TlsAlloc();
	assert(Except_index != TLS_OUT_OF_INDEXES);
	cond = TlsSetValue(Except_index, NULL);
	assert(cond == TRUE);
}

void Except_push(Except_Frame *fp) {
	//往帧里面压入元素
	BOOL cond;

	fp->prev = TlsGetValue(Except_index);
	cond = TlsSetValue(Except_index, fp);//？？？？
	assert(cond == TRUE);
}

void Except_pop(void) {
	//往帧里面弹出元素
	BOOL cond;
	Except_Frame *tos = TlsGetValue(Except_index);

	cond = TlsSetValue(Except_index, tos->prev);//？？？？
	assert(cond == TRUE);
}
#endif
