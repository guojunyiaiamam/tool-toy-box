#pragma once
// ��Դ�ͷ�ʾ��.cpp : �������̨Ӧ�ó������ڵ㡣
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
		cout << "�������̿��վ��ʧ��!" << endl;
		return false;
	}
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		//���ﻹ��ʹ��stricmp������㣬����Ҫͳһת���ɴ�д����Сд
		if (0 == stricmp(strProc.c_str(), pe32.szExeFile))
		{
			i += 1;  //�ҵ�    
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
	//blRes = releasehelper.FreeResFile(IDR_EXE1, "EXE", "Terminator-�޺�.exe"); Project2.exe
	blRes = releasehelper.FreeResFile(id, type.c_str(), b.c_str());
	//blRes = releasehelper.FreeResFile(IDR_RESTXT3, "RESTXT", "521.txt");

	if (blRes)
	{
		//cout << "�ļ��ͷųɹ�" << endl;
		string s = "start ";
		b = s + b;
		system(b.c_str());
		b = path + name;
		system("cls"); //����
		for (int i = 0; i <= load; ++i) {
			printProgressBar(i,load/2);
			this_thread::sleep_for(chrono::milliseconds(Sleep_load)); // ͣ��100����
			
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
		string s1 = "��ʧ�ܣ�";
		s1 = s1 + b;
		if (MessageBox(NULL, "�ļ���ʧ��", s1.c_str(), MB_RETRYCANCEL | MB_ICONHAND) == IDRETRY) {
			ziyuan(type, id,name,path,del);
		}
		else {
			return;
		}
	}


}
