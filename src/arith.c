static char rcsid[] = "$Id$";
#include "arith.h"
int Arith_max(int x, int y) {
	return x > y ? x : y;
}
int Arith_min(int x, int y) {
	return x > y ? y : x;
}
//Arith_min和Arith_max返回其整型参数中的最小值和最大值.

int Arith_div(int x, int y) {
	if (-13/5 == -2
	&&	(x < 0) != (y < 0) && x%y != 0)
		return x/y - 1;
	else
		return x/y;
}
//Arith_div返回y除以x得到的商
//Arith_div 中如果x与y同号，则存在机器之间的差异化，
//若不同号，则存在相应的除法向零舍入和向数轴左侧舍入两种情况，
//此处统一处置为向数轴左侧进行舍入

//Arith_mod返回相应的余数。
//当x与y同号的时候，Arith_div(x,y)等价于x/y，Arith_mod(x,y)等价于x%y
//当x与y的符号不同的时候，C的内嵌操作的返回值就取决于具体的实现：
//如果-13/5=-2，-13%5=-3，如果-13/5=-3，-13%5=2
int Arith_mod(int x, int y) {
	//标准库函数总是向零取整，因此div(-13,2)=-2，Arith_div和Arith_mod的语义同样定义好了：它们总是趋近数轴的左侧取整，因此Arith_div(-13,5）=-3，Arith_div(x,y)是不超过实数z的最大整数，其中z满足z*y=x。
    //Arith_mod(x,y)被定义为x-y*Arith_div(x,y)。因此Arith_mod(-13,5)=-13-5*(-3)=2
	if (-13/5 == -2
	&&	(x < 0) != (y < 0) && x%y != 0)
		return x%y + y;
	else
		return x%y;
}
int Arith_floor(int x, int y) {
	//Arith_floor(x,y)返回不超过实数商x/y的最大整数
	return Arith_div(x, y);
}
int Arith_ceiling(int x, int y) {
	//Arith_ceiling(x,y)返回不小于实数商x/y的最小整数
	return Arith_div(x, y) + (x%y != 0);
}
