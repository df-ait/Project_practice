#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <tchar.h>//为了写出同时兼容ANSI(多字节字符集)还有Unicode编码的代码，为了引入TCHAR类型
#include <graphics.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") //链接winmm.lib库文件

//防止VS中认为scanf函数不安全，从而报警告
#define _CRT_SECURE_NO_WARNINGS
#define GUAN_QIA_COUNT 3
#define boom_pic_num 6
#define plane_total_hp  3000

static const int MAX = 100;//最大毫秒

//采用枚举
enum GameInform {
	Width = 700,			//宽度
	Height = 700,			//高度
	Bullet_num = 30,		//子弹数目
	Bullet_speed = 7,		//我方子弹移动速度
	Plane_speed = 5,		//飞机移动速度
	Enemy_bullet_speed = 3,	//敌方子弹速度
	Enemy_num = 8,			//敌机数量
	Enemy_speed = 2,		//敌机速度
};

typedef enum {
	BIG_ENEMY = 3,
	MIDDLE_ENEMY = 2,
	SMALL_ENEMY = 1,
	HERO = 0,
}plane_type;

enum GameState {
	MAIN_MENU = 0,	//主菜单
	PLAYING,		//游戏进行中
	SINGLE_PLAYER,	//单人游戏
	DOUBLE_PLAYER,	//双人游戏
	GAME_OVER,		//游戏结束
};

GameState game_status = MAIN_MENU;	//游戏界面状态
bool is_exit = false;	//退出游戏标志
bool is_music_playing = true;	//音乐播放标志
bool auto_shoot = false;	//自动射击标志,是否进入自动射击模式

typedef struct BOOM
{
	IMAGE  boom_png[boom_pic_num] ;	//爆炸特效图片
	int all_frame = 0;				//总帧数
	int current_frame = 0;			//当前帧数
	unsigned int last_time = 0;	//上一帧时间
	bool is_playing = false;	//是否正在播放
	int width = 0;				//每一帧的宽度
	int height = 0;				//每一帧的高度
	int frame_dalay = 100;		//间隔帧数100ms
	int x;						//绘制爆炸特效的坐标
	int y;
};

BOOM png_boom;		//定义爆炸特效

typedef struct zd {
	int x = 0;				//横坐标
	int y = 0;				//纵坐标
	int is_alive = 0;		//检查是否被使用
	IMAGE zd_img[2];		//飞机图片
	int width;				//飞机宽度
	int height;				//飞机高度
};

//创建的飞机结构体，包括我方飞机和敌方飞机
typedef struct plane {
	int x = 0;				//飞机横坐标
	int y = 0;				//飞机纵坐标
	int is_alive = 0;		//检查飞机是否存活/是否存在
	int hp = 0;				//飞机血量
	plane_type type;		//记录飞机类型
	IMAGE plane_img;		//飞机图片
	int width;				//飞机宽度
	int height;				//飞机高度
	zd bullet[Bullet_num];	//飞机子弹
};

plane my_plane;				//我方飞机
zd bullet[Bullet_num];		//子弹
plane enemy[Enemy_num];		//敌方飞机

IMAGE img_mainMeun;			//存储主菜单图片
IMAGE img_startButton;		//存储开始按钮图片
IMAGE img_exitButton;		//存储退出按钮图片
IMAGE back_btn;				//存储返回主界面图片

IMAGE back_img[3];			//存储游戏界面背景图
IMAGE img_plane[4];			//存储我方飞机图像
IMAGE img_emegy[6];			//存储敌方飞机图像
IMAGE img_bullet[2];		//存储我方子弹图片
IMAGE img_enemy_bullet;		//存储敌方子弹图片

int enemyFreqs[GUAN_QIA_COUNT] = { 100,50,25 };//敌机创建频率
bool gameOver = false;	//游戏结束标志
bool gameStart = true;	//游戏开始标志
int cur_score = 0;	//当前分数

//用于限制玩家不能连发子弹
static unsigned long long lastShootTime = 0;
const int SHOOT_COOLDOWN = 200; // 300毫秒冷却时间

/**************************我方攻击敌方**************************/
/**************************敌方攻击我方*************************/
//新增敌方子弹

