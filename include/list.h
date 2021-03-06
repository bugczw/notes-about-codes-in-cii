/* $Id$ */
#ifndef LIST_INCLUDED
#define LIST_INCLUDED
#define T List_T
typedef struct T *T;
//T是一个指向某个T实例的指针
struct T {
	T rest;
	//结构体指针，可以理解为别名，使之指向它某个T实例·
	void *first;
};
extern T      List_append (T list, T tail);
extern T      List_copy   (T list);
extern T      List_list   (void *x, ...);
extern T      List_pop    (T list, void **x);
extern T      List_push   (T list, void *x);
extern T      List_reverse(T list);
extern int    List_length (T list);
extern void   List_free   (T *list);
extern void   List_map    (T list,
	void apply(void **x, void *cl), void *cl);
extern void **List_toArray(T list, void *end);
#undef T
#endif
