#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
//#include <iostream>
//#include <thread>
//��ֹVS����Ϊscanf��������ȫ���Ӷ�������
#define _CRT_SECURE_NO_WARNINGS
#define GUAN_QIA_COUNT 3

static const int MAX = 100;//������

//����ö��
enum GameInform {
	Width = 700,			//���
	Height = 700,			//�߶�
	Bullet_num = 30,		//�ӵ���Ŀ
	Bullet_speed = 7,		//�ӵ��ƶ��ٶ�
	Plane_speed = 5,		//�ɻ��ƶ��ٶ�
	Enemy_num = 8,			//�л�����
	Enemy_speed = 2,		//�л��ٶ�
};

typedef enum {
	BIG_ENEMY = 3,
	MIDDLE_ENEMY = 2,
	SMALL_ENEMY = 1,
	HERO = 0
}plane_type;

//�����ķɻ��ṹ�壬�����ҷ��ɻ��͵з��ɻ�
typedef struct plane {
	int x = 0;				//�ɻ�������
	int y = 0;				//�ɻ�������
	int is_alive = 0;		//���ɻ��Ƿ���/�Ƿ����
	int hp = 1000;			//�ɻ�Ѫ��
	plane_type type;		//��¼�ɻ�����
	IMAGE plane_img;		//�ɻ�ͼƬ
	int width;				//�ɻ����
	int height;				//�ɻ��߶�
};

typedef struct zd {
	int x = 0;				//������
	int y = 0;				//������
	int is_alive = 0;		//����Ƿ�ʹ��
	IMAGE zd_img[2];		//�ɻ�ͼƬ
	int width;				//�ɻ����
	int height;				//�ɻ��߶�
};

plane my_plane;				//�ҷ��ɻ�
zd bullet[Bullet_num];		//�ӵ�
plane enemy[Enemy_num];		//�з��ɻ�

IMAGE back_img[3];				//�洢����ͼ
IMAGE img_plane[4];			//�洢�ҷ��ɻ�ͼ��
IMAGE img_emegy[6];			//�洢�з��ɻ�ͼ��
IMAGE img_bullet[2];		//�洢�ӵ�ͼƬ

int enemyFreqs[GUAN_QIA_COUNT] = { 100,50,25 };//�л�����Ƶ��
bool updata = false;

//��������
void drawPNG(int picture_x, int picture_y, IMAGE* picture)//xΪ����ͼƬ��X���꣬yΪY����
{
	DWORD* dst = GetImageBuffer();
	//GetImageBuffer()���������ڻ�ȡ��ͼ�豸���Դ�ָ�룬EASYX�Դ�
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture);//��ȡpicture���Դ�ָ��
	int picture_width = picture->getwidth();//��ȡpicture�Ŀ�ȣ�EASYX�Դ�
	int picture_height = picture->getheight();//��ȡpicture�ĸ߶ȣ�EASYX�Դ�
	int graphWidth = getwidth();
	//��ȡ��ͼ���Ŀ�ȣ�EASYX�Դ�
	int graphHeight = getheight();
	//��ȡ��ͼ���ĸ߶ȣ�EASYX�Դ�
	int dstX = 0;
	//���Դ������صĽǱ�
	//ʵ��͸����ͼ��ʽ��Cp=��p*FP+(1-��p)*BP����Ҷ˹���������е���ɫ�ĸ��ʼ���
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width;//���Դ������صĽǱ�
			int sa = ((src[srcX] & 0xff000000) >> 24);//0xAArrggbb;AA��͸����
			int sr = ((src[srcX] & 0xff0000) >> 16);//��ȡRGB���R
			int sg = ((src[srcX] & 0xff00) >> 8);
			//G
			int sb = src[srcX] & 0xff;
			//B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <=
				graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth;//���Դ������صĽǱ�
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)
					//��ʽ��Cp = ��p * FP + (1 - ��p) * BP����p = sa / 255, FP = sr, BP = dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)
					//��p=sa/255,FP = sg, BP = dg
					| (sb * sa / 255 + db * (255 - sa) / 255);
				//��p=sa/255,FP = sb, BP = db
			}
		}
	}
}
//����߽�����ĸ�������
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

//��������2�����ڴ���߽�����µ�png͸��ͼ��
void drawPNG2(int x, int y, IMAGE* picture) {
	IMAGE imgTmp;
	if (y < 0 || y>Width) {
		SetWorkingImage(picture);
		getimage(&imgTmp, 0, -y,picture->getwidth(), picture->getheight() + y);
		SetWorkingImage();
		y = 0;
		drawPNG(x, 0, &imgTmp);
	}
	else {
		drawPNG(x, y, picture);
	}
}

