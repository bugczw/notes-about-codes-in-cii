static char rcsid[] = "$Id$";
//checking implementation，实现导出的各个函数，会捕获内存管理描述的各种内存错误，并将其作为已检查的运行时错误报告
//这种内存管理方式，通过两种链表来管理，一种是空闲链表，用于分配；另外一种是link，放置到htab的哈希数组中，用于查找和释放。
//优点是使用则只需要负责申请，其他一概不管，管理由memchk来负责；
//缺点是会形成很多大小等于align的内存节点，不能用于再分配，没有回收机制会造成浪费。
//在频繁申请释放的极端情况下，会消耗大量的内存，系统内存吃紧，而freelist上缺存在大量的align大小的内存空间节点不能回收使用；
//在习题5.5中指出实现一种合并相邻空闲块机制，能够回收这样的内存资源。
//分配函数不返回同一地址两次？？？？？通过不释放任何内存块来实现
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "except.h"
#include "mem.h"
union align {
#ifdef MAXALIGN
	char pad[MAXALIGN];
#else
	int i;
	long l;
	long *lp;
	void *p;
	void (*fp)(void);
	float f;
	double d;
	long double ld;
#endif
};
//基本类型的联合体，32位系统中sizeof的大小是12，即long double的长度是12，主要用于做对齐，确保任何类型的数据都可以保存在Mem_alloc返回的块中，
//Mem_alloc返回的内存大小以align的长度12位最小单位，返回n*12字节的内存，，这样就可以保证任意指针类型都可以使用返回的内存空间；


#define hash(p, t) (((unsigned long)(p)>>3) & \
	(sizeof (t)/sizeof ((t)[0])-1))
//hash表的hash值计算方式，此处的处理方式是p的值右移3位再与上t的大小减一，
//按位取与，控制在N=sizeof(t)/sizeof((t)[0]（计算数组大小）常用方法，这样hash序号值控制在0到(N-1)之间，包括N-1


#define NDESCRIPTORS 512
#define NALLOC ((4096 + sizeof (union align) - 1)/ \
	(sizeof (union align)))*(sizeof (union align))
const Except_T Mem_Failed = { "Allocation failed" };
static struct descriptor {
	struct descriptor *free;
	//descriptor描述符结构，free空闲指针，如果非NULL，则其指代的节点是空闲的，
	//free=NULL,内存耗尽，即ptr挂载的内存块已用，同一内存块可以在不同地址被许多descriptor所指向
	//以htab表头的link普通链表，其中的空闲链表通过free字段与freelist建立一个环形链表，其free字段指向链表的第一个描述符
	struct descriptor *link;
	//link普通链表，挂载htab[n]上，
	//实现箭头是link，严格的以htab[n]为头，hash值相同的内存空间划分在相同链表中。 
	//块描述符的链表，这些块的散列到htab中的同一索引
	const void *ptr;//块的地址，在代码其他地方分配
	long size;//块的长度
	const char *file;
	int line;//该块的坐标，客户程序调用相关分配函数的源代码所处的位置（参数传递给分配函数）
} *htab[2048];
//htab管理内存的hash数组，支持2048个链表结构，
//htab本身是一个descriptor的指针数组，形成了空闲块链表，该链表的头是空descriptor
 


