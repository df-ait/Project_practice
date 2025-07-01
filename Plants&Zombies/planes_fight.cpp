#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <ctype.h>
//防止VS中认为scanf函数不安全，从而报警告
#define _CRT_SECURE_NO_WARNINGS

enum My {
	Width = 700,			//宽度
	Height = 700,			//高度
	Bullet_num = 30,		//子弹数目
	Bullet_speed = 12,		//子弹移动速度
	Plane_speed = 10,		//飞机移动速度
	Enemy_num = 6,			//敌机数量
	Enemy_speed = 1,		//敌机速度
	Big,					//敌机 大
	Middle,					//敌机 中等
	Small					//敌机 小
};

//创建的飞机结构体，包括我方飞机和敌方飞机
typedef struct plane{
	int x = 0;				//飞机横坐标
	int y = 0;				//飞机纵坐标
	int is_alive = 1;		//检查飞机是否可以正常存活下去
	int hp=1000;			//飞机血量
	char type;				//记录敌机大小,B代表大敌机，M代表中等敌机，S代表小敌机,M代表玩家飞机
	int weight;				//飞机宽度
	int height;				//飞机高度
};

plane my_plane;				//我方飞机
plane bullet[Bullet_num];	//子弹
plane enemy[Enemy_num];		//敌方飞机

IMAGE back_img[2];				//存储背景图
IMAGE img_plane[2];			//存储我方飞机图像
IMAGE img_enemy[4];			//存储敌方飞机图像
IMAGE img_bullet[2];		//存储子弹图片

//导入全部所需图片
void load_image() {
	loadimage(back_img, "map1.jpg", 700, 700, true);
	loadimage(back_img+1, "map3.jpg", 700, 700, true);
	loadimage(img_plane, "control_plane1.png", 50, 50, true);
	loadimage(img_plane + 1, "control_plane2.png", 50, 50, true);
	loadimage(img_enemy,"emege.png",45,45);
	loadimage(img_enemy + 1, "emege2.png", 60, 60);
	loadimage(img_enemy + 2, "emege3.png", 50, 50);
	loadimage(img_enemy + 3, "emege4.png", 45, 45);
	loadimage(img_bullet , "shot.png",25,25);
	loadimage(img_bullet + 1, "shot2.png", 30, 30);
}

//我方飞机的坐标
//int x = 350, y = 0;
void Draw(IMAGE plane,IMAGE back_img) {
	putimage(0, 0, &back_img);
	putimage(my_plane.x ,my_plane.y , &plane);
}

void init_game() {
	//加载所有图片
	load_image();
	//loadimage(&back_img, "control_plane1.png", 50, 50,true);
	putimage(0, 0, back_img);
	my_plane.weight = 50;
	my_plane.height = 50;
	my_plane.type = 'M';
	//初始化子弹
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].x = 0;
		bullet[i].y = 0;
		bullet[i].is_alive = 1;
	}
	//设置飞机位置,将飞机放在游戏窗口最下面的正中间
	my_plane.x = (Height/2)-(my_plane.weight/2);
	my_plane.y = Height - my_plane.height;
	putimage(my_plane.x, my_plane.y, img_plane);
}

int compare_img(IMAGE pic_a, IMAGE pic_b) {
	DWORD* pixes_a = GetImageBuffer(&pic_a);
	DWORD* pixes_b = GetImageBuffer(&pic_b);
	int pixCount_a = pic_a.getwidth() * pic_a.getheight();
	int pixCount_b = pic_b.getwidth() * pic_b.getheight();
	unsigned int hash_a=0, hash_b=0;
	for (int i = 0; i < pixCount_a; i++) {
		hash_a += pixes_a[i];
	}
	for (int j = 0; j < pixCount_b; j++) {
		hash_b += pixes_b[j];
	}
	if (hash_a == hash_b) return 1;
	return 0;
}

void plane_move() {
	IMAGE now_bk = back_img[0];
	IMAGE now_plane = img_plane[0];
	while (1) {
		switch (toupper(_getch())) {
		case 'A':
			/*输入A，意味着要飞机朝左边移动，则y值不变，飞机x值减小*/
			my_plane.x -= Plane_speed;
			//控制飞机边界情况，不能让飞机飞出游戏边界
			if (my_plane.x < 0)my_plane.x = 0;
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
		case 'J':
			/*输入J，意味着要飞机要发射子弹攻击敌人*/



			break;
		case 'C':
			/*输入C，意味着要切换地图*/
			if (compare_img(now_bk, back_img[0])) {
				//如果此时背景就为初始背景，那么按下C应该切换到另外一张背景图
				now_bk = back_img[1];
			}
			else {
				//如果此时背景不为初始背景，那么按下C应该切换到初始背景图，下面切换飞机皮肤同理
				now_bk = back_img[0];
			}
			putimage(0, 0, &now_bk);
			break;
		case 'P':
			/*输入P，意味着要切换飞机皮肤*/
			if (compare_img(now_plane, img_plane[0])) {
				now_plane = img_plane[1];
			}
			else {
				now_plane = img_plane[0];
			}
			putimage(0, 0, &now_bk);
			break;
		case 'Q':
			/*输入Q，意味着要退出游戏*/
			return;
		default:
			break;
		}
		Draw(now_plane, now_bk);
	}
}

int main() {
	//初始化窗口
	initgraph(Width,Height);
	//初始化游戏
	init_game();
	//下面是对飞机or背景的操作
	plane_move();
	//system("pause");
	return 0;
}