//����ȫ������ͼƬ
void load_image() {
	//����ͼƬ
	loadimage(back_img, "map1.jpg", 700, 700, true);
	loadimage(back_img + 1, "map3.jpg", 700, 700, true);
	//��ҷɻ�ͼƬ
	loadimage(&img_plane[0], "player_1_50.png.");
	loadimage(&img_plane[1], "player_2_75.png");
	loadimage(&img_plane[2], "player_3_50.png");
	loadimage(&img_plane[3], "player_4_50.png");
	//�л�ͼƬ
	loadimage(img_emegy, "big_emegy_1.png");//��л�
	loadimage(img_emegy + 1, "big_emegy_2.png");
	loadimage(img_emegy + 2, "middle_emegy_1.png");//�ел�
	loadimage(img_emegy + 3, "middle_emegy_2.png");
	loadimage(img_emegy + 4, "middle_emegy_3.png");
	//loadimage(img_emegy + 6, "small_emegy_1.png");//С�л�
	loadimage(img_emegy + 5, "small_emegy_2.png");
	//�ӵ�ͼƬ
	loadimage(img_bullet, "shot_7.png");
	loadimage(img_bullet + 1, "shot_2_10.png");
}

//��ʱ��
int timer() {
	static unsigned long long lastTime = 0;
	unsigned long long currentTime = GetTickCount();
	//GetTickCount()����ϵͳ���������ڵĺ�����
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

//��ʼ����Ϸ
void init_game() {
	load_image();
	//���ñ���ͼƬ
	putimage(0, 0, back_img);
	//��ʼ���ҷ��ɻ�
	my_plane.height = img_plane[0].getheight();
	my_plane.width = img_plane[0].getwidth();
	my_plane.type = HERO;
	my_plane.is_alive = 1;
	my_plane.x = (Height / 2) - (my_plane.width / 2);
	my_plane.y = Height - my_plane.height;
	drawPNG2(my_plane.x, my_plane.y, img_plane);
	//��ʼ���л�
	for (int i = 0; i < Enemy_num; i++) {
		enemy[i].is_alive = 0;
		int kind = rand() % 6; //���ѡ��л�����
		enemy[i].plane_img = img_emegy[kind];
		enemy[i].height = enemy[i].plane_img.getheight();
		enemy[i].width = enemy[i].plane_img.getwidth();
		//printf("enemy[%d] width = %d, height = %d\t%d\n", i, enemy[i].width, enemy[i].height,kind);
		//drawPNG(20+50*i, 20+50*i, &enemy[i].plane_img);
	}
	//��ʼ���ӵ�
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].is_alive = 0;
		bullet[i].zd_img[0] = img_bullet[0];
		bullet[i].zd_img[1] = img_bullet[1];
		bullet[i].height = img_bullet[0].getheight();
		bullet[i].width = img_bullet[0].getwidth();
	}
}

//�����л�
void create_emegy() {
	for (int i = 0; i < Enemy_num; i++) {
		if (!enemy[i].is_alive) {
			enemy[i].is_alive = 1;
			enemy[i].x = rand() % (Width - enemy[i].width);
			enemy[i].y = -10;
			printf("�л�����enemy[%d] x = %d, y = %d\n", i, enemy[i].x, enemy[i].y);
			if (enemy[i].width >= 80)
			{
				enemy[i].type = BIG_ENEMY;
				enemy[i].hp = 1000; //�л�Ѫ��
			}
			else if(enemy[i].width>=60 && enemy[i].width < 80)
			{
				enemy[i].type = MIDDLE_ENEMY;
				enemy[i].hp = 600; //�л�Ѫ��
			}
			else
			{
				enemy[i].type = SMALL_ENEMY;
				enemy[i].hp = 300; //�л�Ѫ��
			}
			return;
		}
	}
}

//�л��ƶ�
void updata_emegy() {
	for (int i = 0; i < Enemy_num; i++) {
		if (enemy[i].is_alive) {
			if (enemy[i].y >= Width-enemy[i].height) {
				enemy[i].is_alive = 0; //�ɳ���Ļ������
				continue;
			}
			enemy[i].y+= Enemy_speed;
		}
		if(enemy[i].hp<=0) {
			enemy[i].is_alive = 0; //�л�Ѫ��С�ڵ���0������
			printf("�л�[%d]������\n", i);
		}
	}
}

//�����ص��жϺ���
//��A[x1_1,y1_1,x1_2,y1_2]  B[x2_1, y2_1, x2_2, y2_2].
bool is_overlap(int x1_1, int x1_2, int y1_1, int y1_2, int x2_1, int x2_2, int y2_1, int y2_2) {
	int zx = abs(x1_1 + x1_2 - x2_1 - x2_2);
	int x = abs(x1_1 - x1_2) + abs(x2_1 - x2_2);
	int zy = abs(y1_1 + y1_2 - y2_1 - y2_2);
	int y = abs(y1_1 - y1_2) + abs(y2_1 - y2_2);
	return
		(zx <= x && zy <= y);
}

//�����ײ����
void  is_collide() {
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
						bullet[i].is_alive = 0; //�ӵ�����
						printf("�ӵ�[%d]��ײ�л�[%d]\n", i, j);
						enemy[j].hp -= 300; //�л�Ѫ������
					}
				}
			}
		}
	}
}

//�����ӵ�
void create_bullet() {
	for (int i = 0; i < Bullet_num; i++) {
		if (!bullet[i].is_alive) {
			bullet[i].is_alive = 1;
			bullet[i].x = my_plane.x + my_plane.width / 2 - bullet[i].width / 2;
			bullet[i].y = my_plane.y - bullet[i].height;
			printf("�ӵ�[%d] x = %d, y = %d\n", i, bullet[i].x, bullet[i].y);
			return;
		}
	}
}

