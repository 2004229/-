#include<stdio.h>
#include<graphics.h>
#include"tools.h"
#include"vector2.h"
#include<time.h>
#include<mmsystem.h>
#include<math.h>
#pragma comment(lib,"winmm.lib")

#define WIN_WIDTH 900
#define WIN_HEIGHT 600
#define ZM_MAX 20

enum {WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};//ö��ֲ������

IMAGE imgBg;
IMAGE imgBar;
IMAGE imgCards[ZHI_WU_COUNT];
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];
IMAGE imgSunshineBall[29];
IMAGE imgZmStand[11];

int curX, curY;//�ƶ�ֲ�ﵱǰ��λ��
int curZhiWu;//0,û��ѡ�У�1��ѡ���һ��ֲ��Դ�����
int sunshine;//����ֵ

enum {GOING,WIN,FAIL};
int killCount;
int zmCount;
int gameStatus;

struct zhiwu{
	int type;//0,û��ѡ�У�1��ѡ���һ��ֲ��
	int frameIndex;//�ڼ�֡
	bool catched;//����ʬ����
	int deadTime;//������ʱ��������ΪѪ��
	int timer;
	int x, y;
	int shoottime;
	int sunshineprice;
};
struct zhiwu map[3][9];//ֲ�����µ�λ��

//�ӵ���������
struct bullet {
	int x, y;
	bool used;
	int speed;
	int row;//�ӵ���
	bool blast;//�Ƿ�ը
	int frameIndex;
};
struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];

struct sunshineBall {
	int x, y;//��������
	int frameIndex;//��ǰ��ʾ��ͼƬ֡���
	int destY;//�������y����
	bool used;//�Ƿ���ʹ��
	int timer;//��¼���������ʱ��
	float xoff;//������ƫ����
	float yoff;
	float t;//������ʱ��㣬0-1
	vector2 p1, p2, p3, p4;//�ĸ����Ƶ�
	vector2 pCur;//���⵱ǰλ��
	float speed;//�ٶ�
	int status;//��ǰ��״̬
};
struct sunshineBall balls[10];//�����
enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };

struct zm {//��ʬ�Ľṹ��
	int x, y;
	int frameIndex;
	bool used;
	int speed;//�ٶ�
	int row;
	int blood;
	bool dead;
	bool eating;
};
struct zm zms[10];
IMAGE imgZM[22];
IMAGE imgZMDEAD[20];
IMAGE imgZMEat[21];

void collectSunshine(ExMessage* msg){//�ռ�����
	int count = sizeof(balls) / sizeof(balls[0]);//������������
	int w = imgSunshineBall[0].getwidth();//��ȡ����ͼƬ�Ŀ������
	int h = imgSunshineBall[0].getheight();
	for(int i = 0; i < count; i++) {//��������������ͣ������������ı䵱ǰ����ʹ��״̬������ֵ+25��������Ч
		if (balls[i].used) {
			if (msg->x) {
				int x = balls[i].pCur.x;
				int y = balls[i].pCur.y;
				if (msg->x > x && msg->x<x + w &&
					msg->y>y && msg->y < y + h) {
					balls[i].status = SUNSHINE_COLLECT;
					
					PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC);
					balls[i].p1 = balls[i].pCur;
					balls[i].p4 = vector2( 262,0 );
					balls[i].t = 0;
					float distance = dis(balls[i].p1 - balls[i].p4);
					float off = 8;
					balls[i].speed = 1.0 / (distance/off);
					break;
				}
			}
		}
	}
}

