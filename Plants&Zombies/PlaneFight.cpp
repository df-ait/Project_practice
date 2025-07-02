#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
//#include <iostream>
//#include <thread>
//防止VS中认为scanf函数不安全，从而报警告
#define _CRT_SECURE_NO_WARNINGS
#define GUAN_QIA_COUNT 3

static const int MAX = 100;//最大毫秒

//采用枚举
enum GameInform {
	Width = 700,			//宽度
	Height = 700,			//高度
	Bullet_num = 30,		//子弹数目
	Bullet_speed = 12,		//子弹移动速度
	Plane_speed = 10,		//飞机移动速度
	Enemy_num = 6,			//敌机数量
	Enemy_speed = 1,		//敌机速度
};

typedef enum {
	ENEMY = 1,
	HERO = 0
}plane_type;

//创建的飞机结构体，包括我方飞机和敌方飞机
typedef struct plane {
	int x = 0;				//飞机横坐标
	int y = 0;				//飞机纵坐标
	int is_alive = 0;		//检查飞机是否存活/是否存在
	int hp = 1000;			//飞机血量
	plane_type type ;		//记录飞机类型
	IMAGE plane_img;		//飞机图片
	int width;				//飞机宽度
	int height;				//飞机高度
};

typedef struct zd {
	int x = 0;				//横坐标
	int y = 0;				//纵坐标
	int is_alive = 0;		//检查是否被使用
	IMAGE zd_img[2];		//飞机图片
	int width;				//飞机宽度
	int height;				//飞机高度
};

plane my_plane;				//我方飞机
zd bullet[Bullet_num];		//子弹
plane enemy[Enemy_num];		//敌方飞机

IMAGE back_img[3];				//存储背景图
IMAGE img_plane[4];			//存储我方飞机图像
IMAGE img_emegy[7];			//存储敌方飞机图像
IMAGE img_bullet[2];		//存储子弹图片

static IMAGE now_bk = back_img[0];		//用于更改飞机图片和背景图片
static IMAGE now_plane = img_plane[1];
int enemyFreqs[GUAN_QIA_COUNT] = { 100,50,25 };//敌机创建频率
bool updata = false;

//导入全部所需图片
void load_image() {
	//背景图片
	loadimage(back_img, "map1.jpg", 700, 700, true);
	loadimage(back_img + 1, "map3.jpg", 700, 700, true);
	//玩家飞机图片
	loadimage(&img_plane[0], "player_1_50.png.");
	loadimage(&img_plane[1], "player_2_75.png");
	loadimage(&img_plane[2], "player_3_50.png");
	loadimage(&img_plane[3], "player_4_50.png");
	//敌机图片
	loadimage(img_emegy, "big_emegy_1.png");//大敌机
	loadimage(img_emegy + 1, "big_emegy_2.png");
	loadimage(img_emegy + 2, "middle_emegy_1.png");//中敌机
	loadimage(img_emegy + 3, "middle_emegy_2.png");
	loadimage(img_emegy + 4, "middle_emegy_3.png");
	loadimage(img_emegy + 5, "small_emegy_1.png");//小敌机
	loadimage(img_emegy + 6, "small_emegy_1.png");
	//子弹图片
	loadimage(img_bullet, "shot_7.png");
	loadimage(img_bullet + 1, "shot_2_10.png");
}

//辅助函数
void drawPNG(int picture_x, int picture_y, IMAGE* picture)//x为载入图片的X坐标，y为Y坐标
{
	DWORD* dst = GetImageBuffer();
	//GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture);//获取picture的显存指针
	int picture_width = picture->getwidth();//获取picture的宽度，EASYX自带
	int picture_height = picture->getheight();//获取picture的高度，EASYX自带
	int graphWidth = getwidth();
	//获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();
	//获取绘图区的高度，EASYX自带
	int dstX = 0;
	//在显存里像素的角标
	//实现透明贴图公式：Cp=αp*FP+(1-αp)*BP，贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width;//在显存里像素的角标
			int sa = ((src[srcX] & 0xff000000) >> 24);//0xAArrggbb;AA是透明度
			int sr = ((src[srcX] & 0xff0000) >> 16);//获取RGB里的R
			int sg = ((src[srcX] & 0xff00) >> 8);
			//G
			int sb = src[srcX] & 0xff;
			//B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <=
				graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth;//在显存里像素的角标
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)
					//公式：Cp = αp * FP + (1 - αp) * BP；αp = sa / 255, FP = sr, BP = dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)
					//αp=sa/255,FP = sg, BP = dg
					| (sb * sa / 255 + db * (255 - sa) / 255);
				//αp=sa/255,FP = sb, BP = db
			}
		}
	}
}

