#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include <fstream>
#include<iostream>
#include <windows.h>		//��opencv�������ռ�CV�г�ͻ��������һ���ļ���ʹ��
#include<string>
#include<io.h>
#pragma warning(disable:4996)
using namespace std;
void jiami(int a, char* b[], bool aaa) {
	//char srcFileName[260] = { 0 };
	char dstFileName[260] = { 0 };
	//printf("���ļ�:");
	//scanf("%s", srcFileName);
	//C
	FILE* fpSrc = fopen(b[a], "rb");
//	cout << b;
	FILE* fpDst = fopen("temp.guojunyi", "wb");
	if (NULL == fpSrc || NULL == fpDst) {
	//	printf("open no\n");
	}
	else {
		//printf("open yes\n");
	}
	int r;
	char c;
	while (true)
	{
		r = fread(&c, 1, 1, fpSrc);
		if (r < 1) {
			break;
		}

		//����
		c ^= 0x130919;//c=c^0x130919; 

		//
		fwrite(&c, 1, 1, fpDst);
	}




	fclose(fpDst);
	fclose(fpSrc);

	string  s = b[a];

	const char* file_path = s.c_str(); // ָ���ļ�·��
	remove(file_path);
	rename("temp.guojunyi", b[a]);//��b.txt������Ϊc.txt
	string over = "�������:", ss = over + b[a];
	MessageBox(NULL, ss.c_str(), "����", MB_OK | MB_ICONEXCLAMATION);

}