void userClick() {//������ĺ���
	ExMessage msg;//��Ϣ���͵ı��������������Ϣ
	static int status = 0;//�ж��Ƿ����ѡ�У�Ĭ��û��
	if (peekmessage(&msg)) {//���������Ϣ
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338&& msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {//�ж�����ڿ���������
				int index = (msg.x - 338) / 65;//��¼��ǰ���ѡ��Ŀ��ı��
				//printf("%d\n", index);
				status = 1;
				curZhiWu = index + 1;//ȷ�ϱ�ѡ�У���ȷ���ǵڼ���ֲ��
			}
			else {
				collectSunshine(&msg);//�ռ�����
			}
		}
		else if(msg.message==WM_MOUSEMOVE&&status==1){//��ǰһ���жϵĻ����ϣ�����ƶ�
			curX = msg.x;//��¼�������
			curY = msg.y;
		}
		else if (msg.message==WM_LBUTTONUP&&status==1) {//����뿪

			if (msg.x > 256-112 && msg.y > 179 && msg.y < 489) {//�������ֲ����
				int row = (msg.y - 179) / 102;//������
				int col = (msg.x - 256+112) / 81;//������
				//printf("%d,%d\n", row, col);
				if (map[row][col].type == 0) {
					map[row][col].type = curZhiWu;//ǰ����жϼ��㵱ǰ��ֲ������
					if (map[row][col].type==1) {//�����㶹������������
						if (sunshine < 100) {
							map[row][col].type = 0;
							curZhiWu = 0;//���ò���
							status = 0;

						}
						else {
							sunshine -= 100;
							map[row][col].frameIndex = 0;
							map[row][col].shoottime == 0;
							map[row][col].x = 256 - 112 + col * 81;
							map[row][col].y = 179 + row * 102 + 14;

						}
					}
					else if (map[row][col].type == 2) {//����̫������������
						if (sunshine < 50) {
							map[row][col].type = 0;
							curZhiWu = 0;//���ò���
							status = 0;
						}
						else {
							sunshine -= 50;
							map[row][col].frameIndex = 0;
							map[row][col].shoottime == 0;
							map[row][col].x = 256 - 112 + col * 81;
							map[row][col].y = 179 + row * 102 + 14;
						}
					}
					//map[row][col].frameIndex = 0;
					//map[row][col].shoottime == 0;
					//map[row][col].x = 256 -112+ col * 81;
					//map[row][col].y = 179 + row * 102 + 14;
				}
			}
			curZhiWu = 0;//���ò���
			status = 0;
		}
	}
}

bool fileExist(const char* name) {//�ж��ļ��Ƿ��ܴ򿪵ĺ���
	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}

void gameInit() {//��ʼ����ϷͼƬ�ز�
	//������Ϸ����
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar,"res/bar5.png");
	memset(imgZhiWu, 0, sizeof(imgZhiWu));//���ָ��
	memset(map, 0, sizeof(map));
	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;;

	//��ʼ��ֲ�￨��
	char name[64];//ֲ�￨�Ƶ��ļ���
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i,j + 1);
			//�ж��ļ��Ƿ����
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;//�����ڴ�
				loadimage(imgZhiWu[i][j], name);
			}
			else {
				break;
			}
		}
	}
	curZhiWu = 0;
	sunshine = 50;

	memset(balls, 0, sizeof(balls));//�������ڴ�ȫ����Ϊ0
	for (int i = 0; i < 29; i++) {//��ʼ������
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
		//printf("%d\n", i);
	}

	//�����������
	srand(time(NULL));

	//������Ϸ����
	initgraph(WIN_WIDTH,WIN_HEIGHT);

	//��������
	LOGFONT f;
	gettextstyle(&f);//��ȡ��ǰ����
	f.lfHeight = 30;//������
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");//�����ļ���
	f.lfQuality = ANTIALIASED_QUALITY;//�����
	settextstyle(&f);//��������
	setbkmode(TRANSPARENT);//�������屳��Ϊ͸��
	setcolor(BLACK);//������ɫ��ɫ

	//��ʼ����ʬ����
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM[i], name);
	}
	//��ʼ���㶹�ӵ�
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[3].getwidth() * k,
			imgBulletBlast[3].getheight() * k, true);
	}
	for (int i = 0; i < 20; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png",i + 1);
		loadimage(&imgZMDEAD[i], name);
	}
	for (int i = 0; i < 21; i++) {
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}
	for (int i = 0; i < 11; i++) {
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZmStand[i], name);
	}
}