//辅助函数
//修改了的版本，增加了对图片的检查和边界处理，把图片是否有效以及缓冲区指针是否有效分开检查了，这样尝试以后得到的报错结果会更准确
/*GetImageBufer()函数可能会产生空指针NULL（例如显存不足的时候），此时直接访问就是在访问一个空指针，会导致访问冲突
*/
void drawPNG(int picture_x, int picture_y, IMAGE* picture)//x为载入图片的X坐标，y为Y坐标
{
		// 加强参数检查
		if (picture == NULL || picture->getwidth() == 0 || picture->getheight() == 0) {
			//printf("警告：尝试绘制无效的图片！\n");
			return;
		}

		// 获取缓冲区指针并检查有效性
		DWORD* dst = GetImageBuffer(NULL);
		DWORD* src = GetImageBuffer(picture);
		if (dst == NULL || src == NULL) {
			//printf("警告：无法获取图像缓冲区指针！\n");
			return;
		}

		int picture_width = picture->getwidth();
		int picture_height = picture->getheight();
		int graphWidth = getwidth();
		int graphHeight = getheight();

		// 检查绘制位置是否完全在屏幕外
		if (picture_x + picture_width < 0 || picture_x >= graphWidth ||
			picture_y + picture_height < 0 || picture_y >= graphHeight) {
			return;
		}

		// 计算实际需要绘制的区域(避免部分在屏幕外时越界)
		int startX = max(0, -picture_x);//处理picture_x负坐标情况
		int endX = min(picture_width, graphWidth - picture_x);
		int startY = max(0, -picture_y);//处理picture_y负坐标情况
		int endY = min(picture_height, graphHeight - picture_y);

		for (int iy = startY; iy < endY; iy++) {
			for (int ix = startX; ix < endX; ix++) {
				int srcX = ix + iy * picture_width;
				int sa = ((src[srcX] & 0xff000000) >> 24);
				if (sa == 0) continue; // 完全透明则跳过

				int sr = ((src[srcX] & 0xff0000) >> 16);
				int sg = ((src[srcX] & 0xff00) >> 8);
				int sb = src[srcX] & 0xff;

				int dstX = (ix + picture_x) + (iy + picture_y) * graphWidth;
				if (dstX >= 0 && dstX < graphWidth * graphHeight) {
					int dr = ((dst[dstX] & 0xff0000) >> 16);
					int dg = ((dst[dstX] & 0xff00) >> 8);
					int db = dst[dstX] & 0xff;

					dst[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16) |
						((sg * sa / 255 + dg * (255 - sa) / 255) << 8) |
						(sb * sa / 255 + db * (255 - sa) / 255);
				}
			}
		}
}
//处理边界情况的辅助函数
//void drawPNG2(int x, int y, IMAGE* picture) {
//	IMAGE imgTmp;
//	if (y < 0) {
//		SetWorkingImage(picture);
//		getimage(&imgTmp, 0, -y,
//			picture->getwidth(), picture->getheight() + y);
//		SetWorkingImage();
//		y = 0;
//		drawPNG(x, 0, &imgTmp);
//	}
//	else {
//		drawPNG(x, y, picture);
//	}
//}

//辅助函数2，用于处理边界情况下的png透明图像
void drawPNG2(int x, int y, IMAGE* picture) {
		if (picture == NULL) return;

		// 如果完全在屏幕外则不绘制
		if (x > getwidth() || y > getheight() ||
			x + picture->getwidth() < 0 || y + picture->getheight() < 0) {
			return;
		}

		// 部分在屏幕外则调用裁剪版本
		if (x < 0 || y < 0 ||
			x + picture->getwidth() > getwidth() ||
			y + picture->getheight() > getheight()) {
			IMAGE* imgTmp = new IMAGE();
			SetWorkingImage(picture);
			getimage(imgTmp,
				max(0, -x), max(0, -y),
				min(picture->getwidth(), getwidth() - x),
				min(picture->getheight(), getheight() - y));
			SetWorkingImage();
			drawPNG(max(0, x), max(0, y), imgTmp);
			delete imgTmp;//显式的释放这个变量
		}
		else {
			drawPNG(x, y, picture);
		}
}

