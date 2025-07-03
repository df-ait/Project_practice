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
	Enemy_num = 8,			//敌机数量
};

int Plane_speed = 5,		//飞机移动速度
int Enemy_speed = 2,		//敌机速度
int Bullet_speed = 7,		//我方子弹移动速度
int df_level = 0;	//当前关卡
int Enemy_bullet_speed[GUAN_QIA_COUNT] = {3,6,9};	//敌方子弹速度
int enemyFreqs[GUAN_QIA_COUNT] = { 100,50,25 };//敌机创建频率

typedef enum {
	OTHER = 4,
	BIG_ENEMY = 3,
	MIDDLE_ENEMY = 2,
	SMALL_ENEMY = 1,
	HERO = 0,
}plane_type;

enum GameState {
	MAIN_MENU = 0,	//主菜单
	SINGLE_PLAYER,	//单人游戏
	DOUBLE_PLAYER,	//双人游戏
	GAME_OVER,		//游戏结束
};

GameState last_game_status = MAIN_MENU;	//上一次游戏界面状态
GameState game_status = MAIN_MENU;	//游戏界面状态
bool is_exit = false;	//退出游戏标志
bool is_music_playing = true;	//音乐播放标志
bool auto_shoot_my = false;	//我方自动射击标志,是否进入自动射击模式
bool auto_shoot_you = false; //对方自动射击标志

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
	bool is_winner = false;	//是否获胜
};

//用户自定义输入框
typedef struct inputBox {
	int x, y, width, height;  // 位置和大小
	char text[256];           // 存储用户输入
	bool is_active;           // 是否正在输入
	int cursor_pos;           // 光标位置
	int cursor_blink;         // 光标闪烁计时
};

plane my_plane;				//我方飞机
plane you_plane;			//双人游戏的另一方飞机
zd bullet[Bullet_num];		//我方子弹
zd bullet_you[Bullet_num];	//对方子弹
plane enemy[Enemy_num];		//敌方飞机

IMAGE img_mainMeun;			//存储主菜单图片
IMAGE img_singleButton;		//存储单人游戏按钮图片
IMAGE img_doubleButton;		//存储双人游戏按钮图片
IMAGE img_exitButton;		//存储退出按钮图片
IMAGE back_btn;				//存储返回主界面图片

IMAGE back_img[3];			//存储游戏界面背景图
IMAGE img_plane[2];			//存储我方飞机图像
IMAGE img_your_plane[2];	//存储对方飞机图像
IMAGE img_emegy[6];			//存储敌方飞机图像
IMAGE img_bullet[2];		//存储我方子弹图片
IMAGE img_enemy_bullet;		//存储敌方子弹图片

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

