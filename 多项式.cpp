#include<stdio.h>
#include<stdlib.h>
#include<iostream>

#define ListsizeMax 100//��ʼ�ռ������
#define Listtncrement 10//�ռ��������
using namespace std;

typedef struct dxs {//����ʽ���Ա�Ľṹ��
	int* x;//��ַ
	int length;//��ǰ�ĳ���
	int listsize;//��ǰ���洢����
}DXS;

int InitDXS(DXS& L, int s) {//��ʼ��4�ζ���ʽ 
	L.x = (int*)malloc(ListsizeMax * sizeof(int));
	L.length = 5;
	L.listsize = 100;
	return 1;
}

void CreatDXS(DXS& L) {//��ʼ������ʽԪ�� 
	for (int i = 0; i < L.length; i++) {
		cout << "�������Ϊ" << i << "�����ϵ����" << endl;
		cin >> L.x[i];
	}
}

DXS DXSadd(DXS s1, DXS s2) {//�ӷ� 
	DXS sum;
	InitDXS(sum, 5);
	for (int i = 0; i < sum.length; i++) {
		sum.x[i] = s1.x[i] + s2.x[i];
	}
	return sum;
}

DXS DXSjian(DXS s1, DXS s2) {//���� 
	DXS sum;
	InitDXS(sum, 5);
	for (int i = 0; i < sum.length; i++) {
		sum.x[i] = s1.x[i] - s2.x[i];
	}
	return sum;
}

DXS DXSCF(DXS s1, DXS s2) {//�˷� 
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
	//s2Ϊ2�ζ���ʽ
	//s1Ϊ4�ζ���ʽ
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

	cout << "��ʼ��������ʽ:" << endl;
	CreatDXS(s1);
	cout << "��һ������ʽΪ��" << endl;
	cout << s1.x[4] << "x^4+" << s1.x[3] << "x^3+" << s1.x[2]
		<< "x^2+" << s1.x[1] << "x+" << s1.x[0] << "=0" << endl;
	cout << "�ڶ�������ʽΪ��" << endl;
	CreatDXS(s2);
	cout << s2.x[4] << "x^4+" << s2.x[3] << "x^3+" << s2.x[2]
		<< "x^2+" << s2.x[1] << "x+" << s2.x[0] << "=0" << endl;

	DXS sum;
	InitDXS(sum, 5);
	sum = DXSadd(s1, s2);
	cout << "����ʽ�ĺ�Ϊ��" << endl;
	cout << sum.x[4] << "x^4+" << sum.x[3] << "x^3+" << sum.x[2]
		<< "x^2+" << sum.x[1] << "x+(" << sum.x[0] << ")=0" << endl;

	sum = DXSjian(s1, s2);
	cout << "����ʽ�Ĳ�Ϊ��" << endl;
	cout << sum.x[4] << "x^4+" << sum.x[3] << "x^3+" << sum.x[2]
		<< "x^2+" << sum.x[1] << "x+(" << sum.x[0] << ")=0" << endl;

	sum = DXSCF(s1, s2);
	cout << "����ʽ�Ļ�Ϊ��" << endl;
	cout << sum.x[8] << "x^8+" << sum.x[7] << "x^7+" << sum.x[6] << "x^6+" << sum.x[5] << "x^5+" << sum.x[4] << "x^4+" << sum.x[3] << "x^3+" << sum.x[2]
		<< "x^2+" << sum.x[1] << "x+(" << sum.x[0] << ")=0" << endl;
	return 0;
}