//导入全部所需图片
void load_image() {
	//主界面背景图片
	loadimage(&img_mainMeun, "main_meun.jpg" , 700 , 700 ,true);
	//开始按钮图片
	loadimage(&img_startButton, "start_button.png");
	//退出按钮图片
	loadimage(&img_exitButton, "exit_button.png");
	//返回主界面按钮图片
	loadimage(&back_btn, "back_btn.png");

	//游戏背景图片
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
	//loadimage(img_emegy + 6, "small_emegy_1.png");//小敌机
	loadimage(img_emegy + 5, "small_emegy_2.png");
	//子弹图片
	loadimage(img_bullet, "shot_7.png");
	loadimage(img_bullet + 1, "shot_2_10.png");
	//敌方子弹图片	
	loadimage(&img_enemy_bullet, "emegy_shot.png");
	//爆炸特效图片
	loadimage(png_boom.boom_png, "boom_1.png");
	loadimage(png_boom.boom_png+1, "boom_2.png");
	loadimage(png_boom.boom_png+2, "boom_3.png");
	loadimage(png_boom.boom_png+3, "boom_4.png");
	loadimage(png_boom.boom_png+4, "boom_5.png");
	loadimage(png_boom.boom_png+5, "boom_6.png");
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

//设计游戏主界面
void draw_main_meun() {
	putimage(0, 0, &img_mainMeun); //绘制主菜单背景

	//计算开始按钮位置
	int btn_width = img_startButton.getwidth();
	int btn_height = img_startButton.getheight();
	int btn_x = (Width - btn_width)/2; //水平居中
	int btn_y = (Height - btn_height)*0.7; 
	drawPNG2(btn_x, btn_y, &img_startButton); //绘制开始按钮

	//计算退出按钮位置
	int ebtn_width = img_exitButton.getwidth();
	int ebtn_height = img_exitButton.getheight();
	int ebtn_x = (Width - ebtn_width) / 2; //水平居中
	int ebtn_y = (Height - ebtn_height) * 0.9;
	drawPNG2(ebtn_x, ebtn_y, &img_exitButton); //绘制开始按钮

	//检测鼠标点击事件
	MOUSEMSG msg;
	if (MouseHit()) {
		msg = GetMouseMsg();
		if (msg.uMsg == WM_LBUTTONDOWN) { //左键按下
			//判断鼠标点击位置是否在开始按钮范围内
			if (msg.x >= btn_x && msg.x <= btn_x + btn_width &&
				msg.y >= btn_y && msg.y <= btn_y + btn_height) {
				game_status = PLAYING; //点击开始按钮，进入游戏状态
			}
			else if (msg.x >= ebtn_x && msg.x <= ebtn_x + ebtn_width &&
				msg.y >= ebtn_y && msg.y <= ebtn_y+ebtn_height) {
				is_exit = true; //点击退出按钮，设置退出标志
				return;
			}
		}
	}
}

//设计游戏结束界面
void draw_over_meun() {
	my_plane.hp = 0; //游戏结束时我方飞机血量归零
	setlinecolor(RGB(255, 215, 0)); // 金色边框
	setlinestyle(PS_SOLID, 3); // 3像素宽的实线
	//先搞一个黑色半透明背景
	setfillstyle(BS_HATCHED, HS_DIAGCROSS, BLACK);
	setbkcolor(BLACK);
	solidrectangle(0, 0, Width, BLACK); //填充白色背景

	//绘制游戏结束文字
	settextstyle(46, 0, _T("黑体"));
	settextcolor(BLACK);
	outtextxy(Width / 2 - 100, Height / 2 - 50, _T("游戏结束"));

	//绘制分数显示
	settextstyle(46, 0, _T("黑体"));
	TCHAR score_text[64];
	settextcolor(BLACK);
	_stprintf_s(score_text, _T("您的得分是：%d"), cur_score);
	outtextxy(Width / 2 - 150, Height / 2 + 20, score_text);

	//返回主菜单按钮
	int back_btn_width = back_btn.getwidth();
	int back_btn_height = back_btn.getheight();
	int back_btn_x = (Width - back_btn_width) / 2; //水平居中
	int back_btn_y = (Height - back_btn_height) * 0.8;
	drawPNG2(back_btn_x, back_btn_y, &back_btn); //绘制返回按钮

	//检测鼠标点击事件
	MOUSEMSG msg;
	if (MouseHit()) {
		msg = GetMouseMsg();
		if (msg.uMsg == WM_LBUTTONDOWN) { //左键按下
			//判断鼠标点击位置是否在开始按钮范围内
			if (msg.x >= back_btn_x && msg.x <= back_btn_x + back_btn_width &&
				msg.y >= back_btn_y && msg.y <= back_btn_y + back_btn_height) {
				game_status = MAIN_MENU; //按下按钮，回到主界面
			}
		}
	}
}

//初始化敌方子弹
void init_e_bullet(zd* bullet) {
	for (int i = 0; i < Bullet_num; i++) {
		(bullet + i)->is_alive = 0;
		(bullet + i)->height = img_enemy_bullet.getheight();
		(bullet + i)->width = img_enemy_bullet.getwidth();
		(bullet + i)->zd_img[0] = img_enemy_bullet;
		(bullet + i)->zd_img[1] = img_enemy_bullet;
	}
}

//初始化游戏
void init_game() {
	load_image();
	//设置背景图片
	putimage(0, 0, back_img);
	//初始化我方飞机
	my_plane.height = img_plane[0].getheight();
	my_plane.width = img_plane[0].getwidth();
	my_plane.type = HERO;
	my_plane.is_alive = 1;
	my_plane.x = (Height / 2) - (my_plane.width / 2);
	my_plane.y = Height - my_plane.height;
	my_plane.hp = plane_total_hp; //我方飞机血量
	//init_bullet(my_plane.bullet);
	drawPNG2(my_plane.x, my_plane.y, img_plane);
	//初始化敌机
	for (int i = 0; i < Enemy_num; i++) {
		enemy[i].is_alive = 0;
		int kind = rand() % 6; //随机选择敌机类型
		enemy[i].plane_img = img_emegy[kind];
		enemy[i].height = enemy[i].plane_img.getheight();
		enemy[i].width = enemy[i].plane_img.getwidth();
		init_e_bullet(enemy[i].bullet);
		//printf("enemy[%d] width = %d, height = %d\t%d\n", i, enemy[i].width, enemy[i].height,kind);
		//drawPNG(20+50*i, 20+50*i, &enemy[i].plane_img);
	}
	//初始化子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].is_alive = 0;
		bullet[i].zd_img[0] = img_bullet[0];
		bullet[i].zd_img[1] = img_bullet[1];
		bullet[i].height = img_bullet[0].getheight();
		bullet[i].width = img_bullet[0].getwidth();
	}
}