//all 导入全部所需图片
void load_image() {
	//主界面背景图片
	loadimage(&img_mainMeun, "main_meun.jpg" , 700 , 700 ,true);
	//开始按钮图片
	loadimage(&img_singleButton, "single_game_btn.png");
	//退出按钮图片
	loadimage(&img_exitButton, "exit_button.png");
	//双人游戏按钮图片
	loadimage(&img_doubleButton, "double_game_btn.png");
	//返回主界面按钮图片
	loadimage(&back_btn, "back_btn.png");

	//游戏背景图片
	loadimage(back_img, "map1.jpg", 700, 700, true);
	loadimage(back_img + 1, "map3.jpg", 700, 700, true);
	//玩家飞机图片
	loadimage(&img_plane[0], "player_1_50.png.");
	loadimage(&img_plane[1], "player_2_75.png");
	//玩家二飞机图片
	loadimage(&img_your_plane[0], "player_3_50.png");
	loadimage(&img_your_plane[1], "player_4_50.png");

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

//all 计时器
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

//检测当前分数，改变敌机速度
void check_level() {
	if (cur_score < 65)
	{
		df_level = 0;
	}
	else if (cur_score < 130 &&cur_score>=65) {
		df_level = 1;
	}
	else
	{
		df_level = 2;
	}
}

//all 设计游戏主界面
void draw_main_meun() {
	putimage(0, 0, &img_mainMeun); //绘制主菜单背景

	//计算单人游戏按钮位置
	int btn_width = img_singleButton.getwidth();
	int btn_height = img_singleButton.getheight();
	int btn_x = (Width - btn_width)/2 - btn_width/2; //水平居中
	int btn_y = (Height - btn_height)*0.7; 
	drawPNG2(btn_x, btn_y, &img_singleButton); //绘制开始按钮

	//计算双人游戏按钮位置
	int dbtn_width = img_doubleButton.getwidth();
	int dbtn_height = img_doubleButton.getheight();
	int dbtn_x = (Width - dbtn_width) / 2 + dbtn_width/2; //水平居中
	int dbtn_y = (Height - dbtn_height) * 0.7;
	drawPNG2(dbtn_x, dbtn_y, &img_doubleButton); //绘制双人游戏按钮

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
			//单人游戏按钮范围内
			if (msg.x >= btn_x && msg.x <= btn_x + btn_width &&
				msg.y >= btn_y && msg.y <= btn_y + btn_height) {
				game_status = SINGLE_PLAYER; //点击开始按钮，进入游戏状态
			}
			//双人游戏按钮范围内
			else if (msg.x >= dbtn_x && msg.x <= dbtn_x + dbtn_width &&
				msg.y >= dbtn_y && msg.y <= dbtn_y + dbtn_height) {
				game_status = DOUBLE_PLAYER; //点击双人游戏按钮，进入双人游戏状态
			}
			//退出按钮范围内
			else if (msg.x >= ebtn_x && msg.x <= ebtn_x + ebtn_width &&
				msg.y >= ebtn_y && msg.y <= ebtn_y+ebtn_height) {
				is_exit = true; //点击退出按钮，设置退出标志
				return;
			}
		}
	}
}

//single 设计游戏结束界面
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

