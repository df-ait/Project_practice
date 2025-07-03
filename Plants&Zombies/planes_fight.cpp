#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <tchar.h>//Ϊ��д��ͬʱ����ANSI(���ֽ��ַ���)����Unicode����Ĵ��룬Ϊ������TCHAR����

//��ֹVS����Ϊscanf��������ȫ���Ӷ�������
#define _CRT_SECURE_NO_WARNINGS
#define GUAN_QIA_COUNT 3
#define boom_pic_num 6
#define plane_total_hp  50000

static const int MAX = 100;//������

//����ö��
enum GameInform {
	Width = 700,			//���
	Height = 700,			//�߶�
	Bullet_num = 30,		//�ӵ���Ŀ
	Bullet_speed = 7,		//�ҷ��ӵ��ƶ��ٶ�
	Plane_speed = 5,		//�ɻ��ƶ��ٶ�
	Enemy_bullet_speed = 3,	//�з��ӵ��ٶ�
	Enemy_num = 8,			//�л�����
	Enemy_speed = 2,		//�л��ٶ�
};

typedef enum {
	BIG_ENEMY = 3,
	MIDDLE_ENEMY = 2,
	SMALL_ENEMY = 1,
	HERO = 0,
}plane_type;

enum GameState {
	MAIN_MENU = 0,	//���˵�
	PLAYING,		//��Ϸ������
	GAME_OVER,			//��Ϸ��ͣ
};

GameState game_status;	//��Ϸ����״̬

typedef struct BOOM
{
	IMAGE  boom_png[boom_pic_num] ;	//��ը��ЧͼƬ
	int all_frame = 0;				//��֡��
	int current_frame = 0;			//��ǰ֡��
	unsigned int last_time = 0;	//��һ֡ʱ��
	bool is_playing = false;	//�Ƿ����ڲ���
	int width = 0;				//ÿһ֡�Ŀ��
	int height = 0;				//ÿһ֡�ĸ߶�
	int frame_dalay = 100;		//���֡��100ms
	int x;						//���Ʊ�ը��Ч������
	int y;
};

BOOM png_boom;		//���屬ը��Ч

typedef struct zd {
	int x = 0;				//������
	int y = 0;				//������
	int is_alive = 0;		//����Ƿ�ʹ��
	IMAGE zd_img[2];		//�ɻ�ͼƬ
	int width;				//�ɻ����
	int height;				//�ɻ��߶�
};

//�����ķɻ��ṹ�壬�����ҷ��ɻ��͵з��ɻ�
typedef struct plane {
	int x = 0;				//�ɻ�������
	int y = 0;				//�ɻ�������
	int is_alive = 0;		//���ɻ��Ƿ���/�Ƿ����
	int hp = 0;				//�ɻ�Ѫ��
	plane_type type;		//��¼�ɻ�����
	IMAGE plane_img;		//�ɻ�ͼƬ
	int width;				//�ɻ����
	int height;				//�ɻ��߶�
	zd bullet[Bullet_num];	//�ɻ��ӵ�
};

plane my_plane;				//�ҷ��ɻ�
zd bullet[Bullet_num];		//�ӵ�
plane enemy[Enemy_num];		//�з��ɻ�

IMAGE back_img[3];				//�洢����ͼ
IMAGE img_plane[4];			//�洢�ҷ��ɻ�ͼ��
IMAGE img_emegy[6];			//�洢�з��ɻ�ͼ��
IMAGE img_bullet[2];		//�洢�ҷ��ӵ�ͼƬ
IMAGE img_enemy_bullet;		//�洢�з��ӵ�ͼƬ

int enemyFreqs[GUAN_QIA_COUNT] = { 100,50,25 };//�л�����Ƶ��
bool gameOver = false;	//��Ϸ������־
bool gameStart = true;	//��Ϸ��ʼ��־

//����������Ҳ��������ӵ�
static unsigned long long lastShootTime = 0;
const int SHOOT_COOLDOWN = 200; // 300������ȴʱ��

/**************************�ҷ������з�******************/
/**************************�з������ҷ�*****************/
//�����з��ӵ�

//��������
//�޸��˵İ汾�������˶�ͼƬ�ļ��ͱ߽紦����ͼƬ�Ƿ���Ч�Լ�������ָ���Ƿ���Ч�ֿ�����ˣ����������Ժ�õ��ı��������׼ȷ
/*GetImageBufer()�������ܻ������ָ��NULL�������Դ治���ʱ�򣩣���ʱֱ�ӷ��ʾ����ڷ���һ����ָ�룬�ᵼ�·��ʳ�ͻ
*/
void drawPNG(int picture_x, int picture_y, IMAGE* picture)//xΪ����ͼƬ��X���꣬yΪY����
{
		// ��ǿ�������
		if (picture == NULL || picture->getwidth() == 0 || picture->getheight() == 0) {
			//printf("���棺���Ի�����Ч��ͼƬ��\n");
			return;
		}

		// ��ȡ������ָ�벢�����Ч��
		DWORD* dst = GetImageBuffer(NULL);
		DWORD* src = GetImageBuffer(picture);
		if (dst == NULL || src == NULL) {
			//printf("���棺�޷���ȡͼ�񻺳���ָ�룡\n");
			return;
		}

		int picture_width = picture->getwidth();
		int picture_height = picture->getheight();
		int graphWidth = getwidth();
		int graphHeight = getheight();

		// ������λ���Ƿ���ȫ����Ļ��
		if (picture_x + picture_width < 0 || picture_x >= graphWidth ||
			picture_y + picture_height < 0 || picture_y >= graphHeight) {
			return;
		}

		// ����ʵ����Ҫ���Ƶ�����(���ⲿ������Ļ��ʱԽ��)
		int startX = max(0, -picture_x);//����picture_x���������
		int endX = min(picture_width, graphWidth - picture_x);
		int startY = max(0, -picture_y);//����picture_y���������
		int endY = min(picture_height, graphHeight - picture_y);

		for (int iy = startY; iy < endY; iy++) {
			for (int ix = startX; ix < endX; ix++) {
				int srcX = ix + iy * picture_width;
				int sa = ((src[srcX] & 0xff000000) >> 24);
				if (sa == 0) continue; // ��ȫ͸��������

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
		if (picture == NULL) return;

		// �����ȫ����Ļ���򲻻���
		if (x > getwidth() || y > getheight() ||
			x + picture->getwidth() < 0 || y + picture->getheight() < 0) {
			return;
		}

		// ��������Ļ������òü��汾
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
			delete imgTmp;//��ʽ���ͷ��������
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
	//�з��ӵ�ͼƬ	
	loadimage(&img_enemy_bullet, "emegy_shot.png");
	//��ը��ЧͼƬ
	loadimage(png_boom.boom_png, "boom_1.png");
	loadimage(png_boom.boom_png+1, "boom_2.png");
	loadimage(png_boom.boom_png+2, "boom_3.png");
	loadimage(png_boom.boom_png+3, "boom_4.png");
	loadimage(png_boom.boom_png+4, "boom_5.png");
	loadimage(png_boom.boom_png+5, "boom_6.png");
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

//��ʼ���з��ӵ�
void init_e_bullet(zd* bullet) {
	for (int i = 0; i < Bullet_num; i++) {
		(bullet + i)->is_alive = 0;
		(bullet + i)->height = img_enemy_bullet.getheight();
		(bullet + i)->width = img_enemy_bullet.getwidth();
		(bullet + i)->zd_img[0] = img_enemy_bullet;
		(bullet + i)->zd_img[1] = img_enemy_bullet;
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
	my_plane.hp = plane_total_hp; //�ҷ��ɻ�Ѫ��
	//init_bullet(my_plane.bullet);
	drawPNG2(my_plane.x, my_plane.y, img_plane);
	//��ʼ���л�
	for (int i = 0; i < Enemy_num; i++) {
		enemy[i].is_alive = 0;
		int kind = rand() % 6; //���ѡ��л�����
		enemy[i].plane_img = img_emegy[kind];
		enemy[i].height = enemy[i].plane_img.getheight();
		enemy[i].width = enemy[i].plane_img.getwidth();
		init_e_bullet(enemy[i].bullet);
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

//������ը����
void create_boom(int x, int y) {
	png_boom.all_frame = 6; //��֡��
	png_boom.current_frame = 0; //��ǰ֡��
	png_boom.last_time = GetTickCount(); //��һ֡ʱ��
	png_boom.is_playing = true; //��ʼ����
	png_boom.width = png_boom.boom_png[0].getwidth();
	png_boom.height = png_boom.boom_png[0].getheight();
	png_boom.x = x - png_boom.width/2; //��ը��Ч��x����
	png_boom.y = y - png_boom.height / 2; //��ը��Ч��y����
}

//���±�ը��Ч
void update_boom() {
	if (png_boom.is_playing) {
		unsigned int current_time = GetTickCount();
		if (current_time - png_boom.last_time >= png_boom.frame_dalay) {
			png_boom.last_time = current_time;
			png_boom.current_frame++;
			//���Ž����ж�
			if (png_boom.current_frame >= png_boom.all_frame) {
				png_boom.is_playing = false; //�������
				return;
			}
		}
	}
}

//�����л�
void create_emegy() {
	for (int i = 0; i < Enemy_num; i++) {
		if (!enemy[i].is_alive) {
			enemy[i].is_alive = 1;
			enemy[i].x = rand() % (Width - enemy[i].width);
			enemy[i].y = -10;
			//printf("�л�����enemy[%d] x = %d, y = %d\n", i, enemy[i].x, enemy[i].y);
			if (enemy[i].width >= 80)
			{
				enemy[i].type = BIG_ENEMY;
				enemy[i].hp = 1000; //�л�Ѫ��
			}
			else if (enemy[i].width >= 60 && enemy[i].width < 80)
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
			create_boom(enemy[i].x + enemy[i].width / 2, enemy[i].y + enemy[i].height / 2);
			enemy[i].is_alive = 0; //�л�Ѫ��С�ڵ���0������
			//printf("�л�[%d]������\n", i);
		}
	}
}

//�����ص��жϺ���,��A[x1_1,y1_1,x1_2,y1_2]  B[x2_1, y2_1, x2_2, y2_2].
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
					//����ӵ��͵л��Ƿ���ײ
					if (is_overlap(zdX1, zdX2, zdY1, zdY2, emX1, emX2, emY1, emY2)) {
						bullet[i].is_alive = 0; //�ӵ�����
						//printf("�ӵ�[%d]��ײ�л�[%d]\n", i, j);
						enemy[j].hp -= 300; //�л�Ѫ������
					}
				}
			}
		}
	}
	//Ӣ�۷ɻ�����ײ�䷶Χ����ά��
	int myX1 = my_plane.x + my_plane.width / 4;
	int myX2 = myX1 + my_plane.width * 0.75;
	int myY1 = my_plane.y + my_plane.height / 4;
	int myY2 = myY1 + my_plane.height * 0.75;
	//���л��ӵ���Ӣ�۷ɻ��Ƿ���ײ
	//ÿһ���л���ÿһ��is_alive==1���ӵ���Ҫ��Ӣ�۷ɻ�����Ƿ���ײ
	//for (int j = 0; j < Enemy_num; j++)
	//{//���ѭ���ǵл�
	//	if (!enemy[j].is_alive) continue; //����л������ڣ�������
		for(int i = 0 ; i< Enemy_num ; i++){
			if (!enemy[i].is_alive) continue; //����л������ڣ�������
			for(int j =  0; j < Bullet_num; j++) {
				if (enemy[i].bullet[j].is_alive) {
					int em_bulletX1 = enemy[i].bullet[j].x + enemy[i].bullet[j].width / 4;
					int em_bulletX2 = em_bulletX1 + enemy[i].bullet[j].width * 0.75;
					int em_bulletY1 = enemy[i].bullet[j].y + enemy[i].bullet[j].height / 4;
					int em_bulletY2 = em_bulletY1 + enemy[i].bullet[j].height * 0.75;
					//����ӵ���Ӣ�۷ɻ��Ƿ���ײ
					if (is_overlap(myX1, myX2, myY1, myY2, em_bulletX1, em_bulletX2, em_bulletY1, em_bulletY2)) {
						my_plane.hp -= 50; //Ӣ�۷ɻ�Ѫ������
						enemy[i].bullet[j].is_alive = 0; //�л��ӵ�����
						printf("Ӣ�۷ɻ�hp -50\n");
						if (my_plane.hp <= 0) {
							my_plane.is_alive = 0; //Ӣ�۷ɻ�����
							create_boom(my_plane.x+my_plane.width/2, my_plane.y+my_plane.height/2); //������ը��Ч
							printf("Ӣ�۷ɻ�������\n");
							gameOver = true; //��Ϸ����
							return;
						}
					}
				}
			}
	}
	//���л���Ӣ�۷ɻ��Ƿ���ײ
	for (int j = 0; j < Enemy_num; j++) {
		if (enemy[j].is_alive) {
			int emX1 = enemy[j].x + enemy[j].width / 4;
			int emX2 = emX1 + enemy[j].width * 0.75;
			int emY1 = enemy[j].y + enemy[j].height / 4;
			int emY2 = emY1 + enemy[j].height * 0.75;
			//����ӵ��͵л��Ƿ���ײ
			if (is_overlap(myX1, myX2, myY1, myY2, emX1, emX2, emY1, emY2)) {
				my_plane.hp -= 20; //Ӣ�۷ɻ�Ѫ������
				enemy[j].hp -= 20; //�л�Ѫ������
				printf("Ӣ�۷ɻ�hp -20");
				//printf("Ӣ�۷ɻ���ײ�л�[%d]\n", j);
				if (my_plane.hp <= 0) {
					my_plane.is_alive = 0; //Ӣ�۷ɻ�����
					create_boom(my_plane.x + my_plane.width / 2, my_plane.y + my_plane.height / 2);
					printf("Ӣ�۷ɻ�������\n");
					gameOver = true; //��Ϸ����
					return;
				}
			}
		}
	}
}

//�����ӵ�
void create_bullet() {
	//�����ӵ�����Ƶ��
	unsigned long long currentTime = GetTickCount();
	if (currentTime - lastShootTime < SHOOT_COOLDOWN) {
		return; // ��ȴʱ��δ�������ܷ���
	}

	for (int i = 0; i < Bullet_num; i++) {
		if (!bullet[i].is_alive&&rand()%100 < 20) {
			bullet[i].is_alive = 1;
			bullet[i].x = my_plane.x + my_plane.width / 2 - bullet[i].width / 2;
			bullet[i].y = my_plane.y - bullet[i].height;
			//printf("�ӵ�[%d] x = %d, y = %d\n", i, bullet[i].x, bullet[i].y);
			lastShootTime = currentTime; // ��¼���η���ʱ��
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

void draw_blood() {
	int total_hp_long = 300;		//Ѫ���ܳ���
	int hp_height = 20;				//Ѫ���߶�
	int border = 2;					//Ѫ���߿���
	int x = 10, y = 10;				//Ѫ��λ��

	//����Ѫ������
	setfillcolor(RGB(100, 100, 100));//��ɫ����
	fillrectangle(x, y, x + total_hp_long, y + hp_height);
	//���㵱ǰѪ���ı�ֵ
	float hp_precent = 1.0f;
	if (my_plane.hp > 0) {
		hp_precent = (float)my_plane.hp / (float)plane_total_hp;//����Ѫ����ֵ
	}
	//����Ѫ���л�Ѫ����ɫ
	COLORREF hp_color;
	if (hp_precent > 0.6f) hp_color = GREEN;
	else if (hp_precent <= 0.6f && hp_precent > 0.3f) hp_color = YELLOW;
	else hp_color = RED;
	setfillcolor(hp_color);//����Ѫ����ɫ
	fillrectangle(x, y, x + total_hp_long * hp_precent, y + hp_height);
	//�������ֱ�ǩ
	settextstyle(12, 0, _T("����"));//�����ı������壬����ʹ�С�ĺ�����EasyX�Դ�
	settextcolor(WHITE);
	TCHAR label[32];
	_stprintf_s(label, _T("%d/%d"),my_plane.hp, plane_total_hp);
	outtextxy(x+100, y+4, label);
}

//���¸�ͼ��λ��(�ƶ�)
void Draw_plane() {
	//����Ӣ�۷ɻ�
	if(my_plane.is_alive) {
		putimage(0, 0, back_img);
		drawPNG2(my_plane.x, my_plane.y, img_plane);
		printf("Ӣ�۷ɻ���ǰѪ�� hp = %d\n", my_plane.hp);
	}
	//���Ƶл�
	for (int i = 0; i < Enemy_num; i++)
	{
		if (enemy[i].is_alive) {
			//printf("���Ƶл�[%d] x = %d, y = %d\n", i, enemy[i].x, enemy[i].y);
			drawPNG2(enemy[i].x, enemy[i].y, &enemy[i].plane_img);
		}
	}
	//�����ӵ�
	for(int i = 0; i < Bullet_num; i++) {
		if (bullet[i].is_alive) {
			drawPNG2(bullet[i].x, bullet[i].y, &bullet[i].zd_img[0]);
		}
	}
	//���Ƶл��ӵ�
	for (int i = 0; i < Enemy_num; i++)
	{
		if (!enemy[i].is_alive) continue; //����л������ڣ�������
		for(int j = 0; j < Bullet_num; j++) {
			if (enemy[i].bullet[j].is_alive) {
				drawPNG2(enemy[i].bullet[j].x, enemy[i].bullet[j].y, &img_enemy_bullet);
				//printf("���Ƶл�[%d]�ӵ� x = %d, y = %d\n", i, enemy_bullet[j].x, enemy_bullet[j].y);
			}
		}
	}
	//���Ʊ�ը��Ч
	if (png_boom.is_playing) {
		drawPNG2(png_boom.x, png_boom.y, &png_boom.boom_png[png_boom.current_frame]);
	}
	//��Ѫ��
	draw_blood();
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

//�����з��ӵ�
void create_emegy_bullet() {
	//��Ϊÿһ���л�������ô���ӵ������Ա������ел���ÿ���л����з����ӵ��Ļ��ᣬ����л��ӵ��ƶ�Ҳ�Ǹ��߼�
	for (int j = 0; j < Enemy_num; j++)
	{
		if (!enemy[j].is_alive) continue; //����л������ڣ�������
		for (int i = 0; i < Bullet_num; i++) {
			if (!enemy[j].bullet[i].is_alive && rand() % 1000 < 2) {//ǧ��֮���ĸ��ʷ����ӵ�
				enemy[j].bullet[i].is_alive = 1;
				enemy[j].bullet[i].x = enemy[j].x + enemy[j].width / 2 - enemy[j].bullet[i].width / 2;
				enemy[j].bullet[i].y = enemy[j].y + enemy[j].bullet[i].height;
				//printf("�з��ӵ�[%d] x = %d, y = %d\n", i, enemy_bullet[i].x, enemy_bullet[i].y);
				break;
			}
		}
	}
}

//�з��ӵ��ƶ�
void updata_enemy_bullet() {
	for (int j = 0; j < Enemy_num; j++)
	{
		if (!enemy[j].is_alive) continue; //����л������ڣ�������
		for (int i = 0; i < Bullet_num; i++) {
			if (enemy[j].bullet[i].is_alive) {
				if(enemy[j].bullet[i].y > Height- enemy[j].bullet[i].height) {
					enemy[j].bullet[i].is_alive = 0; //�ӵ��ɳ���Ļ������
					continue;
				}
				enemy[j].bullet[i].y += Enemy_bullet_speed; //�з��ӵ������ƶ�
			}
		}
	}
}

int main() {
	//��ʼ������
	initgraph(Width, Height);
	init_game();
	int time_count = 0;
	BeginBatchDraw();
	while (1) {
			Draw_plane();
			change_plane_event();
			time_count += timer();
			if (time_count >= enemyFreqs[1] || is_enegy_alive()) {
				create_emegy(); //�����л�
				updata_emegy();
				create_emegy_bullet(); //�����з��ӵ�
				is_collide(); //�����ײ
				updata_enemy_bullet(); //�з��ӵ��ƶ�
				time_count = 0;
			}
			updata_bullet(); //�ӵ��ƶ�
			is_collide(); //�����ײ
			update_boom(); //���±�ը��Ч
			FlushBatchDraw();
			Sleep(10); //��ʱ10����,����CPUռ�ù���
			if (gameOver) break;
	}
	EndBatchDraw();	
	system("pause");
	return 0;
}