//创建爆炸播放
void create_boom(int x, int y) {
	png_boom.all_frame = 6; //总帧数
	png_boom.current_frame = 0; //当前帧数
	png_boom.last_time = GetTickCount(); //上一帧时间
	png_boom.is_playing = true; //开始播放
	png_boom.width = png_boom.boom_png[0].getwidth();
	png_boom.height = png_boom.boom_png[0].getheight();
	png_boom.x = x - png_boom.width/2; //爆炸特效的x坐标
	png_boom.y = y - png_boom.height / 2; //爆炸特效的y坐标
}

//更新爆炸特效
void update_boom() {
	if (png_boom.is_playing) {
		unsigned int current_time = GetTickCount();
		if (current_time - png_boom.last_time >= png_boom.frame_dalay) {
			png_boom.last_time = current_time;
			png_boom.current_frame++;
			//播放结束判断
			if (png_boom.current_frame >= png_boom.all_frame) {
				png_boom.is_playing = false; //播放完毕
				return;
			}
		}
	}
}

//创建敌机
void create_emegy() {
	for (int i = 0; i < Enemy_num; i++) {
		if (!enemy[i].is_alive) {
			enemy[i].is_alive = 1;
			enemy[i].x = rand() % (Width - enemy[i].width);
			enemy[i].y = -10;
			//printf("敌机创建enemy[%d] x = %d, y = %d\n", i, enemy[i].x, enemy[i].y);
			if (enemy[i].width >= 80)
			{
				enemy[i].type = BIG_ENEMY;
				enemy[i].hp = 1000; //敌机血量
			}
			else if (enemy[i].width >= 60 && enemy[i].width < 80)
			{
				enemy[i].type = MIDDLE_ENEMY;
				enemy[i].hp = 600; //敌机血量
			}
			else
			{
				enemy[i].type = SMALL_ENEMY;
				enemy[i].hp = 300; //敌机血量
			}
			return;
		}
	}
}

