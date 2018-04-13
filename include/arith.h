/* $Id$ */
/*
Arith_div，Arith_mod统一向数轴的左侧进行舍入。
如果x/y是可表示的，则(x/y)*y+x%y=x成立
*/
extern int Arith_max(int x, int y);
extern int Arith_min(int x, int y);
extern int Arith_div(int x, int y);
extern int Arith_mod(int x, int y);
extern int Arith_ceiling(int x, int y);
extern int Arith_floor  (int x, int y);