void dramZM() {//��Ⱦ��ʬ
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			//IMAGE* img = &imgZM[zms[i].frameIndex];
			//IMAGE* img = (zms[i].dead) ? imgZMDEAD : imgZM;
			IMAGE* img = NULL;
			if (zms[i].dead) img = imgZMDEAD;
			else if (zms[i].eating) img = imgZMEat;
			else img = imgZM;

			img += zms[i].frameIndex;

			putimagePNG(zms[i].x, zms[i].y  , img);//������
			//-img->getheight()

		}
	}
}

void drawSunShine() {//��Ⱦ����
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		//if (balls[i].used || balls[i].xoff) {
		if(balls[i].used){
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			//putimagePNG(balls[i].x, balls[i].y, img);
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
			//printf("%d", i);
		}
	}
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(280, 67, scoreText);//�������
}

void drawCrads() {//��Ⱦֲ�￨��
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 338 + i * 64;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
}

void drawzhiwu() {
	//��Ⱦ�϶��е�ֲ��,��Ⱦ����ֲ��
	if (curZhiWu) {
		putimagePNG(curX - 37, curY - 37, imgZhiWu[curZhiWu - 1][0]);
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				int zhiwuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				putimagePNG(map[i][j].x, map[i][j].y, imgZhiWu[zhiwuType][index]);
			}
		}
	}
}

void drawbullets() {
	int bulletsMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletsMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
}

void updateWindow() {//��ȾͼƬ
	BeginBatchDraw();//˫����
	putimage(-112, 0, &imgBg);//��Ⱦ����
	putimagePNG(250,0,&imgBar);//��Ⱦֲ���б�

	drawCrads();//��Ⱦֲ�￨��
	drawzhiwu();//��Ⱦ�϶��е�ֲ��
	drawSunShine();	//��Ⱦ����
	drawbullets();//��Ⱦ�ӵ�
	dramZM();//��Ⱦ��ʬ
	EndBatchDraw();//����˫����
}

void createSunshine() {//��������
	static int count = 0;
	static int fre = 100;
	count++;
	if (count >= fre) {
		fre = 100 + rand() % 100;
		count = 0;
		//���������ȡ����
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax) return;
		//������Ȼ����
		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 -112+ rand() % (900 - 340+112), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;//���������ܾ���
		balls[i].speed = 1.0 / (distance / off);//�����ٶ�
	}

	//���տ���������
	int ballmax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == XIANG_RI_KUI + 1) {
				map[i][j].timer++;
				if (map[i][j].timer > 200) {
					map[i][j].timer = 0;
					int k;
					for (k = 0; k < ballmax && balls[k].used; k++);
					if (k >= ballmax) return;
					balls[k].used = true;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);//�������
					int w = (rand() % 50 + 100) * (rand() % 2 ? 1 : -1);
					balls[k].p4 = vector2(map[i][j].x + w,
						map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() - 
						imgSunshineBall[0].getheight());//�����յ�
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);//����p2
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);//����P3
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;
					balls[k].t = 0;
					balls[k].timer = 0;
				}
			}
		}
	}
}