//处理边界情况的辅助函数
void drawPNG2(int x, int y, IMAGE* picture) {
	IMAGE imgTmp;
	if (y < 0) {
		SetWorkingImage(picture);
		getimage(&imgTmp, 0, -y,
			picture->getwidth(), picture->getheight() + y);
		SetWorkingImage();
		y = 0;
		drawPNG(x, 0, &imgTmp);
	}
	else {
		drawPNG(x, y, picture);
	}
}

//计时器
int timer() {
	static unsigned long long lastTime = 0;
	unsigned long long currentTime = GetTickCount();
	//GetTickCount()返回系统启动到现在的毫秒数
	if (lastTime == 0) {
		lastTime = currentTime;
		return 0;
	}
	else {
		int ret = currentTime - lastTime;
		lastTime = currentTime;
		return ret;
	}
}

//初始化游戏
void init_game() {
	initgraph(Width, Height);
	//加载图片
	load_image();
	putimage(0, 0, &back_img[0]);
	//初始化子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].is_alive = 0;
		bullet[i].zd_img[0] = img_bullet[0];
		bullet[i].zd_img[1] = img_bullet[1];
		bullet[i].height = img_bullet[0].getheight();
		bullet[i].width = img_bullet[0].getwidth();
	}
	//初始化敌机
	for (int i = 0; i < Enemy_num; i++) {
		enemy[i].is_alive = 0;
		enemy[i].plane_img = img_emegy[rand()%7];
		enemy[i].height = enemy[i].plane_img.getheight();
		enemy[i].width = enemy[i].plane_img.getwidth();
		enemy[i].type = ENEMY;;
	}
	//初始化我方飞机
	my_plane.height = img_plane[0].getheight();
	my_plane.width = img_plane[0].getwidth();
	my_plane.type = HERO;
	my_plane.is_alive = 1;
	my_plane.x = Width / 2 - my_plane.width / 2;
	my_plane.y = my_plane.height;
}

//创建敌机
void create_emegy(){
	for (int i = 0; i < Enemy_num; i++) {
		if (enemy[i].is_alive = 0) {
			enemy[i].is_alive = 1;
			enemy[i].x = rand() % (Width - enemy[i].width);
			enemy[i].y = -100;
			return;
		}
	}
}

//矩阵重叠判断函数
//设A[x1_1,y1_1,x1_2,y1_2]  B[x2_1, y2_1, x2_2, y2_2].
bool is_overlap(int x1_1, int x1_2, int y1_1, int y1_2, int x2_1, int x2_2, int y2_1, int y2_2) {
	int zx = abs(x1_1 + x1_2 - x2_1 - x2_2);
	int x = abs(x1_1 - x1_2) + abs(x2_1 - x2_2);
	int zy = abs(y1_1 + y1_2 - y2_1 - y2_2);
	int y = abs(y1_1 - y1_2) + abs(y2_1 - y2_2);
	return
		(zx <= x && zy <= y);
}

//检测碰撞函数
void  is_collide() {
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			for (int j = 0; j < Enemy_num; j++) {
				if (enemy[j].is_alive) {
					int zdX1 = bullet[i].x + bullet[i].width/4;
					int zdX2 = zdX1 + bullet[i].width * 0.75;
					int zdY1 = bullet[i].y + bullet[i].height / 4;
					int zdY2 = zdY1 + bullet[i].height * 0.75;
					int emX1 = enemy[j].x + enemy[j].width / 4;
					int emX2 = emX1 + enemy[j].width * 0.75;
					int emY1 = enemy[j].y + enemy[j].height / 4;
					int emY2 = emY1 + enemy[j].height * 0.75;

					if (is_overlap(zdX1, zdX2, zdY1, zdY2, emX1, emX2, emY1, emY2)) {
						bullet[i].is_alive = 0; //子弹死亡
						enemy[j].hp -= 300; //敌机血量减少
					}
				}
			}
		}
	}
}

