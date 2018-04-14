static char rcsid[] = "$Id$";
//checking implementation��ʵ�ֵ����ĸ����������Ჶ���ڴ���������ĸ����ڴ���󣬲�������Ϊ�Ѽ�������ʱ���󱨸�
//�����ڴ����ʽ��ͨ����������������һ���ǿ����������ڷ��䣻����һ����link�����õ�htab�Ĺ�ϣ�����У����ڲ��Һ��ͷš�
//�ŵ���ʹ����ֻ��Ҫ�������룬����һ�Ų��ܣ�������memchk������
//ȱ���ǻ��γɺܶ��С����align���ڴ�ڵ㣬���������ٷ��䣬û�л��ջ��ƻ�����˷ѡ�
//��Ƶ�������ͷŵļ�������£������Ĵ������ڴ棬ϵͳ�ڴ�Խ�����freelist��ȱ���ڴ�����align��С���ڴ�ռ�ڵ㲻�ܻ���ʹ�ã�
//��ϰ��5.5��ָ��ʵ��һ�ֺϲ����ڿ��п���ƣ��ܹ������������ڴ���Դ��
//���亯��������ͬһ��ַ���Σ���������ͨ�����ͷ��κ��ڴ����ʵ��
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
//�������͵������壬32λϵͳ��sizeof�Ĵ�С��12����long double�ĳ�����12����Ҫ���������룬ȷ���κ����͵����ݶ����Ա�����Mem_alloc���صĿ��У�
//Mem_alloc���ص��ڴ��С��align�ĳ���12λ��С��λ������n*12�ֽڵ��ڴ棬�������Ϳ��Ա�֤����ָ�����Ͷ�����ʹ�÷��ص��ڴ�ռ䣻


#define hash(p, t) (((unsigned long)(p)>>3) & \
	(sizeof (t)/sizeof ((t)[0])-1))
//hash���hashֵ���㷽ʽ���˴��Ĵ���ʽ��p��ֵ����3λ������t�Ĵ�С��һ��
//��λȡ�룬������N=sizeof(t)/sizeof((t)[0]�����������С�����÷���������hash���ֵ������0��(N-1)֮�䣬����N-1


#define NDESCRIPTORS 512
#define NALLOC ((4096 + sizeof (union align) - 1)/ \
	(sizeof (union align)))*(sizeof (union align))
const Except_T Mem_Failed = { "Allocation failed" };
static struct descriptor {
	struct descriptor *free;
	//descriptor�������ṹ��free����ָ�룬�����NULL������ָ���Ľڵ��ǿ��еģ�
	//free=NULL,�ڴ�ľ�����ptr���ص��ڴ�����ã�ͬһ�ڴ������ڲ�ͬ��ַ�����descriptor��ָ��
	//��htab��ͷ��link��ͨ�������еĿ�������ͨ��free�ֶ���freelist����һ������������free�ֶ�ָ������ĵ�һ��������
	struct descriptor *link;
	//link��ͨ��������htab[n]�ϣ�
	//ʵ�ּ�ͷ��link���ϸ����htab[n]Ϊͷ��hashֵ��ͬ���ڴ�ռ仮������ͬ�����С� 
	//����������������Щ���ɢ�е�htab�е�ͬһ����
	const void *ptr;//��ĵ�ַ���ڴ��������ط�����
	long size;//��ĳ���
	const char *file;
	int line;//�ÿ�����꣬�ͻ����������ط��亯����Դ����������λ�ã��������ݸ����亯����
} *htab[2048];
//htab�����ڴ��hash���飬֧��2048������ṹ��
//htab������һ��descriptor��ָ�����飬�γ��˿��п������������ͷ�ǿ�descriptor
 


