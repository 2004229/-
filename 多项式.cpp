#include<stdio.h>
#include<stdlib.h>
#include<iostream>

#define ListsizeMax 100//初始空间分配量
#define Listtncrement 10//空间分配增量
using namespace std;

typedef struct dxs {//多项式线性表的结构体
	int* x;//地址
	int length;//当前的长度
	int listsize;//当前最大存储容量
}DXS;

int InitDXS(DXS& L, int s) {//初始化4次多项式 
	L.x = (int*)malloc(ListsizeMax * sizeof(int));
	L.length = 5;
	L.listsize = 100;
	return 1;
}

void CreatDXS(DXS& L) {//初始化多项式元素 
	for (int i = 0; i < L.length; i++) {
		cout << "输入次数为" << i << "的项的系数：" << endl;
		cin >> L.x[i];
	}
}

DXS DXSadd(DXS s1, DXS s2) {//加法 
	DXS sum;
	InitDXS(sum, 5);
	for (int i = 0; i < sum.length; i++) {
		sum.x[i] = s1.x[i] + s2.x[i];
	}
	return sum;
}

DXS DXSjian(DXS s1, DXS s2) {//减法 
	DXS sum;
	InitDXS(sum, 5);
	for (int i = 0; i < sum.length; i++) {
		sum.x[i] = s1.x[i] - s2.x[i];
	}
	return sum;
}

DXS DXSCF(DXS s1, DXS s2) {//乘法 
	DXS j;
	InitDXS(j, 9);
	
	j.x[0] = s1.x[0] * s2.x[0];
	j.x[1] = s1.x[0] * s2.x[1] + s1.x[1] * s2.x[0];
	j.x[2] = s1.x[0] * s2.x[2] + s1.x[1] * s2.x[1] + s1.x[2] * s2.x[0];
	j.x[3] = s1.x[0] * s2.x[3] + s1.x[1] * s2.x[2] + s1.x[2] * s2.x[1] + s1.x[3] * s2.x[0];
	j.x[4] = s1.x[0] * s2.x[4] + s1.x[1] * s2.x[3] + s1.x[2] * s2.x[2] + s1.x[3] + s2.x[1] + s1.x[4] * s2.x[0];
	j.x[5] = s1.x[1] * s2.x[4] + s1.x[2] * s2.x[3] + s1.x[3] * s2.x[2] + s1.x[4] * s2.x[1];
	j.x[6] = s1.x[2] * s2.x[4] + s1.x[3] * s2.x[3] + s1.x[4] * s2.x[2];
	j.x[7] = s1.x[3] * s2.x[4] + s1.x[4] * s2.x[3];
	j.x[8] = s1.x[4] * s2.x[4];
	
	return j;
}


DXS DXSC(DXS s1, DXS s2) {
	//s2为2次多项式
	//s1为4次多项式
	DXS j;
	InitDXS(j, 3);
	j.x[2] = s1.x[4] / s2.x[2];
	return j;
}

int main(int argc, char* argv[]) {
	DXS s1;
	DXS s2;
	//int i;
	InitDXS(s1, 5);
	InitDXS(s2, 5);

	cout << "开始创建多项式:" << endl;
	CreatDXS(s1);
	cout << "第一个多项式为：" << endl;
	cout << s1.x[4] << "x^4+" << s1.x[3] << "x^3+" << s1.x[2]
		<< "x^2+" << s1.x[1] << "x+" << s1.x[0] << "=0" << endl;
	cout << "第二个多项式为：" << endl;
	CreatDXS(s2);
	cout << s2.x[4] << "x^4+" << s2.x[3] << "x^3+" << s2.x[2]
		<< "x^2+" << s2.x[1] << "x+" << s2.x[0] << "=0" << endl;

	DXS sum;
	InitDXS(sum, 5);
	sum = DXSadd(s1, s2);
	cout << "多项式的和为：" << endl;
	cout << sum.x[4] << "x^4+" << sum.x[3] << "x^3+" << sum.x[2]
		<< "x^2+" << sum.x[1] << "x+(" << sum.x[0] << ")=0" << endl;

	sum = DXSjian(s1, s2);
	cout << "多项式的差为：" << endl;
	cout << sum.x[4] << "x^4+" << sum.x[3] << "x^3+" << sum.x[2]
		<< "x^2+" << sum.x[1] << "x+(" << sum.x[0] << ")=0" << endl;

	sum = DXSCF(s1, s2);
	cout << "多项式的积为：" << endl;
	cout << sum.x[8] << "x^8+" << sum.x[7] << "x^7+" << sum.x[6] << "x^6+" << sum.x[5] << "x^5+" << sum.x[4] << "x^4+" << sum.x[3] << "x^3+" << sum.x[2]
		<< "x^2+" << sum.x[1] << "x+(" << sum.x[0] << ")=0" << endl;
	return 0;
}
