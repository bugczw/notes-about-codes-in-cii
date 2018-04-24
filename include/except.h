/* $Id$ */
/*
代码模块：
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
或者
TRY
  S
FINALLY
  S1
END_TRY
第二种等价于
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
// 具体的错误原因或者错误标志，用于捕获错误时进行比对，const char* 字符串

typedef struct T {
	const char *reason;
} T;//一个描述异常信息的字符串，当发生异常处理时输出该字符串
typedef struct Except_Frame Except_Frame;
struct Except_Frame {
	Except_Frame *prev;
	//  *prev,异常帧
	//一个逆向的单向链表，通过链表的结尾单元来添加和删除；
	jmp_buf env;
	// env是上下文环境变量，即longjmp跳转的寻址目标；
	const char *file;
	int line;
	const T *exception;
	// const char *file 出错的文件，int line出错的行,const T*exception出错的具体信息，
	//后面三个变量是根据需要来设计的，你也可以有自己的变量设计，
};
//相当于结构体typedef struct Except_Frame{}Except_Frame;指向异常栈顶端的异常帧

//用枚举类型来定义程序执行中的错误处理(跳转)标志的几种状态，
//Except_entered 必须为0，它等于第一次调用setjmp的返回值
//Except_raised 表示错误产生，即执行过程中出现了错误
//Except_handled 表示捕获的错误已处理
//Except_finalized 表示异常处理结束
enum { Except_entered=0, Except_raised,
       Except_handled,   Except_finalized };
  
  // 外部定义的栈顶结构体，具体在except.c中定义
extern Except_Frame *Except_stack;
extern const Except_T Assert_Failed;

// 错误捕获函数，包含longjmp的调用，程序执行过程中出现异常，需要调用此函数来触发异常
void Except_raise(const T *e, const char *file,int line);

//考虑到了WIN32平台
#ifdef WIN32
#include <windows.h>

extern int Except_index;
extern void Except_init(void);
extern void Except_push(Except_Frame *fp);
extern void Except_pop(void);
#endif
#ifdef WIN32
/* $Id$ */
//实现函数里面地址，如文件坐标（行列数）直接通过参数传入
// 捕获异常的封装宏定义，加入了__FILE__和__LINE__，allocate中可以使用此函数替代
#define RAISE(e) Except_raise(&(e), __FILE__, __LINE__)

// 如果RAISE捕获的异常未被处理，则执行RERAISE再次处理，直到异常栈Except_stack的栈顶为空NULL
// 宏定义中明确使用了Except_frame变量，所以必须与下面的#define TRY等一起使用，
// 否则程序报错：Except_frame undefined
#define RERAISE Except_raise(Except_frame.exception, \
	Except_frame.file, Except_frame.line)//可以输出变量的异常坐标

#define RETURN switch (Except_pop(),0) default: return
//从左往右执行，先执行出栈操作，执行0的switch判断，
//书中指出由于TRY-ELSE-END_TRY等结构是宏定义，如果直接使用return编译会报错，所以需要进行封装 

// 通过宏定义的方式实现TRY EXCEPT(x) ELSE FINALLY END_TRY的自定义关键字
//为了TRY EXCEPT(e) ELSE FINALLY 和END_TRY能够组合，这些定义总是将if的两个{}分开，


#define TRY do { \
 //TRY语句将一个新的Except_Frame压入异常栈，并调用setjmp
	volatile int Except_flag; \
   //定义异常状态标志 取值范围为之前enum枚举类型定义的值，此处定义为volatile 自动变量，告诉优化器不优化，此变量会被频繁修改，每次都要重新获取值，不能用优化过后的值。
	Except_Frame Except_frame; \
	//创建新的异常结构体变量 
	if (Except_index == -1) \
		Except_init(); \
	Except_push(&Except_frame);  \
	//将新建的异常结构体变量设置为栈顶 
	Except_flag = setjmp(Except_frame.env); \
	//设置跳转点，将返回的结果赋值给Except_flag，(具体上下文的环境属性保存在Except_frame.env中) 
	if (Except_flag == Except_entered) {
    //第一个if分支，第一次执行setjmp时返回的是0，因此条件成立，进入执行用户程序，书中指代为S; 
	//S执行如果无异常则需要跳转到FINALLY(如果有)或者END_TRY, 如果S执行有异常需要调用RAISE(e)跳转回setjmp，根据返回值和异常信息判断进行不同的else 分支。
#define EXCEPT(e) \
	 //测试该帧的exception字段来确定应用那个处理程序
		if (Except_flag == Except_entered) Except_pop(); \
	  //如果类型是TRY-EXCEPT，那么TRY S执行后无异常，则执行此行代码 无错误产生，将此前压栈的异常结构体变量弹出丢弃；
	  //如果有异常，那么程序从S调回到TRY中的1处，在S中longjmp会将setjmp的返回值设置为Except_raised
	} else if (Except_frame.exception == &(e)) { \
	//若Except_flag为Except_raised，判断Except_frame.exception与给定的e错误信息是否一致
	//不一致程序跳过进入下一个判断，可能是EXCEPT(e) 或者ELSE 或FINALLY 或 END_TRY
		Except_flag = Except_handled;
		//如果上面的判断错误一致，则进入到“{”里面执行，将Except_flag 设置为Except_handled
#define ELSE \
		if (Except_flag == Except_entered) Except_pop(); \
		  //如果是TRY-ELSE结构，TRY S ELSE S1则此处和EXCEPT(e)作用一致，
		  //如果TRY S的 S执行无错误，则执行此行代码，将栈顶异常弹出丢弃，否则将执行else 后的代码，
	} else { \
		Except_flag = Except_handled;
		//Except_flag设置为Except_handled状态，表示异常已捕获且处理。执行S1程序
#define FINALLY \
	 //执行其清理代码，并再次引发弹出的异常帧中的储存异常
		if (Except_flag == Except_entered) Except_pop(); \
		  //执行到此处证明上面如果未产生异常，则异常栈顶弹出并丢弃 
	} { \
		if (Except_flag == Except_entered) \
			Except_flag = Except_finalized;
#define END_TRY \
		if (Except_flag == Except_entered) Except_pop(); \
		} if (Except_flag == Except_raised) RERAISE; \
		  //如果条件成立，证明之前的异常没有被处理，此处通过RERAISE再次引发，
		  //如果处理不了，最后会在RERAISE中异常退出，本书中的例子使用的是abort函数退出.
} while (0)
#else
//对应#ifdef WIN32，如果定义了 WIN32
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