//double 模式下的结束界面
void draw_over_meun_double() {
	//my_plane.hp = 0; //游戏结束时我方飞机血量归零
	setlinecolor(RGB(255, 215, 0)); // 金色边框
	setlinestyle(PS_SOLID, 3); // 3像素宽的实线
	//先搞一个黑色半透明背景
	setfillstyle(BS_HATCHED, HS_DIAGCROSS, BLACK);
	//setbkcolor(WHITE);
	solidrectangle(0, 0, Width, BLACK); //填充背景

	//绘制游戏结束文字
	settextstyle(46, 0, _T("黑体"));
	settextcolor(RED);
	outtextxy(Width / 2 - 100, Height / 2 - 50, _T("游戏结束"));

	//赢家信息
	settextstyle(46, 0, _T("黑体"));
	settextcolor(RED);
	if (my_plane.is_winner) {
		outtextxy(Width / 2 - 150, Height / 2 + 20, _T("该局赢家是：Player_1"));
	}
	else if (you_plane.is_winner) {
		outtextxy(Width / 2 - 150, Height / 2 + 20, _T("该局赢家是：Player_2"));
	}
	else {
		outtextxy(Width / 2 - 150, Height / 2 + 20, _T("该局没有赢家，为平局"));
	}

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

//single 初始化敌方子弹
void init_e_bullet(zd* bullet) {
	for (int i = 0; i < Bullet_num; i++) {
		(bullet + i)->is_alive = 0;
		(bullet + i)->height = img_enemy_bullet.getheight();
		(bullet + i)->width = img_enemy_bullet.getwidth();
		(bullet + i)->zd_img[0] = img_enemy_bullet;
		(bullet + i)->zd_img[1] = img_enemy_bullet;
	}
}

//设置音乐音量
//void set_music_volume(int volume) {
//	//设置音量，范围0-1000
//	if (volume < 0) volume = 0;
//	if (volume > 1000) volume = 1000;
//	TCHAR command[64];
//	_stprintf_s(command, _T("setaudio bgm volume to %d"), volume);
//	mciSendString(command, NULL, 0, NULL);
//}
//
////音乐初始化
//void init_music() {
//	//打开音乐文件
//	mciSendString("open \"game_bgm.mp3\" alias bgm type avivideo", NULL, 0, NULL);
//	printf("音乐文件打开成功\n");
//	MCIERROR err = mciSendString("play bgm", NULL, 0, NULL);
//	if (err != 0) {
//		char errorMsg[256];
//		mciGetErrorString(err, errorMsg, sizeof(errorMsg));
//		printf("播放失败\n");
//		printf("错误信息: %s\n", errorMsg);
//	}
//	else {
//		printf("音乐应正在播放，请检查系统音量\n");
//	}
//	//设置循环播放
//	mciSendString(_T("play bgm repeat"), NULL, 0, NULL);
//	//设置初始音量
//	set_music_volume(500);
//	
//	
//}
//
////停止音乐
//void stop_music() {
//	mciSendString(_T("stop bgm"), NULL, 0, NULL);
//	is_music_playing = false; //音乐停止标志
//}

//all 初始化游戏
void init_game() {
	load_image();
	//init_music(); //初始化音乐
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
	my_plane.plane_img = img_plane[0]; //设置飞机图片
	//init_bullet(my_plane.bullet);
	drawPNG2(my_plane.x, my_plane.y, img_plane);
	//初始化对面飞机
	you_plane.height = img_your_plane[0].getheight();
	you_plane.width = img_your_plane[0].getwidth();
	you_plane.type = OTHER;
	you_plane.is_alive = 1;
	you_plane.x = (Height / 2) - (you_plane.width / 2);
	you_plane.y = 0;
	you_plane.hp = plane_total_hp; //飞机血量
	you_plane.plane_img = img_your_plane[0]; //设置飞机图片
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
	//初始化PLAYER_1子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].is_alive = 0;
		bullet[i].zd_img[0] = img_bullet[0];
		bullet[i].zd_img[1] = img_bullet[1];
		bullet[i].height = img_bullet[0].getheight();
		bullet[i].width = img_bullet[0].getwidth();
	}
	//初始化PLAYER_2子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet_you[i].is_alive = 0;
		bullet_you[i].zd_img[0] = img_enemy_bullet;
		bullet_you[i].zd_img[1] = img_enemy_bullet;
		bullet_you[i].height = img_enemy_bullet.getheight();
		bullet_you[i].width = img_enemy_bullet.getwidth();
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

//single+double更新爆炸特效
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

//single 创建敌机
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

//single 敌机移动
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

//single 检测碰撞函数
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

//single+double 创建子弹
void create_bullet() {
	//限制子弹发射频率
	static unsigned long long lastShootTime = 0; //记录上次发射时间
	unsigned long long currentTime = GetTickCount();
	if (currentTime - lastShootTime < SHOOT_COOLDOWN || !auto_shoot_my) {
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

//single+double 子弹移动
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

//double 创建PLAYER_2子弹
void create_you_bullet() {
	//限制子弹发射频率
	static unsigned long long lastShootTime_u = 0; //记录上次发射时间
	unsigned long long currentTime_u = GetTickCount();
	if (currentTime_u - lastShootTime_u < SHOOT_COOLDOWN || !auto_shoot_you) {
		return; // 冷却时间未到，不能发射
	}

	for (int i = 0; i < Bullet_num; i++) {
		if (!bullet_you[i].is_alive) {
			bullet_you[i].is_alive = 1;
			bullet_you[i].x = you_plane.x + you_plane.width / 2 - bullet_you[i].width / 2;
			bullet_you[i].y = you_plane.y + bullet_you[i].height;
			//printf("子弹[%d] x = %d, y = %d\n", i, bullet[i].x, bullet[i].y);
			lastShootTime_u = currentTime_u; // 记录本次发射时间
			return;
		}
	}
}

//double PLAYER_2子弹移动
void updata_you_bullet() {
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet_you[i].is_alive) {
			if (bullet_you[i].y > Height) {
				bullet_you[i].is_alive = 0; //子弹飞出屏幕，死亡
				continue;
			}
			bullet_you[i].y += Bullet_speed;
		}
	}
}

//single 绘制血条
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

//single 绘制分数显示
void draw_score() {
	settextstyle(12, 0, _T("宋体"));
	settextcolor(WHITE);
	setbkmode(TRANSPARENT); //设置背景透明
	
	//格式化得分文本
	TCHAR scoreText[32];
	_stprintf_s(scoreText, _T("当前得分: %d"), cur_score);
	outtextxy(Width - 200, 10, scoreText); //在右上角显示分数
	TCHAR lev_1[128];
	TCHAR lev_2[128];
	TCHAR lev_3[128];
	_stprintf_s(lev_1, _T("简单模式,敌机产生频率:%dms,敌机子弹速度:%d"), enemyFreqs[df_level], Enemy_bullet_speed[df_level]);
	_stprintf_s(lev_2, _T("中等模式,敌机产生频率:%dms,敌机子弹速度:%d"), enemyFreqs[df_level], Enemy_bullet_speed[df_level]);
	_stprintf_s(lev_3, _T("困难模式,敌机产生频率:%dms,敌机子弹速度:%d"), enemyFreqs[df_level], Enemy_bullet_speed[df_level]);
	if (df_level == 0) outtextxy(Width - 260, 25, lev_1);
	else if(df_level == 1) outtextxy(Width - 260, 25, lev_2);
	else if(df_level == 2) outtextxy(Width - 260, 25, lev_3);
	
}

//single 更新各图像位置(移动)--在游戏界面
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

//double 双人游戏中两飞机是否碰撞
void is_collide_double() {
	//player_1
	int myX1 = my_plane.x + my_plane.width / 4;
	int myX2 = myX1 + my_plane.width * 0.75;
	int myY1 = my_plane.y + my_plane.height / 4;
	int myY2 = myY1 + my_plane.height * 0.75;
	//player_2
	int uX1 = you_plane.x + you_plane.width / 4;
	int uX2 = uX1 + you_plane.width * 0.75;
	int uY1 = you_plane.y + you_plane.height / 4;
	int uY2 = uY1 + you_plane.height * 0.75;

	//检测两飞机是否碰撞
	if (is_overlap(myX1, myX2, myY1, myY2, uX1, uX2, uY1, uY2)) {
		my_plane.hp -= 20; //PLAYER_1血量减少
		you_plane.hp -= 20;//PLAYER_2血量减少
		printf("双方飞机  碰撞  hp -50\n");
		printf("双方当前血量为_player_1:%d,player_2:%d\n", my_plane.hp, you_plane.hp);
		if (my_plane.hp <= 0) {
			my_plane.is_alive = 0; //PLAYER_1飞机死亡
			my_plane.hp = 0; //血量归0
			create_boom(my_plane.x + my_plane.width / 2, my_plane.y + my_plane.height / 2); //创建爆炸特效
			printf("PLAYER_1被击毁\n");
			you_plane.is_winner = true; //PLAYER_2获胜
			gameOver = true; //游戏结束
			game_status = GAME_OVER; //设置游戏状态为结束
			return;
		}
		if (you_plane.hp <= 0) {
			you_plane.is_alive = 0; //PLAYER_1飞机死亡
			you_plane.hp = 0; //血量归0
			create_boom(you_plane.x + you_plane.width / 2, you_plane.y + you_plane.height / 2); //创建爆炸特效
			printf("PLAYER_2被击毁\n");
			my_plane.is_winner = true; //PLAYER_1获胜
			gameOver = true; //游戏结束
			game_status = GAME_OVER; //设置游戏状态为结束
			return;
		}
	}
	//检测我方子弹与对方飞机是否碰撞
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			if (you_plane.is_alive) {
				int zdX1 = bullet[i].x + bullet[i].width / 4;
				int zdX2 = zdX1 + bullet[i].width * 0.75;
				int zdY1 = bullet[i].y + bullet[i].height / 4;
				int zdY2 = zdY1 + bullet[i].height * 0.75;
				if (is_overlap(zdX1, zdX2, zdY1, zdY2, uX1, uX2, uY1, uY2)) {
					bullet[i].is_alive = 0; //子弹死亡
					//printf("子弹[%d]碰撞敌机[%d]\n", i, j);
					you_plane.hp -= 100; //PLAYER_2血量减少
					printf("Player_1 打 playere_2,  p2.hp -100\n");
					printf("双方当前血量为_player_1:%d,player_2:%d\n", my_plane.hp, you_plane.hp);
					if (you_plane.hp <= 0) {
						you_plane.is_alive = 0; //PLAYER_1飞机死亡
						you_plane.hp = 0; //血量归0
						create_boom(you_plane.x + you_plane.width / 2, you_plane.y + you_plane.height / 2); //创建爆炸特效
						printf("PLAYER_2被击毁\n");
						my_plane.is_winner = true; //PLAYER_1获胜
						gameOver = true; //游戏结束
						game_status = GAME_OVER; //设置游戏状态为结束
						return;
					}
				}
			}
		}
	}

	//检测PLAYER_2子弹与我方飞机是否碰撞,逻辑一样
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet_you[i].is_alive) {
			if (you_plane.is_alive) {
				int zdX1 = bullet_you[i].x + bullet_you[i].width / 4;
				int zdX2 = zdX1 + bullet_you[i].width * 0.75;
				int zdY1 = bullet_you[i].y + bullet_you[i].height / 4;
				int zdY2 = zdY1 + bullet_you[i].height * 0.75;
				if (is_overlap(zdX1, zdX2, zdY1, zdY2, myX1, myX2, myY1, myY2)) {
					bullet_you[i].is_alive = 0; //子弹死亡
					//printf("子弹[%d]碰撞敌机[%d]\n", i, j);
					my_plane.hp -= 100; //PLAYER_1血量减少
					printf("Player_2 打 playere_1,  p1.hp -100\n");
					printf("双方当前血量为_player_1:%d,player_2:%d\n", my_plane.hp, you_plane.hp);
					if (my_plane.hp <= 0) {
						my_plane.is_alive = 0; //PLAYER_1飞机死亡
						my_plane.hp = 0; //血量归0
						create_boom(my_plane.x + my_plane.width / 2, my_plane.y + my_plane.height / 2); //创建爆炸特效
						printf("PLAYER_1被击毁\n");
						you_plane.is_winner = true; //PLAYER_2获胜
						gameOver = true; //游戏结束
						game_status = GAME_OVER; //设置游戏状态为结束
						return;
					}		
				}
			}
		}
	}
}

