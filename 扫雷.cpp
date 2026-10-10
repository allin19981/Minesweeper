#define _CRT_SECURE_NO_WARNINGS
#define MINIAUDIO_IMPLEMENTATION
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<easyx.h>
#include<windows.h>
#include"miniaudio.h"
#pragma comment(lib,"winmm.lib")
#define ROW 10
#define COL 10
#define MineNum 10
ma_engine engine;
ma_sound bgm;
ma_sound lose;
ma_sound win;
ma_sound click;
ma_sound boom;
ma_sound flag;
IMAGE imgs[13]; //定义图片资源
void loadResource()
{
	for (int i = 0;i < 13;i++)
	{
		char imPath[50] = { 0 };//获取图片
		sprintf_s(imPath, "./images/%d.png", i);
		loadimage(&imgs[i], imPath);
	}
}
void draw(int map[][COL]);
void show(int map[][COL]);
void init(int map[][COL]);
void mouseMsg(ExMessage* msg, int map[][COL]);
void boomBlank(int map[][COL], int row, int col);
int judge(int map[][COL], int row, int col);
void expolde(int map[][COL]);
int main() 
{
	//创建窗口
	initgraph(350, 350);

	//放音乐
	ma_engine_init(NULL, &engine);
	ma_sound_init_from_file(&engine, "./images/start.mp3", 0, NULL, NULL, &bgm);
	ma_sound_init_from_file(&engine, "./images/click.mp3", 0, NULL, NULL, &click);
	ma_sound_init_from_file(&engine, "./images/win.mp3", 0, NULL, NULL, &win);
	ma_sound_init_from_file(&engine, "./images/lose.mp3", 0, NULL, NULL, &lose);
	ma_sound_init_from_file(&engine, "./images/boom.mp3", 0, NULL, NULL, &boom);
	ma_sound_init_from_file(&engine, "./images/flag.mp3", 0, NULL, NULL, &flag);
	ma_sound_start(&bgm);
	
	//初始化地图
	int map[ROW][COL] = { 0 };
	init(map);
	show(map);
	while (true)
	{
		//处理信息
		ExMessage msg;
		while (peekmessage(&msg, EX_MOUSE))
		{
			switch (msg.message)
			{
				case WM_LBUTTONDOWN://鼠标左键
				case WM_RBUTTONDOWN://鼠标右键
					mouseMsg(&msg, map);
					int ret = judge(map, msg.y / 35, msg.x / 35);//胜利与失败判断
					if (ret == -1)
					{
						draw(map);
						ma_sound_start(&boom);
						Sleep(200);
						expolde(map);
						draw(map);
						ma_sound_start(&lose);
						int select = MessageBox(GetHWnd(), "你失败了\n","要再来一把吗？\n" ,MB_OKCANCEL);
						if (select == IDOK)//再来一把
						{
							init(map);
						}
					}
					else if (ret == 1)
					{
						ma_sound_start(&win);
						Sleep(1000);
						int select = MessageBox(GetHWnd(), "你胜利了\n", "要再来一把吗？\n", MB_OKCANCEL);
						if (select == IDOK)//再来一把
						{
							init(map);
						}
					}
					break;
			}
		}
		draw(map);
	}
	(void)getchar();
	return 0;
}

//输出雷与数字
void show(int map[][COL])
{
	for (int i = 0;i < ROW;i++)
	{
		for (int k = 0;k < COL;k++)
		{
			printf("%2d ", map[i][k]);
		}
		printf("\n");
	}
}

