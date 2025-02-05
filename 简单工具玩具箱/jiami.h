#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include <fstream>
#include<iostream>
#include <windows.h>		//与opencv的命名空间CV有冲突，不能在一个文件中使用
#include<string>
#include<io.h>
#pragma warning(disable:4996)
using namespace std;
void jiami(int a, char* b[], bool aaa) {
	//char srcFileName[260] = { 0 };
	char dstFileName[260] = { 0 };
	//printf("打开文件:");
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

		//加密
		c ^= 0x130919;//c=c^0x130919; 

		//
		fwrite(&c, 1, 1, fpDst);
	}




	fclose(fpDst);
	fclose(fpSrc);

	string  s = b[a];

	const char* file_path = s.c_str(); // 指定文件路径
	remove(file_path);
	rename("temp.guojunyi", b[a]);//将b.txt重命名为c.txt
	string over = "加密完成:", ss = over + b[a];
	MessageBox(NULL, ss.c_str(), "加密", MB_OK | MB_ICONEXCLAMATION);

}