//让敌机位置改变
void fly() {
	//更新敌机状态
	for (int i = 0; i < Enemy_num; i++) {
		if (enemy[i].is_alive) {
			//更新飞行位置
			enemy[i].y += 4;
			if (enemy[i].y > Height) {
				enemy[i].y = -100;
				enemy[i].is_alive = 0;
				continue;
			}
		}
	}
	//更新子弹状态
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			//bullet[i].x = (bullet[i].x + 1) % 2;
			bullet[i].y -= Bullet_speed;
		}
	}//创建敌机
	static int frameCount = 0;
	frameCount++;
	if (frameCount >= 50) {
		frameCount = 0;
		create_emegy();
	}
	//碰撞监测
	is_collide();
}

//更新飞机位置
void update_plane() {
	//更新英雄飞机
	if (my_plane.is_alive) {
		drawPNG(my_plane.x, my_plane.y, &now_plane);
	}
	//更新敌机
	for (int i = 0; i < Enemy_num; i++) {
		if (enemy[i].is_alive) {
			drawPNG(enemy[i].x, enemy[i].y, &enemy[i].plane_img);
		}
	}
}

//飞机事件
void plane_event() {
	if (_kbhit()) {
		/*IMAGE now_bk = back_img[0];
		IMAGE now_plane = img_plane[0];*/
		switch (toupper(_getch())) {
		case 'A':
			/*输入A，意味着要飞机朝左边移动，则y值不变，飞机x值减小*/
			my_plane.x -= Plane_speed;
			//控制飞机边界情况，不能让飞机飞出游戏边界
			if (my_plane.x < 0)my_plane.x = 0;
			//printf("%c", _getch());
			break;
		case 'S':
			/*输入S，意味着要飞机朝下边移动，则x值不变，飞机y值增大*/
			my_plane.y += Plane_speed;
			if (my_plane.y > 650)my_plane.y = 650;
			break;
		case 'W':
			/*输入W，意味着要飞机朝上边移动，则x值不变，飞机y值减小*/
			my_plane.y -= Plane_speed;
			if (my_plane.y < 0)my_plane.y = 0;
			break;
		case 'D':
			/*输入D，意味着要飞机朝右边移动，则y值不变，飞机x值增大*/
			my_plane.x += Plane_speed;
			if (my_plane.x > 650)my_plane.x = 650;
			break;
		//case 'J':
		//	/*输入J，意味着要飞机要发射子弹攻击敌人*/
		//	create_bullet();
		//	break;
		//case 'C':
		//	/*输入C，意味着要切换地图*/
		//	if (compare_img(now_bk, back_img[0])) {
		//		//如果此时背景就为初始背景，那么按下C应该切换到另外一张背景图
		//		now_bk = back_img[1];
		//	}
		//	else {
		//		//如果此时背景不为初始背景，那么按下C应该切换到初始背景图，下面切换飞机皮肤同理
		//		now_bk = back_img[0];
		//	}
		//	//putimage(0, 0, &now_bk);
		//	break;
		//case 'P':
		//	/*输入P，意味着要切换飞机皮肤*/
		//	if (compare_img(now_plane, img_plane[0])) {
		//		now_plane = img_plane[rand() % 3];
		//	}
		//	else {
		//		now_plane = img_plane[0];
		//	}
		//	//putimage(0, 0, &now_bk);
		//	break;
		case 'Q':
			/*输入Q，意味着要退出游戏*/
			return;
		default:
			break;
		}
	}
	/*Draw(now_plane, now_bk);*/
}

int main() {
	srand(time(NULL));
	init_game();
	int time_count = 0;
	while (1) {
		plane_event();
		time_count += timer();
		if (time_count >= MAX) {
			fly();
			time_count = 0;
			updata = true;
		}
		if (updata) {
			BeginBatchDraw();
			update_plane();
			FlushBatchDraw();
		}
	}
}