void updateSunshine() {//����������Ϣ
	
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	//ExMessage msg;
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;//ʵ��ͼƬ֡��1-29ѭ��
			if (balls[i].status == SUNSHINE_DOWN) {//������Ȼ����
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {//��غ������������ڣ�״̬�ı�Ϊͣ��
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {//����ͣ��
				balls[i].timer++;
				if (balls[i].timer > 500) {
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {//�ռ�����
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) {
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {//���տ���������
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2,  sun->p3, sun->p4);
				//printf("���տ�������(%d,%d)��������\n",balls[i].pCur.x,balls[i].pCur.y);
				//printf("t��ֵΪ%f", balls[i].t);
				if (sun->t > 1) {
					//printf("�ı�������������\n");
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
				 
			}
		}
	}
}

void createZM() {
	if (zmCount >= ZM_MAX) {
		return;
	}
	static int zmFre = 200;
	static int count = 0;
	count++;
	if (count > zmFre) {
		count = 0;
		zmFre = rand() % 100 + 300;
		int i;
		int zmMAX = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMAX && zms[i].used; i++);
		if (i < zmMAX) {
			memset(&zms[i], 0, sizeof(zms[i]));
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;
			zms[i].y = ((zms[i].row) * 100) + 132;
			zms[i].speed = 1;
			zms[i].blood = 200;
			zms[i].dead = false;
			zms[i].eating = false;
			zmCount++;
		}
	}
}

void updateZM() {
	
	int zmMAX = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
	count++;
	if (count > 4) {
		count = 0;
		for (int i = 0; i < zmMAX; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 56) {
					//printf("GAME OVER\n");
					//MessageBox(NULL, "over", "over", 0);//���Ż�
					//exit(0);//���Ż�
					gameStatus = FAIL;
				}
			}
		}
	}
	//����λ��
	static int count2 = 0;
	count2++;
	if (count2 > 3) {
		count2 = 0;
		for (int i = 0; i < zmMAX; i++) {
			if (zms[i].used) {
				if (zms[i].dead) {//��ʬ��������״̬������һ�鶯����ı�ʹ��״̬Ϊfalse
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {
						zms[i].used = false;
						killCount++;
						if (killCount == ZM_MAX) {
							gameStatus = WIN;
						}
					}
				}
				else if (zms[i].eating) {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else {
				zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				//��ʬ���ڴ��״̬����0-21֡ѭ������
				}
			}
		}
	}
	
}

void shoot() {
	static int count = 0;
	if (++count <= 1)return;
	count = 0;
	int lines[3] = { 0 };//�Ƿ��н�ʬԽ��������
	int zmCount = sizeof(zms) / sizeof(zms[0]);//��ʬ����
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//�ӵ�����
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();//������
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {//�ж��Ƿ��н�ʬԽ��������
			lines[zms[i].row] = 1;
		}
	}
	for (int i = 0; i < 3; i++) {//����27����ֲλ��
		for (int j = 0; j < 9; j++) {
			if ((map[i][j].type == WAN_DOU + 1)&&lines[i]) {//&&zms[i].x>(j*81+256),��ӵ��жϣ��޷�ʵ��
				//�ҵ��㶹������ǰ���н�ʬ����ʬ�����㶹����
				//�Ż����󣬲�ȷ��bug������
				//static int count = 0;//�����ӵ�������
				//count++;
				map[i][j].shoottime++;
				if (map[i][j].shoottime > 25) {
					map[i][j].shoottime = 0;//���÷���
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 4;
						int zwx = 256-112 + j * 81;//����ֲ������
						int zwy = 179 + i * 102 + 14;
						bullets[k].x = zwx + imgZhiWu[map[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = zwy + 5;//�����ӵ�����
						bullets[k].blast = false;
						bullets[i].frameIndex = 0;
					}
				}
			}
		}
	}
}

void updateBullets() {//�����ӵ�λ��
	static int count = 0;
	if (++count < 2)return;
	count = 0;
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}
			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;

				}
			}
		}
	}
}

void checkBulletZm() {//�ӵ��Խ�ʬ����ײ���
	int bCount = sizeof(bullets) / sizeof(bullets[0]);//�ӵ�����
	int zCount = sizeof(zms) / sizeof(zms[0]);//��ʬ����

	for (int i = 0; i < bCount; i++) {
		if (bullets[i].used == false || bullets[i].blast) continue;
		for (int k = 0; k < zCount; k++) {
			if (zms[k].used == false) continue;
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2) {
				zms[k].blood -= 20;
				bullets[i].blast = true;
				bullets[i].speed = 0;

				if (zms[k].blood <= 0) {
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
				}
				break;
			}
		}
	}
}

void checkZmZhiWu() {//��ʬ��ֲ�����ײ���
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zCount; i++) {
		if (zms[i].dead) continue;
		int row = zms[i].row;
		for (int k = 0; k < 9; k++) {
			if (map[row][k].type == 0) continue;
			int zhiwux = 256-112 + k * 81;
			int x1 = zhiwux + 10;
			int x2 = zhiwux + 60;
			int x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2) {
				if (map[row][k].catched) {
					//zms[i].frameIndex++;
					map[row][k].deadTime -= 2;
					if (map[row][k].deadTime <=0) {
						map[row][k].deadTime = 0;
						map[row][k].type = 0;
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;

					}
				}
				else {
					map[row][k].catched = true;
					map[row][k].deadTime = 200;
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;
				}
			}
		}

	}
}

