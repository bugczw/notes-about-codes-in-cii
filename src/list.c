/*
#include <stdarg.h>����һϵ�н���������ĺ��Լ����������磺
void va_start ( va_list ap, param );
//��va_list�������г�ʼ������apָ��ָ������б��еĵ�һ������
type va_arg ( va_list ap, type ); 
//��ȡ����������Ϊ type ���ͣ�����ֵҲΪ type ����
int vsprintf(char *string, char *format, va_list ap);
//��ap(ͨ�����ַ���) ��format��ʽд���ַ���string��
void va_end ( va_list ap ); 
//����apָ��


ʹ�ÿɱ�����Ĳ��裺
����1)�����ں����ﶨ��һ��va_list�͵ı���,������ָ�������ָ��. 
����2)Ȼ����va_start���ʼ������arg_ptr,�����ĵڶ��������ǵ�һ���ɱ������ǰһ������,��һ���̶��Ĳ���. 
����3)Ȼ����va_arg���ؿɱ�Ĳ���,����ֵ������j. va_arg�ĵڶ�����������Ҫ���صĲ���������.
����4)�����va_end������ɱ�����Ļ�ȡ.Ȼ����Ϳ����ں�����ʹ�õڶ���������.��������ж���ɱ������,���ε���va_arg��ȡ��������.
*/




static char rcsid[] = "$Id$";
#include <stdarg.h>
#include <stddef.h>
#include "assert.h"
#include "mem.h"
#include "list.h"
#define T List_T
T List_push(T list, void *x) {
	//������list����ʼ�����һ������x���½ڵ㣬�������µ�����
	//�����ڴ�������������������Mem_Failed�쳣
	T p;
	NEW(p);
	p->first = x;
	p->rest  = list;
	return p;
}
T List_list(void *x, ...) {
	//�����б������ɱ�Ĳ������ɱ䲿�ִ��ݵ�ָ��Ϊvoid
	//ʹ��һ��˫��ָ��list_T*����ָ���ʾӦ�÷�����½ڵ��ָ��
	//����������һ���������øú���ʱ��Ҫ����N+1��ָ����Ϊ����
	//ǰNָ���NULL�����һ��ΪNULLָ�롣�ú����ᴴ��һ������N���ڵ������
	//��������first�ֶΰ�����N����NULL��ָ�롣����N���ڵ�rest�ֶ�ΪNULL
	//����ʱ���һ������ҪΪNULL
	va_list ap;
	T list, *p = &list;
	va_start(ap, x);
	for ( ; x; x = va_arg(ap, void *)) {
		NEW(*p);
		//*p����δ��ʼ�����ڴ�,����list���䣬mem.h
		(*p)->first = x;
		p = &(*p)->rest;
	}
	*p = NULL;
	va_end(ap);
	return list;
}
T List_append(T list, T tail) {
	//��һ�������ӵ�����һ�����ú�����tail��ֵ��list���1���ڵ��rest�ֶΡ�
	T *p = &list;
	while (*p)
		p = &(*p)->rest;
	*p = tail;
	return list;
}
T List_copy(T list) {
	//���Ʋ������������ظ���
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
	//����һ������first�ֶθ�ֵ��*x���Ƴ���һ���ڵ㣬��󷵻ؽ������
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
	//��ת���������н��˳�򣬷��ؽ������,head���Ƿ�ת������ĵ�һ���ڵ�
	T head = NULL, next;
	for ( ; list; list = next) {
		next = list->rest;
		list->rest = head;
		head = list;
	}
	return head;
}
int List_length(T list) {
	//���ز��������еĽڵ���Ŀ
	int n;
	for (n = 0; list; list = list->rest)
		n++;
	return n;
}
void List_free(T *list) {
	//*list����NULL�����ͷ�*list�����е����нڵ㣬������ΪNULL����
	T next;
	assert(list);
	for ( ; *list; *list = next) {
		next = (*list)->rest;
		FREE(*list);
	}
}
void List_map(T list,
	void apply(void **x, void *cl), void *cl) {
	//��list�����е�ÿ��������applyָ��ĺ�����
	//�ͻ��˿�����list_map������Ӧ�ó����ָ��cl����ָ����������ݸ�*apply�������ڶ�������
	//�������е�ÿ���ڵ㣬����ָ����first�ֶε�ָ���cl��Ϊ����������*apply
	//first���ܻᱻapply�޸�
	assert(apply);
	for ( ; list; list = list->rest)
		apply(&list->first, cl);
}
void **List_toArray(T list, void *end) {
	//����һ�����飬������0-N-1��Χ�ڱ������������N���ڵ�first�ֶ�ֵ��������Ԫ��N������endֵ
	//endͨ����һ��NULLָ�롣��������һ��ָ�������һ��Ԫ�صķ�NULLָ��,��˲���Ҫ����
	int i, n = List_length(list);
	void **array = ALLOC((n + 1)*sizeof (*array));
	for (i = 0; i < n; i++) {
		array[i] = list->first;
		list = list->rest;
	}
	array[i] = end;
	return array;
}