//�ӵ��ƶ�
void updata_bullet() {
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			if (bullet[i].y < 0) {
				bullet[i].is_alive = 0; //�ӵ��ɳ���Ļ������
				continue;
			}
			bullet[i].y -= Bullet_speed;
		}
	}
}

//���·ɻ���λ��(�ƶ�)
void Draw_plane() {
	//����Ӣ�۷ɻ�
	if(my_plane.is_alive) {
		putimage(0, 0, back_img);
		drawPNG2(my_plane.x, my_plane.y, img_plane);
	}
	//���Ƶл�
	for (int i = 0; i < Enemy_num; i++)
	{
		if (enemy[i].is_alive) {
			printf("���Ƶл�[%d] x = %d, y = %d\n", i, enemy[i].x, enemy[i].y);
			drawPNG2(enemy[i].x, enemy[i].y, &enemy[i].plane_img);
		}
	}
	//�����ӵ�
	for(int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			drawPNG2(bullet[i].x, bullet[i].y, &bullet[i].zd_img[0]);
		}
	}
}

//�ɻ��¼�
void plane_event() {
	switch (toupper(_getch())) {
		case 'A':
			/*����A����ζ��Ҫ�ɻ�������ƶ�����yֵ���䣬�ɻ�xֵ��С*/
			my_plane.x -= Plane_speed;
			//���Ʒɻ��߽�����������÷ɻ��ɳ���Ϸ�߽�
			if (my_plane.x < 0)my_plane.x = 0;
			//printf("%c", _getch());
			break;
		case 'S':
			/*����S����ζ��Ҫ�ɻ����±��ƶ�����xֵ���䣬�ɻ�yֵ����*/
			my_plane.y += Plane_speed;
			if (my_plane.y > 650)my_plane.y = 650;
			break;
		case 'W':
			/*����W����ζ��Ҫ�ɻ����ϱ��ƶ�����xֵ���䣬�ɻ�yֵ��С*/
			my_plane.y -= Plane_speed;
			if (my_plane.y < 0)my_plane.y = 0;
			break;
		case 'D':
			/*����D����ζ��Ҫ�ɻ����ұ��ƶ�����yֵ���䣬�ɻ�xֵ����*/
			my_plane.x += Plane_speed;
			if (my_plane.x > 650)my_plane.x = 650;
			break;
		case 'Q':
			/*����Q����ζ��Ҫ�˳���Ϸ*/
			return;
		case ' ':
			/*����ո���ζ��Ҫ�����ӵ�*/
			create_bullet(); //�����ӵ�
			updata_bullet(); //�ӵ��ƶ�
			return;
		default:
			break;
		}
}

//�޸��Ժ�ķɻ��¼�
void change_plane_event() {
	// �������
	if (GetAsyncKeyState('A') & 0x8000) {
		my_plane.x -= Plane_speed;
		//���Ʒɻ��߽�����������÷ɻ��ɳ���Ϸ�߽�
		if (my_plane.x < 0)my_plane.x = 0;
	}
	if (GetAsyncKeyState('W') & 0x8000) {
		/*����W����ζ��Ҫ�ɻ����ϱ��ƶ�����xֵ���䣬�ɻ�yֵ��С*/
		my_plane.y -= Plane_speed;
		if (my_plane.y < 0)my_plane.y = 0;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		/*����S����ζ��Ҫ�ɻ����±��ƶ�����xֵ���䣬�ɻ�yֵ����*/
		my_plane.y += Plane_speed;
		if (my_plane.y > 650)my_plane.y = 650;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		/*����D����ζ��Ҫ�ɻ����ұ��ƶ�����yֵ���䣬�ɻ�xֵ����*/
		my_plane.x += Plane_speed;
		if (my_plane.x > 650)my_plane.x = 650;
	}
	if (GetAsyncKeyState(' ') & 0x8000) {
		/*����ո���ζ��Ҫ�����ӵ�*/
		create_bullet(); //�����ӵ�
		updata_bullet(); //�ӵ��ƶ�
	}
}

bool is_enegy_alive() {
	//���л��Ƿ�ȫ������
	for (int i = 0; i < Enemy_num; i++) {
		if (!enemy[i].is_alive) {
			return true;
		}
	}
	return false;
}


/**************************�ҷ������з�******************/
/**************************�з������ҷ�*****************/


int main() {
	initgraph(Width, Height);
	init_game();
	int time_count = 0;
	BeginBatchDraw();
	while (1) {
		Draw_plane();
		change_plane_event();
		time_count += timer();
		if(time_count >= enemyFreqs[1] || is_enegy_alive()) {
			create_emegy(); //�����л�
			updata_emegy();
			time_count = 0;
		}
		updata_bullet(); //�ӵ��ƶ�
		is_collide(); //�����ײ
		FlushBatchDraw();
		Sleep(10); //��ʱ10����,����CPUռ�ù���
	}
	EndBatchDraw();	
	system("pause");
	return 0;
}