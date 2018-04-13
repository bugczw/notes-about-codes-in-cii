static char rcsid[] = "$Id$";
#include "arith.h"
int Arith_max(int x, int y) {
	return x > y ? x : y;
}
int Arith_min(int x, int y) {
	return x > y ? y : x;
}
//Arith_min��Arith_max���������Ͳ����е���Сֵ�����ֵ.

int Arith_div(int x, int y) {
	if (-13/5 == -2
	&&	(x < 0) != (y < 0) && x%y != 0)
		return x/y - 1;
	else
		return x/y;
}
//Arith_div����y����x�õ�����
//Arith_div �����x��yͬ�ţ�����ڻ���֮��Ĳ��컯��
//����ͬ�ţ��������Ӧ�ĳ����������������������������������
//�˴�ͳһ����Ϊ����������������

//Arith_mod������Ӧ��������
//��x��yͬ�ŵ�ʱ��Arith_div(x,y)�ȼ���x/y��Arith_mod(x,y)�ȼ���x%y
//��x��y�ķ��Ų�ͬ��ʱ��C����Ƕ�����ķ���ֵ��ȡ���ھ����ʵ�֣�
//���-13/5=-2��-13%5=-3�����-13/5=-3��-13%5=2
int Arith_mod(int x, int y) {
	//��׼�⺯����������ȡ�������div(-13,2)=-2��Arith_div��Arith_mod������ͬ��������ˣ���������������������ȡ�������Arith_div(-13,5��=-3��Arith_div(x,y)�ǲ�����ʵ��z���������������z����z*y=x��
    //Arith_mod(x,y)������Ϊx-y*Arith_div(x,y)�����Arith_mod(-13,5)=-13-5*(-3)=2
	if (-13/5 == -2
	&&	(x < 0) != (y < 0) && x%y != 0)
		return x%y + y;
	else
		return x%y;
}
int Arith_floor(int x, int y) {
	//Arith_floor(x,y)���ز�����ʵ����x/y���������
	return Arith_div(x, y);
}
int Arith_ceiling(int x, int y) {
	//Arith_ceiling(x,y)���ز�С��ʵ����x/y����С����
	return Arith_div(x, y) + (x%y != 0);
}
