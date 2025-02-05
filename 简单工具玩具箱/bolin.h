#include <corecrt_math.h>
#include <sstream>

#define bwidth 1000
#define bheight 1000
using namespace std;
int p[512];
// 混合函数
double fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// 线性插值函数
double lerp(double t, double a, double b) {
    return a + t * (b - a);
}

// 计算梯度点
double grad(int hash, double x, double y) {
    int h = hash & 0xF;
    double u = (h < 8) ? x : y;
    double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : 0);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// 生成柏林噪声
double perlinNoise2D(double x, double y) {
    int X = (int)x & 255;
    int Y = (int)y & 255;
    x -= floor(x);
    y -= floor(y);
    double u = fade(x);
    double v = fade(y);

    int A = p[X] + Y;
    int AA = p[A] & 255;
    int AB = p[A + 1] & 255;
    int B = p[X + 1] + Y;
    int BA = p[B] & 255;
    int BB = p[B + 1] & 255;

    double xa = lerp(u, grad(p[AA], x, y), grad(p[BA], x - 1, y));
    double xb = lerp(u, grad(p[AB], x, y - 1), grad(p[BB], x - 1, y - 1));
    return lerp(v, xa, xb);
}

// 生成随机梯度向量


void setupPerlinNoise() {
    srand(time(NULL));
    for (int i = 0; i < 256; i++) {
        p[i] = i;
    }
    for (int i = 0; i < 256; i++) {
        int temp = p[i];
        int j = rand() % 256;
        p[i] = p[j];
        p[j] = temp;
    }
    for (int i = 0; i < 256; i++) {
        p[256 + i] = p[i];
    }
}

void bolin() {
    ExMessage msg;
    char ch;

    initgraph(bwidth, bheight); // 创建一个640x480的窗口

    setupPerlinNoise(); // 初始化柏林噪声

    for (int y = 0; y < bwidth; y++) {
        for (int x = 0; x < bheight; x++) {
            double n = perlinNoise2D(x / 32.0, y / 32.0); // 调整频率
            n = (n + 1) / 2; // 将噪声值从[-1, 1]映射到[0, 1]
            int color = (int)(n * 255); // 将噪声值转换为颜色值
            putpixel(x, y, RGB(color, color, color)); // 绘制像素
        }
    }

    saveimage(_T("bolin.bmp"));
    ostringstream oss;
    int a = 0;
    while (1) {
        a++;
        oss.str(" ");
        msg = getmessage(EX_KEY);
        if (msg.vkcode == 0x43) {
           // cout << "Clear" << endl;
            setupPerlinNoise(); // 初始化柏林噪声

            for (int y = 0; y < bwidth; y++) {
                for (int x = 0; x < bheight; x++) {
                    double n = perlinNoise2D(x / 32.0, y / 32.0); // 调整频率
                    n = (n + 1) / 2; // 将噪声值从[-1, 1]映射到[0, 1]
                    int color = (int)(n * 255); // 将噪声值转换为颜色值
                    putpixel(x, y, RGB(color, color, color)); // 绘制像素
                }
            }
            oss << "bolin" << a << ".bmp";

            saveimage(oss.str().c_str());
        }
    }
closegraph(); // 关闭窗口
}