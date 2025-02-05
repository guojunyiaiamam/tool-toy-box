#pragma once
#include <iostream>
#include <windows.h>
using namespace std;
void hideCursor(bool hide) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	GetConsoleCursorInfo(consoleHandle, &info);
	info.bVisible = hide;
	SetConsoleCursorInfo(consoleHandle, &info);
}
void printProgressBar(int percent,int barWidth=50) {
	cout << "[";
	barWidth; // 进度条的宽度
	for (int pos = 0; pos < barWidth; ++pos) {
		if (pos < (percent / 2)) {
			cout << "■";
		}
		else if (pos == (percent / 2)) {
			cout << ">>";
		}
		else {
			cout << " ";
		}
	}
	cout << "] " << percent << " %\r";
	cout.flush();
}