//初始化
void init(int map[][COL])
{
	loadResource();
	//设置随机数
	srand((unsigned)time(NULL));
	memset(map, 0, sizeof(int) * ROW * COL);
	for (int i = 0;i < MineNum ;)
	{
		int r = rand() % ROW;
		int c = rand() % COL;
		//设置十个雷，用1表示
		if (map[r][c] == 0)
		{
			map[r][c] = -1;
			i++;
		}
	}
	//雷周围加1，雷除外
	for (int i = 0;i < ROW;i++)
	{
		for (int k = 0;k < COL;k++)
		{
			if (map[i][k] == -1)
			{
				for (int r = i - 1;r <= i + 1;r++)
				{
					for (int c = k - 1;c <= k + 1;c++)
					{
						if ((r >= 0 && r < ROW && c >= 0 && c< COL) && map[r][c] != -1)
						{
							map[r][c]++;
						}
					}
				}
			}
		}
	}
	//盖住格子
	for (int i = 0;i < ROW;i++)
	{
		for (int k = 0;k < COL;k++)
		{
			map[i][k] += 20;
		}
	}
}

//绘制
void draw(int map[][COL])
{
	for (int i = 0;i < ROW;i++)
	{
		for (int k = 0;k < COL;k++)
		{
			if (map[i][k] >= 0 && map[i][k] <= 8)
			{
				putimage(k * 35, i * 35, &imgs[map[i][k]]);//贴数字
			}
			else if (map[i][k] == -1)
			{
				putimage(k * 35, i * 35, &imgs[9]);//贴雷
			}
			else if (map[i][k] >= 19 && map[i][k] <= 28)
			{
				putimage(k * 35, i * 35, &imgs[10]);
			}
			else if (map[i][k] >= 39)
			{
				putimage(k * 35, i * 35, &imgs[11]);
			}
			else if (map[i][k] == -2)
			{
				putimage(k * 35, i * 35, &imgs[12]);
			}
		}
	}
}

//鼠标操作函数
void mouseMsg(ExMessage* msg,int map[][COL])
{
	//根据鼠标点击的位置找出数组
	int c = msg->x / 35;
	int r = msg->y / 35;
	int safe = 0;
	//左键打开
	if (msg->message == WM_LBUTTONDOWN)
	{
		if (map[r][c] >= 19 && map[r][c] <= 28)//让重复点击无效
		{
			ma_sound_seek_to_pcm_frame(&click, 0);
			map[r][c] -= 20;
			ma_sound_start(&click);
			boomBlank(map, r, c);
		}
	}

	//右键标记
	else if (msg->message == WM_RBUTTONDOWN)
	{
		if (map[r][c] >= 19 && map[r][c] <= 28)
		{
			map[r][c] += 20;
			ma_sound_start(&flag);
		}
		else if (map[r][c] >= 39)
		{
			map[r][c] -= 20;
			ma_sound_start(&flag);
		}
	}
}

//点击空白格子，爆开函数
void boomBlank(int map[][COL], int row, int col)
{
	if (map[row][col] == 0)
	{
		for (int r = row - 1;r <= row + 1;r++)
		{
			for (int c = col - 1;c <= col + 1;c++)
			{
				if (r >= 0 && r < ROW && c >= 0 && c <= COL//没出界
					&& map[r][c] > 19 && map[r][c] <= 28)//没打开
				{
					map[r][c] -= 20;
					boomBlank(map, r, c);
				}
			}
		}
	}
}

//判断是否胜利
int judge(int map[][COL], int row, int col)
{
	if (map[row][col] == -1)
	{
		return -1;
	}
	//统计打开格子的数量
	int cnt = 0;
	for (int i = 0;i < ROW;i++)
	{
		for (int k = 0;k < COL;k++)
		{
			if (map[i][k] >= 0 && map[i][k] <= 8)
			{
				cnt++;
			}
		}
	}
	if (ROW * COL - MineNum == cnt)
	{
		return 1;//最后剩余10个格子就胜利
	}
	return 0;//不满足判断条件就无事发生
}

//让所有炸弹爆炸
void expolde(int map[][COL])
{
	for (int i = 0;i < COL;i++)
	{
		for (int k = 0;k < ROW;k++)
		{
			if (map[i][k] == 19 || map[i][k] == 39 || map[i][k] == -1)
			{
				map[i][k] = -2;
			}
		}
	}
}