/* $Id$ */
/*
����ģ�飺
TRY
  S
EXCEPT(e1)
  S1
EXCEPT(e2)
  S2
...
EXCEPT(en)
  Sn
ELSE
  S0
END_TRY
����
TRY
  S
FINALLY
  S1
END_TRY
�ڶ��ֵȼ���
TRY 
  S
ELSE 
  S1
  RERAISE;
END_TRY
S1
*/

#ifndef EXCEPT_INCLUDED
#define EXCEPT_INCLUDED
#include <setjmp.h>
#define T Except_T
// ����Ĵ���ԭ����ߴ����־�����ڲ������ʱ���бȶԣ�const char* �ַ���

typedef struct T {
	const char *reason;
} T;//һ�������쳣��Ϣ���ַ������������쳣����ʱ������ַ���
typedef struct Except_Frame Except_Frame;
struct Except_Frame {
	Except_Frame *prev;
	//  *prev,�쳣֡
	//һ������ĵ�������ͨ������Ľ�β��Ԫ����Ӻ�ɾ����
	jmp_buf env;
	// env�������Ļ�����������longjmp��ת��ѰַĿ�ꣻ
	const char *file;
	int line;
	const T *exception;
	// const char *file ������ļ���int line�������,const T*exception����ľ�����Ϣ��
	//�������������Ǹ�����Ҫ����Ƶģ���Ҳ�������Լ��ı�����ƣ�
};
//�൱�ڽṹ��typedef struct Except_Frame{}Except_Frame;ָ���쳣ջ���˵��쳣֡

//��ö���������������ִ���еĴ�����(��ת)��־�ļ���״̬��
//Except_entered ����Ϊ0�������ڵ�һ�ε���setjmp�ķ���ֵ
//Except_raised ��ʾ�����������ִ�й����г����˴���
//Except_handled ��ʾ����Ĵ����Ѵ���
//Except_finalized ��ʾ�쳣�������
enum { Except_entered=0, Except_raised,
       Except_handled,   Except_finalized };
  
  // �ⲿ�����ջ���ṹ�壬������except.c�ж���
extern Except_Frame *Except_stack;
extern const Except_T Assert_Failed;

// ���󲶻���������longjmp�ĵ��ã�����ִ�й����г����쳣����Ҫ���ô˺����������쳣
void Except_raise(const T *e, const char *file,int line);

//���ǵ���WIN32ƽ̨
#ifdef WIN32
#include <windows.h>

extern int Except_index;
extern void Except_init(void);
extern void Except_push(Except_Frame *fp);
extern void Except_pop(void);
#endif
#ifdef WIN32
/* $Id$ */
//ʵ�ֺ��������ַ�����ļ����꣨��������ֱ��ͨ����������
// �����쳣�ķ�װ�궨�壬������__FILE__��__LINE__��allocate�п���ʹ�ô˺������
#define RAISE(e) Except_raise(&(e), __FILE__, __LINE__)

// ���RAISE������쳣δ��������ִ��RERAISE�ٴδ���ֱ���쳣ջExcept_stack��ջ��Ϊ��NULL
// �궨������ȷʹ����Except_frame���������Ա����������#define TRY��һ��ʹ�ã�
// ������򱨴�Except_frame undefined
#define RERAISE Except_raise(Except_frame.exception, \
	Except_frame.file, Except_frame.line)//��������������쳣����

#define RETURN switch (Except_pop(),0) default: return
//��������ִ�У���ִ�г�ջ������ִ��0��switch�жϣ�
//����ָ������TRY-ELSE-END_TRY�Ƚṹ�Ǻ궨�壬���ֱ��ʹ��return����ᱨ��������Ҫ���з�װ 

// ͨ���궨��ķ�ʽʵ��TRY EXCEPT(x) ELSE FINALLY END_TRY���Զ���ؼ���
//Ϊ��TRY EXCEPT(e) ELSE FINALLY ��END_TRY�ܹ���ϣ���Щ�������ǽ�if������{}�ֿ���


#define TRY do { \
 //TRY��佫һ���µ�Except_Frameѹ���쳣ջ��������setjmp
	volatile int Except_flag; \
   //�����쳣״̬��־ ȡֵ��ΧΪ֮ǰenumö�����Ͷ����ֵ���˴�����Ϊvolatile �Զ������������Ż������Ż����˱����ᱻƵ���޸ģ�ÿ�ζ�Ҫ���»�ȡֵ���������Ż������ֵ��
	Except_Frame Except_frame; \
	//�����µ��쳣�ṹ����� 
	if (Except_index == -1) \
		Except_init(); \
	Except_push(&Except_frame);  \
	//���½����쳣�ṹ���������Ϊջ�� 
	Except_flag = setjmp(Except_frame.env); \
	//������ת�㣬�����صĽ����ֵ��Except_flag��(���������ĵĻ������Ա�����Except_frame.env��) 
	if (Except_flag == Except_entered) {
    //��һ��if��֧����һ��ִ��setjmpʱ���ص���0�������������������ִ���û���������ָ��ΪS; 
	//Sִ��������쳣����Ҫ��ת��FINALLY(�����)����END_TRY, ���Sִ�����쳣��Ҫ����RAISE(e)��ת��setjmp�����ݷ���ֵ���쳣��Ϣ�жϽ��в�ͬ��else ��֧��
#define EXCEPT(e) \
	 //���Ը�֡��exception�ֶ���ȷ��Ӧ���Ǹ��������
		if (Except_flag == Except_entered) Except_pop(); \
	  //���������TRY-EXCEPT����ôTRY Sִ�к����쳣����ִ�д��д��� �޴������������ǰѹջ���쳣�ṹ���������������
	  //������쳣����ô�����S���ص�TRY�е�1������S��longjmp�Ὣsetjmp�ķ���ֵ����ΪExcept_raised
	} else if (Except_frame.exception == &(e)) { \
	//��Except_flagΪExcept_raised���ж�Except_frame.exception�������e������Ϣ�Ƿ�һ��
	//��һ�³�������������һ���жϣ�������EXCEPT(e) ����ELSE ��FINALLY �� END_TRY
		Except_flag = Except_handled;
		//���������жϴ���һ�£�����뵽��{������ִ�У���Except_flag ����ΪExcept_handled
#define ELSE \
		if (Except_flag == Except_entered) Except_pop(); \
		  //�����TRY-ELSE�ṹ��TRY S ELSE S1��˴���EXCEPT(e)����һ�£�
		  //���TRY S�� Sִ���޴�����ִ�д��д��룬��ջ���쳣��������������ִ��else ��Ĵ��룬
	} else { \
		Except_flag = Except_handled;
		//Except_flag����ΪExcept_handled״̬����ʾ�쳣�Ѳ����Ҵ���ִ��S1����
#define FINALLY \
	 //ִ����������룬���ٴ������������쳣֡�еĴ����쳣
		if (Except_flag == Except_entered) Except_pop(); \
		  //ִ�е��˴�֤���������δ�����쳣�����쳣ջ������������ 
	} { \
		if (Except_flag == Except_entered) \
			Except_flag = Except_finalized;
#define END_TRY \
		if (Except_flag == Except_entered) Except_pop(); \
		} if (Except_flag == Except_raised) RERAISE; \
		  //�������������֤��֮ǰ���쳣û�б������˴�ͨ��RERAISE�ٴ�������
		  //��������ˣ�������RERAISE���쳣�˳��������е�����ʹ�õ���abort�����˳�.
} while (0)
#else
//��Ӧ#ifdef WIN32����������� WIN32
#define RAISE(e) Except_raise(&(e), __FILE__, __LINE__)
#define RERAISE Except_raise(Except_frame.exception, \
	Except_frame.file, Except_frame.line)
#define RETURN switch (Except_stack = Except_stack->prev,0) default: return
#define TRY do { \
	volatile int Except_flag; \
	Except_Frame Except_frame; \
	Except_frame.prev = Except_stack; \
	Except_stack = &Except_frame;  \
	Except_flag = setjmp(Except_frame.env); \
	if (Except_flag == Except_entered) {
#define EXCEPT(e) \
		if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
	} else if (Except_frame.exception == &(e)) { \
		Except_flag = Except_handled;
#define ELSE \
		if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
	} else { \
		Except_flag = Except_handled;
#define FINALLY \
		if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
	} { \
		if (Except_flag == Except_entered) \
			Except_flag = Except_finalized;
#define END_TRY \
		if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
		} if (Except_flag == Except_raised) RERAISE; \
} while (0)
#endif
#undef T
#endif
