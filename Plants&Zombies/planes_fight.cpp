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
	int y = 0;					//飞机纵坐标
	bool is_alive = true;	//检查飞机是否可以正常存活下去
	int hp=1000;					//飞机血量
	int type;				//记录敌机大小
	int weight;				//记录飞机大小
	int height;
};

plane my_plane;				//我方飞机
plane bullet[Bullet_num];	//子弹
plane enemy[Enemy_num];		//敌方飞机

//我方飞机的坐标
//int x = 350, y = 0;
void Draw(IMAGE plane,IMAGE back_img) {
	putimage(0, 0, &back_img);
	putimage(my_plane.x ,my_plane.y , &plane);
}

int main() {
	//初始化窗口
	initgraph(700,700);
	IMAGE back_img;
	loadimage(&back_img,"map1.jpg",700,700,true);
	//loadimage(&back_img, "control_plane1.png", 50, 50,true);
	putimage(0,0,&back_img);


	//初始化游戏
	IMAGE plane;
	loadimage(&plane, "control_plane1.png",50,50);
	my_plane.x = 325;
	my_plane.y = 650;
	//要把飞机背景变成透明的
	putimage(my_plane.x, my_plane.y, &plane);

	while (1) {
		//char input =toupper( _getch());
		//printf("%c\n", input);
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
			if (my_plane.x >650 )my_plane.x = 650;
			break;
		case 'J':
			/*输入J，意味着要飞机要发射子弹攻击敌人*/
			


			break;
		default:
			break;
		}
		Draw(plane,back_img);
	}
	
	system("pause");
	return 0;
}