/*
#include <stdarg.h>包含一系列解决变参问题的宏以及函数。例如：
void va_start ( va_list ap, param );
//对va_list变量进行初始化，将ap指针指向参数列表中的第一个参数
type va_arg ( va_list ap, type ); 
//获取参数，类型为 type 类型，返回值也为 type 类型
int vsprintf(char *string, char *format, va_list ap);
//将ap(通常是字符串) 按format格式写入字符串string中
void va_end ( va_list ap ); 
//回收ap指针


使用可变参数的步骤：
　　1)首先在函数里定义一个va_list型的变量,可以是指向参数的指针. 
　　2)然后用va_start宏初始化变量arg_ptr,这个宏的第二个参数是第一个可变参数的前一个参数,是一个固定的参数. 
　　3)然后用va_arg返回可变的参数,并赋值给整数j. va_arg的第二个参数是你要返回的参数的类型.
　　4)最后用va_end宏结束可变参数的获取.然后你就可以在函数里使用第二个参数了.如果函数有多个可变参数的,依次调用va_arg获取各个参数.
*/




static char rcsid[] = "$Id$";
#include <stdarg.h>
#include <stddef.h>
#include "assert.h"
#include "mem.h"
#include "list.h"
#define T List_T
T List_push(T list, void *x) {
	//在链表list的起始处添加一个包含x的新节点，并返回新的链表
	//可用于创建新链表，但可能引发Mem_Failed异常
	T p;
	NEW(p);
	p->first = x;
	p->rest  = list;
	return p;
}
T List_list(void *x, ...) {
	//参数列表，数量可变的参数，可变部分传递的指针为void
	//使用一个双重指针list_T*，来指向表示应该分配的新节点的指针
	//创建并返回一个链表。调用该函数时需要传递N+1个指针作为参数
	//前N指针非NULL，最后一个为NULL指针。该函数会创建一个包含N个节点的链表
	//各个结点的first字段包含了N个非NULL的指针。而第N个节点rest字段为NULL
	//调用时最后一个参数要为NULL
	va_list ap;
	T list, *p = &list;
	va_start(ap, x);
	for ( ; x; x = va_arg(ap, void *)) {
		NEW(*p);
		//*p分配未初始化的内存,即给list分配，mem.h
		(*p)->first = x;
		p = &(*p)->rest;
	}
	*p = NULL;
	va_end(ap);
	return list;
}
T List_append(T list, T tail) {
	//将一个链表附加到另外一个，该函数的tail赋值给list最后1个节点的rest字段。
	T *p = &list;
	while (*p)
		p = &(*p)->rest;
	*p = tail;
	return list;
}
T List_copy(T list) {
	//复制参数链表。并返回副本
	T head, *p = &head;
	for ( ; list; list = list->rest) {
		NEW(*p);
		(*p)->first = list->first;
		p = &(*p)->rest;
	}
	*p = NULL;
	return head;
}
T List_pop(T list, void **x) {
	//将第一个结点的first字段赋值给*x，移除第一个节点，最后返回结果链表
	if (list) {
		T head = list->rest;
		if (x)
			*x = list->first;
		FREE(list);
		return head;
	} else
		return list;
}
T List_reverse(T list) {
	//逆转参数链表中结点顺序，返回结果链表,head总是反转后链表的第一个节点
	T head = NULL, next;
	for ( ; list; list = next) {
		next = list->rest;
		list->rest = head;
		head = list;
	}
	return head;
}
int List_length(T list) {
	//返回参数链表中的节点数目
	int n;
	for (n = 0; list; list = list->rest)
		n++;
	return n;
}
void List_free(T *list) {
	//*list不是NULL，则释放*list链表中的所有节点，并设置为NULL类型
	T next;
	assert(list);
	for ( ; *list; *list = next) {
		next = (*list)->rest;
		FREE(*list);
	}
}
void List_map(T list,
	void apply(void **x, void *cl), void *cl) {
	//对list链表中的每个结点调用apply指向的函数。
	//客户端可以用list_map传递于应用程序的指针cl，该指针接下来传递给*apply，用作第二个参数
	//对链表中的每个节点，都会指向结点first字段的指针和cl作为参数来调用*apply
	//first可能会被apply修改
	assert(apply);
	for ( ; list; list = list->rest)
		apply(&list->first, cl);
}
void **List_toArray(T list, void *end) {
	//创建一个数组，数组中0-N-1范围内别包含了链表中N个节点first字段值，数组中元素N包含了end值
	//end通常是一个NULL指针。函数返回一个指向数组第一个元素的非NULL指针,因此不需要检验
	int i, n = List_length(list);
	void **array = ALLOC((n + 1)*sizeof (*array));
	for (i = 0; i < n; i++) {
		array[i] = list->first;
		list = list->rest;
	}
	array[i] = end;
	return array;
}