//敌机移动
void updata_emegy() {
	for (int i = 0; i < Enemy_num; i++) {
		if (enemy[i].is_alive) {
			if (enemy[i].y >= Width-enemy[i].height) {
				enemy[i].is_alive = 0; //飞出屏幕，死亡
				continue;
			}
			enemy[i].y+= Enemy_speed;
		}
		if(enemy[i].hp<=0) {
			create_boom(enemy[i].x + enemy[i].width / 2, enemy[i].y + enemy[i].height / 2);
			enemy[i].is_alive = 0; //敌机血量小于等于0，死亡
			//printf("敌机[%d]被击毁\n", i);
			switch (enemy[i].type) {
				case BIG_ENEMY:
					cur_score += 3; //大敌机得分
					break;
				case MIDDLE_ENEMY:
					cur_score += 2; //中敌机得分
					break;
				case SMALL_ENEMY:
					cur_score += 1; //小敌机得分
					break;
			}
		}
	}
}

//矩阵重叠判断函数,设A[x1_1,y1_1,x1_2,y1_2]  B[x2_1, y2_1, x2_2, y2_2].
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

	//检查子弹和敌机是否碰撞
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			for (int j = 0; j < Enemy_num; j++) {
				if (enemy[j].is_alive) {
					int zdX1 = bullet[i].x + bullet[i].width / 4;
					int zdX2 = zdX1 + bullet[i].width * 0.75;
					int zdY1 = bullet[i].y + bullet[i].height / 4;
					int zdY2 = zdY1 + bullet[i].height * 0.75;
					int emX1 = enemy[j].x + enemy[j].width / 4;
					int emX2 = emX1 + enemy[j].width * 0.75;
					int emY1 = enemy[j].y + enemy[j].height / 4;
					int emY2 = emY1 + enemy[j].height * 0.75;
					if (is_overlap(zdX1, zdX2, zdY1, zdY2, emX1, emX2, emY1, emY2)) {
						bullet[i].is_alive = 0; //子弹死亡
						//printf("子弹[%d]碰撞敌机[%d]\n", i, j);
						enemy[j].hp -= 300; //敌机血量减少
					}
				}
			}
		}
	}
	//英雄飞机的碰撞箱范围，二维的
	int myX1 = my_plane.x + my_plane.width / 4;
	int myX2 = myX1 + my_plane.width * 0.75;
	int myY1 = my_plane.y + my_plane.height / 4;
	int myY2 = myY1 + my_plane.height * 0.75;

	//检查敌机子弹和英雄飞机是否碰撞
	//每一个敌机的每一颗is_alive==1的子弹都要和英雄飞机检查是否碰撞
	//for (int j = 0; j < Enemy_num; j++)
	//{//外层循环是敌机
	//	if (!enemy[j].is_alive) continue; //如果敌机不存在，则跳过
		for(int i = 0 ; i< Enemy_num ; i++){
			if (!enemy[i].is_alive) continue; //如果敌机不存在，则跳过
			for(int j =  0; j < Bullet_num; j++) {
				if (enemy[i].bullet[j].is_alive) {
					int em_bulletX1 = enemy[i].bullet[j].x + enemy[i].bullet[j].width / 4;
					int em_bulletX2 = em_bulletX1 + enemy[i].bullet[j].width * 0.75;
					int em_bulletY1 = enemy[i].bullet[j].y + enemy[i].bullet[j].height / 4;
					int em_bulletY2 = em_bulletY1 + enemy[i].bullet[j].height * 0.75;
					//检查子弹和英雄飞机是否碰撞
					if (is_overlap(myX1, myX2, myY1, myY2, em_bulletX1, em_bulletX2, em_bulletY1, em_bulletY2)) {
						my_plane.hp -= 50; //英雄飞机血量减少
						enemy[i].bullet[j].is_alive = 0; //敌机子弹死亡
						printf("英雄飞机hp -50\n");
						if (my_plane.hp <= 0) {
							my_plane.is_alive = 0; //英雄飞机死亡
							my_plane.hp = 0; //血量归0
							create_boom(my_plane.x+my_plane.width/2, my_plane.y+my_plane.height/2); //创建爆炸特效
							printf("英雄飞机被击毁\n");
							gameOver = true; //游戏结束
							game_status = GAME_OVER; //设置游戏状态为结束
							return;
						}
					}
				}
			}
	}
	//检查敌机和英雄飞机是否碰撞
	for (int j = 0; j < Enemy_num; j++) {
		if (enemy[j].is_alive) {
			int emX1 = enemy[j].x + enemy[j].width / 4;
			int emX2 = emX1 + enemy[j].width * 0.75;
			int emY1 = enemy[j].y + enemy[j].height / 4;
			int emY2 = emY1 + enemy[j].height * 0.75;
			//检查子弹和敌机是否碰撞
			if (is_overlap(myX1, myX2, myY1, myY2, emX1, emX2, emY1, emY2)) {
				my_plane.hp -= 20; //英雄飞机血量减少
				enemy[j].hp -= 20; //敌机血量减少
				printf("英雄飞机hp -20");
				//printf("英雄飞机碰撞敌机[%d]\n", j);
				if (my_plane.hp <= 0) {
					my_plane.is_alive = 0; //英雄飞机死亡
					my_plane.hp = 0; //血量归0
					create_boom(my_plane.x + my_plane.width / 2, my_plane.y + my_plane.height / 2);
					printf("英雄飞机被击毁\n");
					gameOver = true; //游戏结束
					game_status = GAME_OVER;
					return;
				}
			}
		}
	}
}