static struct descriptor freelist = { &freelist };
//freelist空闲内存节点链表，静态变量，所有未占用的空闲内存单元都挂载此环形链表中，
//初始化后freelist->free的值为自身的地址，其他值为NULL或者0？？？？？？？？
static struct descriptor *find(const void *ptr) {
	struct descriptor *bp = htab[hash(ptr, htab)];
	while (bp && bp->ptr != ptr)
		bp = bp->link;
	return bp;
}
//输入ptr地址的hash key值找到对应的链表头，通过link搜索找到ptr的指针，返回该节点
void Mem_free(void *ptr, const char *file, int line) {
	if (ptr) {
		struct descriptor *bp;
		if (((unsigned long)ptr)%(sizeof (union align)) != 0
				//严格对齐值倍数的指针才是有效指针
		|| (bp = find(ptr)) == NULL || bp->free)
			Except_raise(&Assert_Failed, file, line);
		bp->free = freelist.free;
		freelist.free = bp;
	}
}
//Mem_free不是真正的将ptr的内存空间free掉，而是将其加入到freelist链表中，以便重新使用。
void *Mem_resize(void *ptr, long nbytes,
	const char *file, int line) {
	struct descriptor *bp;
	void *newptr;
	assert(ptr);
	assert(nbytes > 0);
	if (((unsigned long)ptr)%(sizeof (union align)) != 0
	|| (bp = find(ptr)) == NULL || bp->free)
		Except_raise(&Assert_Failed, file, line);
	newptr = Mem_alloc(nbytes, file, line);
	memcpy(newptr, ptr,
		nbytes < bp->size ? nbytes : bp->size);
	//将ptr中值赋给newptr，并且赋值其中内存
	Mem_free(ptr, file, line);
	return newptr;
}
//Mem_resize 修改内存空间大小，
//重新申请一段内存空间，将原来ptr内存空间的数据拷贝到新申请的newptr中，然后Mem_free释放ptr所在的bp节点。
void *Mem_calloc(long count, long nbytes,
	const char *file, int line) {
	void *ptr;
	assert(count > 0);
	assert(nbytes > 0);
	ptr = Mem_alloc(count*nbytes, file, line);
	memset(ptr, '\0', count*nbytes);
	return ptr;
}
//批量申请count个nbytes大小的内存空间，然后统一初始化为’\0’。
static struct descriptor *dalloc(void *ptr, long size,
	const char *file, int line) {
	//函数作用:分配初始化并返回一个描述符，自由malloc分配的包含512个描述符的内存块
	//NDESCRIPTORS=512
	static struct descriptor *avail;
	static int nleft;//通过静态变量nleft给这个内存块计算空的容量
	if (nleft <= 0) {
		avail = malloc(NDESCRIPTORS*sizeof (*avail));
		if (avail == NULL)
			return NULL;
		nleft = NDESCRIPTORS;
	}
	avail->ptr  = ptr;
	avail->size = size;
	avail->file = file;
	avail->line = line;
	avail->free = avail->link = NULL;
	nleft--;
	return avail++;
}
//dalloc多了一个步骤，将Mem_alloc申请到的ptr通过avail数组管理起来，所以链表包括freelist和htab链表数组中的单元都是avail指针结构，
//为了减少描述符被破坏的可能性，Mem_free操作的对象就是avail中的元素。


void *Mem_alloc(long nbytes, const char *file, int line){
	struct descriptor *bp;
	void *ptr;
	assert(nbytes > 0);
	nbytes = ((nbytes + sizeof (union align) - 1)/
		(sizeof (union align)))*(sizeof (union align));
	//保证申请的nbytes最小为sizeof(union align)，此时nbytes=1，32位系统为12，且为12的倍数，
	for (bp = freelist.free; bp; bp = bp->free) {
		if (bp->size > nbytes) {
			bp->size -= nbytes;
			ptr = (char *)bp->ptr + bp->size;
			//注意bp->ptr是内存块地址
			if ((bp = dalloc(ptr, nbytes, file, line)) != NULL) {
				unsigned h = hash(ptr, htab);
				bp->link = htab[h];
				htab[h] = bp;
				return ptr;
				//如果没有找到，则bp=bp->free，查找下一个free节点，
				//直到bp=NULL(理论上是不会出现的)，退出循环，申请内存失败；
			} else
				{
					if (file == NULL)
						RAISE(Mem_Failed);
					else
						Except_raise(&Mem_Failed, file, line);
				}
		}
		if (bp == &freelist) {
			//或者是bp=&freelist，即在环形链表中从freelist开始找了一圈，又回到freelist都没有合适大小的空闲内存可以用
			//向系统重新申请新的足够大的内存单元来分配。新的newptr同样作为空间内存加入到freelist中，挂载到freelist.free上，
			//在下一个循环是bp=newptr，因为bp->size=nbytes+NALLOC > nbytes， 进入执行内存分配。
			struct descriptor *newptr;
			if ((ptr = malloc(nbytes + NALLOC)) == NULL
			||  (newptr = dalloc(ptr, nbytes + NALLOC,
					__FILE__, __LINE__)) == NULL)
				//和前面一样也是保证大小对齐到align的大小，这样种方式处理方式可以用在其他场合，
				//满足对齐要求（12的整数倍），分配的大小又大于且接近于4096
				{
					if (file == NULL)
						RAISE(Mem_Failed);
					else
						Except_raise(&Mem_Failed, file, line);
				}
			newptr->free = freelist.free;
			freelist.free = newptr;
		}
	}
	//基本思路是从freelist.free开始，循环查找空闲链表中的空闲单元，
	//找到第一个大小合适的bp->size > nbytes的就从bp->ptr里面划分出nbytes大小给ptr，ptr通过dalloc纳入到avail数组中，
	//返回的bp节点纳入到htab表中，在dalloc中会将此节点的free赋值NULL，表示已占用非空闲。
	//	建立了一个新的描述符指向该内存块的某一位置，但该描述符的free为0

	assert(0);
	return NULL;
}