//single+double 修改以后的我方飞机事件
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
	if (GetAsyncKeyState('Q') & 0x8000) {
		/*输入Q，意味着要发射子弹*/
		static long long lastQTime = 0; //上一次发射子弹的时间/按下空格键的时间
		static bool QPressed = false;
		if(!QPressed) { //每隔100毫秒发射一次子弹
			auto_shoot_my = !auto_shoot_my;//状态取反，每按一次Q键，自动射击状态切换一次
			lastQTime = GetTickCount(); //更新上一次发射子弹的时间
			QPressed = true; //标记Q键已按下
		}
		else {
			QPressed = false; 
		}
		//if (auto_shoot) {
		//	create_bullet(); //创建子弹
		//	//updata_bullet(); //子弹移动
		//}	
	}
}

//double 对方飞机事件
void you_change_plane_event() {
	// 方向控制
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		you_plane.x -= Plane_speed;
		//控制飞机边界情况，不能让飞机飞出游戏边界
		if (you_plane.x < 0)you_plane.x = 0;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
		/*输入W，意味着要飞机朝上边移动，则x值不变，飞机y值减小*/
		you_plane.y -= Plane_speed;
		if (you_plane.y < 0)you_plane.y = 0;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		/*输入S，意味着要飞机朝下边移动，则x值不变，飞机y值增大*/
		you_plane.y += Plane_speed;
		if (you_plane.y > 650)you_plane.y = 650;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		/*输入D，意味着要飞机朝右边移动，则y值不变，飞机x值增大*/
		you_plane.x += Plane_speed;
		if (you_plane.x > 650)you_plane.x = 650;
	}
	if (GetAsyncKeyState('P') & 0x8000) {
		/*输入P，意味对方着要发射子弹*/
		static long long lastPTime = 0; //上一次发射子弹的时间/按下空格键的时间
		static bool PPressed = false;
		if (!PPressed) { //每隔100毫秒发射一次子弹
			auto_shoot_you = !auto_shoot_you;//状态取反，每按一次空格键，自动射击状态切换一次
			lastPTime = GetTickCount(); //更新上一次发射子弹的时间
			PPressed = true; //标记空格键已按下
		}
		else {
			PPressed = false;
		}
	}
}