//创建子弹
void create_bullet() {
	//限制子弹发射频率
	unsigned long long currentTime = GetTickCount();
	if (currentTime - lastShootTime < SHOOT_COOLDOWN || !auto_shoot) {
		return; // 冷却时间未到，不能发射
	}

	for (int i = 0; i < Bullet_num; i++) {
		if (!bullet[i].is_alive) {
			bullet[i].is_alive = 1;
			bullet[i].x = my_plane.x + my_plane.width / 2 - bullet[i].width / 2;
			bullet[i].y = my_plane.y - bullet[i].height;
			//printf("子弹[%d] x = %d, y = %d\n", i, bullet[i].x, bullet[i].y);
			lastShootTime = currentTime; // 记录本次发射时间
			return;
		}
	}
}

//子弹移动
void updata_bullet() {
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			if (bullet[i].y < 0) {
				bullet[i].is_alive = 0; //子弹飞出屏幕，死亡
				continue;
			}
			bullet[i].y -= Bullet_speed;
		}
	}
}

//绘制血条
void draw_blood() {
	setfillstyle(BS_SOLID, 0, BLACK);//因为结束游戏界面有一个网格形状的设置，所以这里强制纯色填充
	setlinecolor(RGB(255, 215, 0)); // 金色边框
	setlinestyle(PS_SOLID, 3); // 3像素宽的实线
	int total_hp_long = 300;		//血条总长度
	int hp_height = 20;				//血条高度
	int border = 2;					//血条边框宽度
	int x = 10, y = 10;				//血条位置

	//先绘制一个背景覆盖多次游戏以后的背景色
	setfillcolor(RGB(50, 50, 50)); //深灰色背景
	fillrectangle(x, y, x + total_hp_long + border * 2, y + hp_height + border * 2);
	 
	//绘制血条背景
	setfillcolor(RGB(100, 100, 100));//灰色背景
	fillrectangle(x, y, x + total_hp_long, y + hp_height);
	//计算当前血量的比值
	float hp_precent = 1.0f;
	if (my_plane.hp > 0) {
		hp_precent = (float)my_plane.hp / (float)plane_total_hp;//除以血量总值
	}
	else
	{
		hp_precent = 0.0f; //如果血量小于等于0，则血量百分比为0
	}
	//根据血量切换血条颜色
	COLORREF hp_color;
	if (hp_precent > 0.6f) hp_color = GREEN;
	else if (hp_precent <= 0.6f && hp_precent > 0.3f) hp_color = YELLOW;
	else hp_color = RED;
	setfillcolor(hp_color);//设置血条颜色
	fillrectangle(x, y, x + total_hp_long * hp_precent, y + hp_height);
	//绘制文字标签
	settextstyle(12, 0, _T("宋体"));//设置文本的字体，方向和大小的函数，EasyX自带
	settextcolor(WHITE);
	TCHAR label[32];
	_stprintf_s(label, _T("%d/%d"),my_plane.hp, plane_total_hp);
	outtextxy(x+100, y+4, label);
}

//绘制分数显示
void draw_score() {
	settextstyle(12, 0, _T("宋体"));
	settextcolor(WHITE);
	setbkmode(TRANSPARENT); //设置背景透明
	
	//格式化得分文本
	TCHAR scoreText[32];
	_stprintf_s(scoreText, _T("当前得分: %d"), cur_score);
	outtextxy(Width - 200, 10, scoreText); //在右上角显示分数
}