static struct descriptor freelist = { &freelist };
//freelist�����ڴ�ڵ�������̬����������δռ�õĿ����ڴ浥Ԫ�����ش˻��������У�
//��ʼ����freelist->free��ֵΪ����ĵ�ַ������ֵΪNULL����0����������������
static struct descriptor *find(const void *ptr) {
	struct descriptor *bp = htab[hash(ptr, htab)];
	while (bp && bp->ptr != ptr)
		bp = bp->link;
	return bp;
}
//����ptr��ַ��hash keyֵ�ҵ���Ӧ������ͷ��ͨ��link�����ҵ�ptr��ָ�룬���ظýڵ�
void Mem_free(void *ptr, const char *file, int line) {
	if (ptr) {
		struct descriptor *bp;
		if (((unsigned long)ptr)%(sizeof (union align)) != 0
				//�ϸ����ֵ������ָ�������Чָ��
		|| (bp = find(ptr)) == NULL || bp->free)
			Except_raise(&Assert_Failed, file, line);
		bp->free = freelist.free;
		freelist.free = bp;
	}
}
//Mem_free���������Ľ�ptr���ڴ�ռ�free�������ǽ�����뵽freelist�����У��Ա�����ʹ�á�
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
	//��ptr��ֵ����newptr�����Ҹ�ֵ�����ڴ�
	Mem_free(ptr, file, line);
	return newptr;
}
//Mem_resize �޸��ڴ�ռ��С��
//��������һ���ڴ�ռ䣬��ԭ��ptr�ڴ�ռ�����ݿ������������newptr�У�Ȼ��Mem_free�ͷ�ptr���ڵ�bp�ڵ㡣
void *Mem_calloc(long count, long nbytes,
	const char *file, int line) {
	void *ptr;
	assert(count > 0);
	assert(nbytes > 0);
	ptr = Mem_alloc(count*nbytes, file, line);
	memset(ptr, '\0', count*nbytes);
	return ptr;
}
//��������count��nbytes��С���ڴ�ռ䣬Ȼ��ͳһ��ʼ��Ϊ��\0����
static struct descriptor *dalloc(void *ptr, long size,
	const char *file, int line) {
	//��������:�����ʼ��������һ��������������malloc����İ���512�����������ڴ��
	//NDESCRIPTORS=512
	static struct descriptor *avail;
	static int nleft;//ͨ����̬����nleft������ڴ�����յ�����
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
//dalloc����һ�����裬��Mem_alloc���뵽��ptrͨ��avail������������������������freelist��htab���������еĵ�Ԫ����availָ��ṹ��
//Ϊ�˼������������ƻ��Ŀ����ԣ�Mem_free�����Ķ������avail�е�Ԫ�ء�


void *Mem_alloc(long nbytes, const char *file, int line){
	struct descriptor *bp;
	void *ptr;
	assert(nbytes > 0);
	nbytes = ((nbytes + sizeof (union align) - 1)/
		(sizeof (union align)))*(sizeof (union align));
	//��֤�����nbytes��СΪsizeof(union align)����ʱnbytes=1��32λϵͳΪ12����Ϊ12�ı�����
	for (bp = freelist.free; bp; bp = bp->free) {
		if (bp->size > nbytes) {
			bp->size -= nbytes;
			ptr = (char *)bp->ptr + bp->size;
			//ע��bp->ptr���ڴ���ַ
			if ((bp = dalloc(ptr, nbytes, file, line)) != NULL) {
				unsigned h = hash(ptr, htab);
				bp->link = htab[h];
				htab[h] = bp;
				return ptr;
				//���û���ҵ�����bp=bp->free��������һ��free�ڵ㣬
				//ֱ��bp=NULL(�������ǲ�����ֵ�)���˳�ѭ���������ڴ�ʧ�ܣ�
			} else
				{
					if (file == NULL)
						RAISE(Mem_Failed);
					else
						Except_raise(&Mem_Failed, file, line);
				}
		}
		if (bp == &freelist) {
			//������bp=&freelist�����ڻ��������д�freelist��ʼ����һȦ���ֻص�freelist��û�к��ʴ�С�Ŀ����ڴ������
			//��ϵͳ���������µ��㹻����ڴ浥Ԫ�����䡣�µ�newptrͬ����Ϊ�ռ��ڴ���뵽freelist�У����ص�freelist.free�ϣ�
			//����һ��ѭ����bp=newptr����Ϊbp->size=nbytes+NALLOC > nbytes�� ����ִ���ڴ���䡣
			struct descriptor *newptr;
			if ((ptr = malloc(nbytes + NALLOC)) == NULL
			||  (newptr = dalloc(ptr, nbytes + NALLOC,
					__FILE__, __LINE__)) == NULL)
				//��ǰ��һ��Ҳ�Ǳ�֤��С���뵽align�Ĵ�С�������ַ�ʽ����ʽ���������������ϣ�
				//�������Ҫ��12����������������Ĵ�С�ִ����ҽӽ���4096
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
	//����˼·�Ǵ�freelist.free��ʼ��ѭ�����ҿ��������еĿ��е�Ԫ��
	//�ҵ���һ����С���ʵ�bp->size > nbytes�ľʹ�bp->ptr���滮�ֳ�nbytes��С��ptr��ptrͨ��dalloc���뵽avail�����У�
	//���ص�bp�ڵ����뵽htab���У���dalloc�лὫ�˽ڵ��free��ֵNULL����ʾ��ռ�÷ǿ��С�
	//	������һ���µ�������ָ����ڴ���ĳһλ�ã�������������freeΪ0

	assert(0);
	return NULL;
}