//double 绘制双人界面(血条)
void common_double_blood(int x, int y, int total_hp_long, int border, int hp_height,plane target) {
	setfillcolor(RGB(50, 50, 50)); //深灰色背景
	fillrectangle(x, y, x + total_hp_long + border * 2, y + hp_height + border * 2);

	//绘制血条背景
	setfillcolor(RGB(100, 100, 100));//灰色背景
	fillrectangle(x, y, x + total_hp_long, y + hp_height);
	//计算当前血量的比值
	float hp_precent = 1.0f;
	if (target.hp > 0) {
		hp_precent = (float)target.hp / (float)plane_total_hp;//除以血量总值
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
	_stprintf_s(label, _T("%d/%d"), target.hp, plane_total_hp);
	outtextxy(x + 100, y + 4, label);
}

//double 画血条
void draw_blood_double() {
	//先绘制PLAYER_1的血条
	setfillstyle(BS_SOLID, 0, BLACK);//因为结束游戏界面有一个网格形状的设置，所以这里强制纯色填充
	setlinecolor(RGB(255, 215, 0)); // 金色边框
	setlinestyle(PS_SOLID, 3);		// 3像素宽的实线
	int my_total_hp_long = 300;		//血条总长度
	int my_hp_height = 20;				//血条高度
	int my_border = 2;					//血条边框宽度
	int my_x = 10, my_y = Height - 30;				//血条位置
	settextstyle(20, 0, _T("宋体"));
	//settextcolor(BLACK);
	outtextxy(my_x, my_y - 20, _T("Player_1"));
	common_double_blood(my_x, my_y, my_total_hp_long, my_border, my_hp_height , my_plane);

	//PLAYER_2的血条绘制
	setfillstyle(BS_SOLID, 0, BLACK);
	setlinecolor(RGB(255, 215, 0));
	setlinestyle(PS_SOLID, 3);
	int u_total_hp_long = 300;
	int u_hp_height = 20;
	int u_border = 2;
	int u_x = Width - 350, u_y = 10;
	settextstyle(20, 0, _T("宋体"));
	//settextcolor(BLACK);
	outtextxy(u_x, u_y + 20, _T("Player_2"));
	common_double_blood(u_x, u_y, u_total_hp_long, u_border, u_hp_height , you_plane);
}

//double 绘制双人游戏界面
void draw_double_game() {
	//绘制我方飞机
	if (my_plane.is_alive) {
		putimage(0, 0, back_img);
		drawPNG2(my_plane.x, my_plane.y, img_plane);
		//printf("PLAYER_1飞机当前血量 hp = %d\n", my_plane.hp);
	}
	//绘制我方子弹
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			drawPNG2(bullet[i].x, bullet[i].y, &bullet[i].zd_img[0]);
		}
	}
	
	//绘制对方飞机
	if (you_plane.is_alive) {
		//putimage(0, 0, back_img);
		drawPNG2(you_plane.x, you_plane.y, img_your_plane);
		//printf("PLAYER_2飞机当前血量 hp = %d\n", you_plane.hp);
	}
	//绘制对方子弹
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet_you[i].is_alive) {
			drawPNG2(bullet_you[i].x, bullet_you[i].y, &bullet_you[i].zd_img[0]);
		}
	}
	draw_blood_double();
}