//更新各图像位置(移动)--在游戏界面
void Draw_plane() {
	//绘制英雄飞机
	if(my_plane.is_alive) {
		putimage(0, 0, back_img);
		drawPNG2(my_plane.x, my_plane.y, img_plane);
		printf("英雄飞机当前血量 hp = %d\n", my_plane.hp);
	}
	//绘制敌机
	for (int i = 0; i < Enemy_num; i++)
	{
		if (enemy[i].is_alive) {
			//printf("绘制敌机[%d] x = %d, y = %d\n", i, enemy[i].x, enemy[i].y);
			drawPNG2(enemy[i].x, enemy[i].y, &enemy[i].plane_img);
		}
	}
	//绘制子弹
	for(int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			drawPNG2(bullet[i].x, bullet[i].y, &bullet[i].zd_img[0]);
		}
	}
	//绘制敌机子弹
	for (int i = 0; i < Enemy_num; i++)
	{
		if (!enemy[i].is_alive) continue; //如果敌机不存在，则跳过
		for(int j = 0; j < Bullet_num; j++) {
			if (enemy[i].bullet[j].is_alive) {
				drawPNG2(enemy[i].bullet[j].x, enemy[i].bullet[j].y, &img_enemy_bullet);
				//printf("绘制敌机[%d]子弹 x = %d, y = %d\n", i, enemy_bullet[j].x, enemy_bullet[j].y);
			}
		}
	}
	//绘制爆炸特效
	if (png_boom.is_playing) {
		drawPNG2(png_boom.x, png_boom.y, &png_boom.boom_png[png_boom.current_frame]);
	}
	//画血条
	draw_blood();
	//画分数
	draw_score();
}

//修改以后的飞机事件
void change_plane_event() {
	// 方向控制
	if (GetAsyncKeyState('A') & 0x8000) {
		my_plane.x -= Plane_speed;
		//控制飞机边界情况，不能让飞机飞出游戏边界
		if (my_plane.x < 0)my_plane.x = 0;
	}
	if (GetAsyncKeyState('W') & 0x8000) {
		/*输入W，意味着要飞机朝上边移动，则x值不变，飞机y值减小*/
		my_plane.y -= Plane_speed;
		if (my_plane.y < 0)my_plane.y = 0;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		/*输入S，意味着要飞机朝下边移动，则x值不变，飞机y值增大*/
		my_plane.y += Plane_speed;
		if (my_plane.y > 650)my_plane.y = 650;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		/*输入D，意味着要飞机朝右边移动，则y值不变，飞机x值增大*/
		my_plane.x += Plane_speed;
		if (my_plane.x > 650)my_plane.x = 650;
	}
	if (GetAsyncKeyState(' ') & 0x8000) {
		/*输入空格，意味着要发射子弹*/
		create_bullet(); //创建子弹
		updata_bullet(); //子弹移动
	}
}

//判断敌人是否存活
bool is_enegy_alive() {
	//检查敌机是否全部死亡
	for (int i = 0; i < Enemy_num; i++) {
		if (!enemy[i].is_alive) {
			return true;
		}
	}
	return false;
}

/**************************我方攻击敌方******************/
/**************************敌方攻击我方*****************/

//创建敌方子弹
void create_emegy_bullet() {
	//因为每一个敌机都有这么多子弹，所以遍历所有敌机，每个敌机都有发射子弹的机会，下面敌机子弹移动也是该逻辑
	for (int j = 0; j < Enemy_num; j++)
	{
		if (!enemy[j].is_alive) continue; //如果敌机不存在，则跳过
		for (int i = 0; i < Bullet_num; i++) {
			if (!enemy[j].bullet[i].is_alive && rand() % 1000 < 2) {//千分之二的概率发射子弹
				enemy[j].bullet[i].is_alive = 1;
				enemy[j].bullet[i].x = enemy[j].x + enemy[j].width / 2 - enemy[j].bullet[i].width / 2;
				enemy[j].bullet[i].y = enemy[j].y + enemy[j].bullet[i].height;
				//printf("敌方子弹[%d] x = %d, y = %d\n", i, enemy_bullet[i].x, enemy_bullet[i].y);
				break;
			}
		}
	}
}

//敌方子弹移动
void updata_enemy_bullet() {
	for (int j = 0; j < Enemy_num; j++)
	{
		if (!enemy[j].is_alive) continue; //如果敌机不存在，则跳过
		for (int i = 0; i < Bullet_num; i++) {
			if (enemy[j].bullet[i].is_alive) {
				if(enemy[j].bullet[i].y > Height- enemy[j].bullet[i].height) {
					enemy[j].bullet[i].is_alive = 0; //子弹飞出屏幕，死亡
					continue;
				}
				enemy[j].bullet[i].y += Enemy_bullet_speed; //敌方子弹向下移动
			}
		}
	}
}

