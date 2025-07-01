#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <ctype.h>
//��ֹVS����Ϊscanf��������ȫ���Ӷ�������
#define _CRT_SECURE_NO_WARNINGS

enum My {
	Width = 700,			//���
	Height = 700,			//�߶�
	Bullet_num = 30,		//�ӵ���Ŀ
	Bullet_speed = 12,		//�ӵ��ƶ��ٶ�
	Plane_speed = 10,		//�ɻ��ƶ��ٶ�
	Enemy_num = 6,			//�л�����
	Enemy_speed = 1,		//�л��ٶ�
	Big,					//�л� ��
	Middle,					//�л� �е�
	Small					//�л� С
};

//�����ķɻ��ṹ�壬�����ҷ��ɻ��͵з��ɻ�
typedef struct plane{
	int x = 0;				//�ɻ�������
	int y = 0;				//�ɻ�������
	int is_alive = 1;		//���ɻ��Ƿ�������������ȥ
	int hp=1000;			//�ɻ�Ѫ��
	char type;				//��¼�л���С,B�����л���M�����еȵл���S����С�л�,M������ҷɻ�
	int weight;				//�ɻ����
	int height;				//�ɻ��߶�
};

plane my_plane;				//�ҷ��ɻ�
plane bullet[Bullet_num];	//�ӵ�
plane enemy[Enemy_num];		//�з��ɻ�

IMAGE back_img[2];				//�洢����ͼ
IMAGE img_plane[2];			//�洢�ҷ��ɻ�ͼ��
IMAGE img_enemy[4];			//�洢�з��ɻ�ͼ��
IMAGE img_bullet[2];		//�洢�ӵ�ͼƬ

//����ȫ������ͼƬ
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

//�ҷ��ɻ�������
//int x = 350, y = 0;
void Draw(IMAGE plane,IMAGE back_img) {
	putimage(0, 0, &back_img);
	putimage(my_plane.x ,my_plane.y , &plane);
}

void init_game() {
	//��������ͼƬ
	load_image();
	//loadimage(&back_img, "control_plane1.png", 50, 50,true);
	putimage(0, 0, back_img);
	my_plane.weight = 50;
	my_plane.height = 50;
	my_plane.type = 'M';
	//��ʼ���ӵ�
	for (int i = 0; i < Bullet_num; i++) {
		bullet[i].x = 0;
		bullet[i].y = 0;
		bullet[i].is_alive = 1;
	}
	//���÷ɻ�λ��,���ɻ�������Ϸ��������������м�
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
			/*����A����ζ��Ҫ�ɻ�������ƶ�����yֵ���䣬�ɻ�xֵ��С*/
			my_plane.x -= Plane_speed;
			//���Ʒɻ��߽�����������÷ɻ��ɳ���Ϸ�߽�
			if (my_plane.x < 0)my_plane.x = 0;
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
		case 'J':
			/*����J����ζ��Ҫ�ɻ�Ҫ�����ӵ���������*/



			break;
		case 'C':
			/*����C����ζ��Ҫ�л���ͼ*/
			if (compare_img(now_bk, back_img[0])) {
				//�����ʱ������Ϊ��ʼ��������ô����CӦ���л�������һ�ű���ͼ
				now_bk = back_img[1];
			}
			else {
				//�����ʱ������Ϊ��ʼ��������ô����CӦ���л�����ʼ����ͼ�������л��ɻ�Ƥ��ͬ��
				now_bk = back_img[0];
			}
			putimage(0, 0, &now_bk);
			break;
		case 'P':
			/*����P����ζ��Ҫ�л��ɻ�Ƥ��*/
			if (compare_img(now_plane, img_plane[0])) {
				now_plane = img_plane[1];
			}
			else {
				now_plane = img_plane[0];
			}
			putimage(0, 0, &now_bk);
			break;
		case 'Q':
			/*����Q����ζ��Ҫ�˳���Ϸ*/
			return;
		default:
			break;
		}
		Draw(now_plane, now_bk);
	}
}

int main() {
	//��ʼ������
	initgraph(Width,Height);
	//��ʼ����Ϸ
	init_game();
	//�����ǶԷɻ�or�����Ĳ���
	plane_move();
	//system("pause");
	return 0;
}