//single 判断敌人是否存活
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

//single 创建敌方子弹
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

//single 敌方子弹移动
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
				enemy[j].bullet[i].y += Enemy_bullet_speed[df_level]; //敌方子弹向下移动
			}
		}
	}
}

//single 每次进入游戏之后都要重置游戏状态
void reset_single_game() {
	cur_score = 0; //重置分数
	auto_shoot_my = false; //重置自动射击状态
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

//double 重置双人游戏状态
void reset_double_game() {
	//初始化我方飞机
	auto_shoot_my = false; //重置自动射击状态
	my_plane.height = img_plane[0].getheight();
	my_plane.width = img_plane[0].getwidth();
	my_plane.type = HERO;
	my_plane.is_alive = 1;
	my_plane.x = (Height / 2) - (my_plane.width / 2);
	my_plane.y = Height - my_plane.height;
	my_plane.hp = plane_total_hp; //我方飞机血量
	my_plane.is_winner = false; //我方飞机获胜状态
	//init_bullet(my_plane.bullet);
	drawPNG2(my_plane.x, my_plane.y, img_plane);
	//初始化对面飞机
	auto_shoot_you = false; //重置对面自动射击状态
	you_plane.height = img_your_plane[0].getheight();
	you_plane.width = img_your_plane[0].getwidth();
	you_plane.type = OTHER;
	you_plane.is_alive = 1;
	you_plane.x = (Height / 2) - (my_plane.width / 2);
	you_plane.y = 0;
	you_plane.hp = plane_total_hp; //飞机血量
	you_plane.is_winner = false; //对面飞机获胜状态

	//初始化PLAYER_1子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].is_alive = 0;
		bullet[i].zd_img[0] = img_bullet[0];
		bullet[i].zd_img[1] = img_bullet[1];
		bullet[i].height = img_bullet[0].getheight();
		bullet[i].width = img_bullet[0].getwidth();
	}
	//初始化PLAYER_2子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet_you[i].is_alive = 0;
		bullet_you[i].zd_img[0] = img_enemy_bullet;
		bullet_you[i].zd_img[1] = img_enemy_bullet;
		bullet_you[i].height = img_enemy_bullet.getheight();
		bullet_you[i].width = img_enemy_bullet.getwidth();
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
			case SINGLE_PLAYER: {
				reset_single_game(); //重置游戏状态
				gameOver = false; //游戏未结束
				int time_count = 0;
				BeginBatchDraw();
				while (1) {
					Draw_plane();
					change_plane_event();
					time_count += timer();
					/**********************************************************/
					create_bullet(); //auto_shooot状态下创建子弹
					int deltaTime = timer(); //获取时间差	
					time_count += deltaTime; //累加时间差
					/*********************************************************/
					if (time_count >= enemyFreqs[df_level]) {
						create_emegy(); //创建敌机
						updata_emegy();
						create_emegy_bullet(); //创建敌方子弹
						is_collide(); //检测碰撞
						time_count = 0;
					}
					updata_bullet(); //子弹移动
					updata_enemy_bullet(); //敌方子弹移动
					is_collide(); //检测碰撞
					update_boom(); //更新爆炸特效
					check_level();
					FlushBatchDraw();
					Sleep(10); //延时10毫秒,避免CPU占用过高
					if (gameOver) break;
				}
				last_game_status = SINGLE_PLAYER; //记录上一次游戏状态
				//EndBatchDraw();
				game_status = GAME_OVER; //游戏结束状态
				//system("pause");
				break;
			}//限定time_count的作用域
			case DOUBLE_PLAYER: {
				reset_double_game(); //重置游戏状态
				gameOver = false; //游戏未结束
				int time_count = 0;
				BeginBatchDraw();
				while (1) {
					draw_double_game();
					change_plane_event();
					you_change_plane_event();
					time_count += timer();
					/**********************************************************/
					create_bullet(); //auto_shooot状态下创建子弹
					create_you_bullet();
					int deltaTime = timer(); //获取时间差	
					time_count += deltaTime; //累加时间差
					/*********************************************************/
					//is_collide_double(); //检测碰撞
					time_count = 0;
					updata_bullet(); //子弹移动
					updata_you_bullet();
					is_collide_double(); //检测碰撞
					update_boom(); //更新爆炸特效
					FlushBatchDraw();
					Sleep(10); //延时10毫秒,避免CPU占用过高
					if (gameOver) break;
				}
				//EndBatchDraw();
				last_game_status = DOUBLE_PLAYER; //记录上一次游戏状态
				game_status = GAME_OVER; //游戏结束状态
				//system("pause");
				break;
			}//同样的限定作用域
			case GAME_OVER:
				if (last_game_status == SINGLE_PLAYER) {
					BeginBatchDraw();
					draw_over_meun(); //绘制游戏结束界面
					draw_blood();
					FlushBatchDraw();
					Sleep(10); //延时10毫秒，避免CPU占用过高
				}
				else if (last_game_status == DOUBLE_PLAYER) {
					BeginBatchDraw();
					draw_over_meun_double();
					draw_blood_double();
					FlushBatchDraw();
					Sleep(10); //延时10毫秒，避免CPU占用过高
				}
				break;
		}
	}
	//stop_music(); //停止音乐
	closegraph(); //关闭图形窗口
	return 0;
}