//每次进入游戏之后都要重置游戏状态
void reset_game() {
	cur_score = 0; //重置分数
	my_plane.height = img_plane[0].getheight();
	my_plane.width = img_plane[0].getwidth();
	my_plane.type = HERO;
	my_plane.is_alive = 1;
	my_plane.x = (Height / 2) - (my_plane.width / 2);
	my_plane.y = Height - my_plane.height;
	my_plane.hp = plane_total_hp; //我方飞机血量
	//init_bullet(my_plane.bullet);
	drawPNG2(my_plane.x, my_plane.y, img_plane);
	//初始化敌机
	for (int i = 0; i < Enemy_num; i++) {
		enemy[i].is_alive = 0;
		int kind = rand() % 6; //随机选择敌机类型
		enemy[i].plane_img = img_emegy[kind];
		enemy[i].height = enemy[i].plane_img.getheight();
		enemy[i].width = enemy[i].plane_img.getwidth();
		init_e_bullet(enemy[i].bullet);
		//printf("enemy[%d] width = %d, height = %d\t%d\n", i, enemy[i].width, enemy[i].height,kind);
		//drawPNG(20+50*i, 20+50*i, &enemy[i].plane_img);
	}
	//初始化子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].is_alive = 0;
		bullet[i].zd_img[0] = img_bullet[0];
		bullet[i].zd_img[1] = img_bullet[1];
		bullet[i].height = img_bullet[0].getheight();
		bullet[i].width = img_bullet[0].getwidth();
	}
}

void music_playing(const char*  music_fileName , bool loop = true) {
	char cmd[256];
	sprintf_s(cmd, "open\ %s \"alias bgmusic", music_fileName);
	mciSendString(cmd, NULL, 0, NULL);

	if (loop) {
		mciSendString("play bgmusic repeat", NULL, 0, NULL); //循环播放
	}
	else
	{
		mciSendString("play bgmusic", NULL, 0, NULL); //不循环播放
	}

	is_music_playing = true; //设置音乐播放状态为true
}

void music_stop() {
	if (is_music_playing) {
		mciSendString("stop bgmusic", NULL, 0, NULL); //停止音乐
		is_music_playing = false; //设置音乐播放状态为false
	}
}


int main() {
	//初始化界面
	initgraph(Width, Height);
	init_game();
	game_status = MAIN_MENU; //初始状态为主菜单
	//int time_count = 0;
	while (1) {
		if (is_exit) break; //如果点击了退出按钮，则退出游戏
		switch (game_status)
		{
			case MAIN_MENU:
				cur_score = 0; //重置分数
				BeginBatchDraw();
				draw_main_meun(); //绘制主菜单
				FlushBatchDraw();
				Sleep(10); //延时10毫秒，避免CPU占用过高
				//system("pause");
				break;
			case PLAYING: {
				reset_game(); //重置游戏状态
				gameOver = false; //游戏未结束
				int time_count = 0;
				BeginBatchDraw();
				while (1) {
					Draw_plane();
					change_plane_event();
					time_count += timer();
					if (time_count >= enemyFreqs[1] || is_enegy_alive()) {
						create_emegy(); //创建敌机
						updata_emegy();
						create_emegy_bullet(); //创建敌方子弹
						is_collide(); //检测碰撞
						updata_enemy_bullet(); //敌方子弹移动
						time_count = 0;
					}
					updata_bullet(); //子弹移动
					is_collide(); //检测碰撞
					update_boom(); //更新爆炸特效
					FlushBatchDraw();
					Sleep(10); //延时10毫秒,避免CPU占用过高
					if (gameOver) break;
				}
				//EndBatchDraw();
				game_status = GAME_OVER; //游戏结束状态
				//system("pause");
				break;
			}//限定time_count的作用域
			case GAME_OVER:
				BeginBatchDraw();
				draw_over_meun(); //绘制游戏结束界面
				draw_blood();
				FlushBatchDraw();
				//Sleep(10); //延时10毫秒，避免CPU占用过高
				break;
		}
	}
	closegraph(); //关闭图形窗口
	return 0;
}