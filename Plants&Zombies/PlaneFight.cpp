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
	Bullet_speed = 12,		//�ӵ��ƶ��ٶ�
	Plane_speed = 10,		//�ɻ��ƶ��ٶ�
	Enemy_num = 6,			//�л�����
	Enemy_speed = 1,		//�л��ٶ�
};

typedef enum {
	ENEMY = 1,
	HERO = 0
}plane_type;

//�����ķɻ��ṹ�壬�����ҷ��ɻ��͵з��ɻ�
typedef struct plane {
	int x = 0;				//�ɻ�������
	int y = 0;				//�ɻ�������
	int is_alive = 0;		//���ɻ��Ƿ���/�Ƿ����
	int hp = 1000;			//�ɻ�Ѫ��
	plane_type type ;		//��¼�ɻ�����
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
IMAGE img_emegy[7];			//�洢�з��ɻ�ͼ��
IMAGE img_bullet[2];		//�洢�ӵ�ͼƬ

static IMAGE now_bk = back_img[0];		//���ڸ��ķɻ�ͼƬ�ͱ���ͼƬ
static IMAGE now_plane = img_plane[1];
int enemyFreqs[GUAN_QIA_COUNT] = { 100,50,25 };//�л�����Ƶ��
bool updata = false;

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
	loadimage(img_emegy + 5, "small_emegy_1.png");//С�л�
	loadimage(img_emegy + 6, "small_emegy_1.png");
	//�ӵ�ͼƬ
	loadimage(img_bullet, "shot_7.png");
	loadimage(img_bullet + 1, "shot_2_10.png");
}

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
	initgraph(Width, Height);
	//����ͼƬ
	load_image();
	putimage(0, 0, &back_img[0]);
	//��ʼ���ӵ�
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].is_alive = 0;
		bullet[i].zd_img[0] = img_bullet[0];
		bullet[i].zd_img[1] = img_bullet[1];
		bullet[i].height = img_bullet[0].getheight();
		bullet[i].width = img_bullet[0].getwidth();
	}
	//��ʼ���л�
	for (int i = 0; i < Enemy_num; i++) {
		enemy[i].is_alive = 0;
		enemy[i].plane_img = img_emegy[rand()%7];
		enemy[i].height = enemy[i].plane_img.getheight();
		enemy[i].width = enemy[i].plane_img.getwidth();
		enemy[i].type = ENEMY;;
	}
	//��ʼ���ҷ��ɻ�
	my_plane.height = img_plane[0].getheight();
	my_plane.width = img_plane[0].getwidth();
	my_plane.type = HERO;
	my_plane.is_alive = 1;
	my_plane.x = Width / 2 - my_plane.width / 2;
	my_plane.y = my_plane.height;
}

//�����л�
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
					int zdX1 = bullet[i].x + bullet[i].width/4;
					int zdX2 = zdX1 + bullet[i].width * 0.75;
					int zdY1 = bullet[i].y + bullet[i].height / 4;
					int zdY2 = zdY1 + bullet[i].height * 0.75;
					int emX1 = enemy[j].x + enemy[j].width / 4;
					int emX2 = emX1 + enemy[j].width * 0.75;
					int emY1 = enemy[j].y + enemy[j].height / 4;
					int emY2 = emY1 + enemy[j].height * 0.75;

					if (is_overlap(zdX1, zdX2, zdY1, zdY2, emX1, emX2, emY1, emY2)) {
						bullet[i].is_alive = 0; //�ӵ�����
						enemy[j].hp -= 300; //�л�Ѫ������
					}
				}
			}
		}
	}
}

//�õл�λ�øı�
void fly() {
	//���µл�״̬
	for (int i = 0; i < Enemy_num; i++) {
		if (enemy[i].is_alive) {
			//���·���λ��
			enemy[i].y += 4;
			if (enemy[i].y > Height) {
				enemy[i].y = -100;
				enemy[i].is_alive = 0;
				continue;
			}
		}
	}
	//�����ӵ�״̬
	for (int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			//bullet[i].x = (bullet[i].x + 1) % 2;
			bullet[i].y -= Bullet_speed;
		}
	}//�����л�
	static int frameCount = 0;
	frameCount++;
	if (frameCount >= 50) {
		frameCount = 0;
		create_emegy();
	}
	//��ײ���
	is_collide();
}

//���·ɻ�λ��
void update_plane() {
	//����Ӣ�۷ɻ�
	if (my_plane.is_alive) {
		drawPNG(my_plane.x, my_plane.y, &now_plane);
	}
	//���µл�
	for (int i = 0; i < Enemy_num; i++) {
		if (enemy[i].is_alive) {
			drawPNG(enemy[i].x, enemy[i].y, &enemy[i].plane_img);
		}
	}
}

//�ɻ��¼�
void plane_event() {
	if (_kbhit()) {
		/*IMAGE now_bk = back_img[0];
		IMAGE now_plane = img_plane[0];*/
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
		//case 'J':
		//	/*����J����ζ��Ҫ�ɻ�Ҫ�����ӵ���������*/
		//	create_bullet();
		//	break;
		//case 'C':
		//	/*����C����ζ��Ҫ�л���ͼ*/
		//	if (compare_img(now_bk, back_img[0])) {
		//		//�����ʱ������Ϊ��ʼ��������ô����CӦ���л�������һ�ű���ͼ
		//		now_bk = back_img[1];
		//	}
		//	else {
		//		//�����ʱ������Ϊ��ʼ��������ô����CӦ���л�����ʼ����ͼ�������л��ɻ�Ƥ��ͬ��
		//		now_bk = back_img[0];
		//	}
		//	//putimage(0, 0, &now_bk);
		//	break;
		//case 'P':
		//	/*����P����ζ��Ҫ�л��ɻ�Ƥ��*/
		//	if (compare_img(now_plane, img_plane[0])) {
		//		now_plane = img_plane[rand() % 3];
		//	}
		//	else {
		//		now_plane = img_plane[0];
		//	}
		//	//putimage(0, 0, &now_bk);
		//	break;
		case 'Q':
			/*����Q����ζ��Ҫ�˳���Ϸ*/
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