void collisionCheck() {//��ײ���
	checkBulletZm();
	checkZmZhiWu();
}

void updatezhiwu() {//���³���ֲ�ﶯ��
	static int count = 0;
	if (++count < 2)return;
	count = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				map[i][j].frameIndex++;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgZhiWu[zhiWuType][index] == NULL) {
					map[i][j].frameIndex = 0;
				}
			}
		}
	}
}

void updategame() {//������Ϸ����
	//static int count = 0;
	//if (++count < 2)return;
	//count = 0;
	updatezhiwu();

	createSunshine();
	updateSunshine();

	createZM();
	updateZM();

	shoot();
	updateBullets();//�����ӵ�

	collisionCheck();
}



void startUI() {//�����˵�
	IMAGE imgBg,imgMenu1,imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	mciSendString("open res/bg.mp3 alias bkmusic", NULL, 0, NULL);
	mciSendString("play bkmusic repeat", NULL, 0, NULL);
	int flag = 0;
	while (1) {
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(475, 75, flag ? &imgMenu2 : &imgMenu1);//nb
		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_MOUSEMOVE ){
				if (msg.x > 474 && msg.x < 474 + 300 &&
					msg.y>75 && msg.y < 75 + 140) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
			else if (msg.message == WM_LBUTTONDOWN&& flag==1 ) {
				EndBatchDraw();
				break;
			}
		}
		EndBatchDraw();
	}
}

void viewScence() {//ʵ����Ϸ����ʱ��Ѳ��
	int xmin = WIN_WIDTH - imgBg.getwidth();//-500
	vector2 points[9] = { {550,80},{530,160},{630,170},{530,200},{515,270},
		{565,370},{605,340},{705,280},{690,340} };
	int index[9];
	for (int i = 0; i < 9; i++) {
		index[i] = rand() % 11;
	}

	int count = 0;
	for (int x = 0; x >= xmin; x -= 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		count++;
		for (int k = 0; k < 9; k++){
			putimagePNG(points[k].x - xmin + x, 
				points[k].y,
				&imgZmStand[index[k]]);
			if (count >= 10) {
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10)count = 0;
		EndBatchDraw();
		Sleep(5);
	}
	//ͣ��
	for (int i = 0; i < 100; i++) {
		BeginBatchDraw();
		putimage(xmin, 0, &imgBg);
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x, points[k].y, &imgZmStand[index[k]]);
			index[k] = (index[k] + 1) % 11;

		}
		EndBatchDraw();
		Sleep(25);
	}
	//Ѳ��
	for (int x = xmin; x <= -112; x += 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		count++;
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xmin + x,
				points[k].y,
				&imgZmStand[index[k]]);
			if (count >= 10) {
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10)count = 0;
		EndBatchDraw();
		Sleep(5);
	}
}

void basrdown() {//ֲ���б����»���
	int height = imgBar.getheight();
	for (int y = -height; y<=0; y++) {
		BeginBatchDraw();
		putimage(-112, 0, &imgBg);
		putimagePNG(250, y, &imgBar);
		for (int i = 0; i < ZHI_WU_COUNT; i++) {
			int x = 338 + i * 65;
			putimage(x, 6+y, &imgCards[i]);
		}
		EndBatchDraw();
		Sleep(2);
	}
}

bool checkOver() {//�ж���Ϸ��ʤ�����
	bool ret = false;
	if (gameStatus == WIN) {
		Sleep(2000);
		loadimage(0, "res/win2.png");
		
		ret = true;
	}
	else if(gameStatus==FAIL){
		Sleep(2000);
		loadimage(0, "res/fail2.png");
		ret = true;
	}
	return ret;
}

int main() {
	gameInit();
	startUI();
	viewScence();

	basrdown();
	int timer = 0;
	bool flag = true;
	while (1)
	{
		userClick();
		timer += getDelay();
		if (timer > 25) {
			flag = true;
			timer = 0;
		}
		if (flag) {
			flag = false;
			updateWindow();
			updategame();
			if(checkOver())break;
		}
	}
	system("pause");
	return 0;
}