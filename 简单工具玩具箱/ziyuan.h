#pragma once
// 资源释放示例.cpp : 定义控制台应用程序的入口点。
//


#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "resource.h"
#include "releaseHelper.h"
#include <windows.h>    
#include <tlhelp32.h> 
#include <string>
#pragma warning(disable:4996)

using namespace std;
BOOL FindProcess1(string strProc)
{
	int i = 0;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		cout << "创建进程快照句柄失败!" << endl;
		return false;
	}
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		//这里还是使用stricmp函数最方便，否则还要统一转换成大写或者小写
		if (0 == stricmp(strProc.c_str(), pe32.szExeFile))
		{
			i += 1;  //找到    
		}
		bMore = ::Process32Next(hProcessSnap, &pe32);
	}
	if (i >= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}bool ifFindProcess(bool a, string b) {


	if (FindProcess1(b.c_str()))
	{

	}
	else
	{
		if (a) {
			string d = "del ";
			d = d + b;
			system(d.c_str());
			return true;
		}
		return true;
	}
}
#define PAUSE cout<<"Please Entry Any Code..."<<endl;getchar();

void ziyuan(string type,unsigned long id,string name, string path,bool del,int Sleep_del=0,int Sleep_load=0,int load=0) {
	CReleaseDLL releasehelper;
	bool blRes;
	string b = path+name;
	//blRes = releasehelper.FreeResFile(IDR_EXE1, "EXE", "Terminator-无害.exe"); Project2.exe
	blRes = releasehelper.FreeResFile(id, type.c_str(), b.c_str());
	//blRes = releasehelper.FreeResFile(IDR_RESTXT3, "RESTXT", "521.txt");

	if (blRes)
	{
		//cout << "文件释放成功" << endl;
		string s = "start ";
		b = s + b;
		system(b.c_str());
		b = path + name;
		system("cls"); //清屏
		for (int i = 0; i <= load; ++i) {
			printProgressBar(i,load/2);
			this_thread::sleep_for(chrono::milliseconds(Sleep_load)); // 停顿100毫秒
			
		}
		Sleep(Sleep_del);
		if (1) {

		}
		while (true)
		{
			if (ifFindProcess(del, b) == true) {
				break;
			}
		}
	}
	else
	{
		string s1 = "打开失败：";
		s1 = s1 + b;
		if (MessageBox(NULL, "文件打开失败", s1.c_str(), MB_RETRYCANCEL | MB_ICONHAND) == IDRETRY) {
			ziyuan(type, id,name,path,del);
		}
		else {
			return;
		}
	}


}
