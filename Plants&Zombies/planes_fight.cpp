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
	int y = 0;					//�ɻ�������
	bool is_alive = true;	//���ɻ��Ƿ�������������ȥ
	int hp=1000;					//�ɻ�Ѫ��
	int type;				//��¼�л���С
	int weight;				//��¼�ɻ���С
	int height;
};

plane my_plane;				//�ҷ��ɻ�
plane bullet[Bullet_num];	//�ӵ�
plane enemy[Enemy_num];		//�з��ɻ�

//�ҷ��ɻ�������
//int x = 350, y = 0;
void Draw(IMAGE plane,IMAGE back_img) {
	putimage(0, 0, &back_img);
	putimage(my_plane.x ,my_plane.y , &plane);
}

int main() {
	//��ʼ������
	initgraph(700,700);
	IMAGE back_img;
	loadimage(&back_img,"map1.jpg",700,700,true);
	//loadimage(&back_img, "control_plane1.png", 50, 50,true);
	putimage(0,0,&back_img);


	//��ʼ����Ϸ
	IMAGE plane;
	loadimage(&plane, "control_plane1.png",50,50);
	my_plane.x = 325;
	my_plane.y = 650;
	//Ҫ�ѷɻ��������͸����
	putimage(my_plane.x, my_plane.y, &plane);

	while (1) {
		//char input =toupper( _getch());
		//printf("%c\n", input);
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
			if (my_plane.x >650 )my_plane.x = 650;
			break;
		case 'J':
			/*����J����ζ��Ҫ�ɻ�Ҫ�����ӵ���������*/
			


			break;
		default:
			break;
		}
		Draw(plane,back_img);
	}
	
	system("pause");
	return 0;
}