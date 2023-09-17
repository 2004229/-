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

enum {WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};//枚举植物种类

IMAGE imgBg;
IMAGE imgBar;
IMAGE imgCards[ZHI_WU_COUNT];
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];
IMAGE imgSunshineBall[29];
IMAGE imgZmStand[11];

int curX, curY;//移动植物当前的位置
int curZhiWu;//0,没有选中，1，选择第一种植物，以此类推
int sunshine;//阳光值

enum {GOING,WIN,FAIL};
int killCount;
int zmCount;
int gameStatus;

struct zhiwu{
	int type;//0,没有选中，1，选择第一种植物
	int frameIndex;//第几帧
	bool catched;//被僵尸攻击
	int deadTime;//死亡计时，后续改为血量
	int timer;
	int x, y;
	int shoottime;
	int sunshineprice;
};
struct zhiwu map[3][9];//植物种下的位置

//子弹数据类型
struct bullet {
	int x, y;
	bool used;
	int speed;
	int row;//子弹行
	bool blast;//是否爆炸
	int frameIndex;
};
struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];

struct sunshineBall {
	int x, y;//阳光坐标
	int frameIndex;//当前显示的图片帧序号
	int destY;//阳光落点y坐标
	bool used;//是否在使用
	int timer;//记录阳光球存在时间
	float xoff;//阳光球偏移量
	float yoff;
	float t;//贝塞尔时间点，0-1
	vector2 p1, p2, p3, p4;//四个控制点
	vector2 pCur;//阳光当前位置
	float speed;//速度
	int status;//当前的状态
};
struct sunshineBall balls[10];//阳光池
enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };

struct zm {//僵尸的结构体
	int x, y;
	int frameIndex;
	bool used;
	int speed;//速度
	int row;
	int blood;
	bool dead;
	bool eating;
};
struct zm zms[10];
IMAGE imgZM[22];
IMAGE imgZMDEAD[20];
IMAGE imgZMEat[21];

void collectSunshine(ExMessage* msg){//收集阳光
	int count = sizeof(balls) / sizeof(balls[0]);//计算阳光总数
	int w = imgSunshineBall[0].getwidth();//获取阳光图片的宽高像素
	int h = imgSunshineBall[0].getheight();
	for(int i = 0; i < count; i++) {//如果鼠标在阳光上停留，并点击，改变当前阳光使用状态，阳光值+25，播放音效
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

void userClick() {//检测鼠标的函数
	ExMessage msg;//信息类型的变量，接收鼠标信息
	static int status = 0;//判断是否被鼠标选中，默认没有
	if (peekmessage(&msg)) {//接收鼠标信息
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338&& msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {//判断鼠标在卡牌区域按下
				int index = (msg.x - 338) / 65;//记录当前鼠标选中目标的标号
				//printf("%d\n", index);
				status = 1;
				curZhiWu = index + 1;//确认被选中，且确定是第几种植物
			}
			else {
				collectSunshine(&msg);//收集阳光
			}
		}
		else if(msg.message==WM_MOUSEMOVE&&status==1){//在前一个判断的基础上，鼠标移动
			curX = msg.x;//记录鼠标坐标
			curY = msg.y;
		}
		else if (msg.message==WM_LBUTTONUP&&status==1) {//鼠标离开

			if (msg.x > 256-112 && msg.y > 179 && msg.y < 489) {//鼠标在种植区域
				int row = (msg.y - 179) / 102;//行坐标
				int col = (msg.x - 256+112) / 81;//列坐标
				//printf("%d,%d\n", row, col);
				if (map[row][col].type == 0) {
					map[row][col].type = curZhiWu;//前面的判断计算当前的植物种类
					if (map[row][col].type==1) {//计算豌豆射手阳光消耗
						if (sunshine < 100) {
							map[row][col].type = 0;
							curZhiWu = 0;//重置参数
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
					else if (map[row][col].type == 2) {//计算太阳花阳光消耗
						if (sunshine < 50) {
							map[row][col].type = 0;
							curZhiWu = 0;//重置参数
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
			curZhiWu = 0;//重置参数
			status = 0;
		}
	}
}

bool fileExist(const char* name) {//判断文件是否能打开的函数
	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}

void gameInit() {//初始化游戏图片素材
	//加载游戏背景
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar,"res/bar5.png");
	memset(imgZhiWu, 0, sizeof(imgZhiWu));//清空指针
	memset(map, 0, sizeof(map));
	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;;

	//初始化植物卡牌
	char name[64];//植物卡牌的文件名
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i,j + 1);
			//判断文件是否存在
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;//分配内存
				loadimage(imgZhiWu[i][j], name);
			}
			else {
				break;
			}
		}
	}
	curZhiWu = 0;
	sunshine = 50;

	memset(balls, 0, sizeof(balls));//将数组内存全设置为0
	for (int i = 0; i < 29; i++) {//初始化阳光
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
		//printf("%d\n", i);
	}

	//配置随机种子
	srand(time(NULL));

	//创建游戏窗口
	initgraph(WIN_WIDTH,WIN_HEIGHT);

	//设置字体
	LOGFONT f;
	gettextstyle(&f);//获取当前字体
	f.lfHeight = 30;//字体宽高
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");//字体文件名
	f.lfQuality = ANTIALIASED_QUALITY;//抗锯齿
	settextstyle(&f);//设置字体
	setbkmode(TRANSPARENT);//设置字体背景为透明
	setcolor(BLACK);//字体颜色黑色

	//初始化僵尸数据
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM[i], name);
	}
	//初始化豌豆子弹
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

void dramZM() {//渲染僵尸
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

			putimagePNG(zms[i].x, zms[i].y  , img);//有问题
			//-img->getheight()

		}
	}
}

void drawSunShine() {//渲染阳光
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
	outtextxy(280, 67, scoreText);//输出分数
}

void drawCrads() {//渲染植物卡牌
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 338 + i * 64;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
}

void drawzhiwu() {
	//渲染拖动中的植物,渲染场上植物
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

void updateWindow() {//渲染图片
	BeginBatchDraw();//双缓冲
	putimage(-112, 0, &imgBg);//渲染背景
	putimagePNG(250,0,&imgBar);//渲染植物列表

	drawCrads();//渲染植物卡牌
	drawzhiwu();//渲染拖动中的植物
	drawSunShine();	//渲染阳光
	drawbullets();//渲染子弹
	dramZM();//渲染僵尸
	EndBatchDraw();//结束双缓冲
}

void createSunshine() {//创建阳光
	static int count = 0;
	static int fre = 100;
	count++;
	if (count >= fre) {
		fre = 100 + rand() % 100;
		count = 0;
		//从阳光池提取阳光
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax) return;
		//阳光自然掉落
		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 -112+ rand() % (900 - 340+112), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;//阳光下落总距离
		balls[i].speed = 1.0 / (distance / off);//下落速度
	}

	//向日葵生产阳光
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
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);//计算起点
					int w = (rand() % 50 + 100) * (rand() % 2 ? 1 : -1);
					balls[k].p4 = vector2(map[i][j].x + w,
						map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() - 
						imgSunshineBall[0].getheight());//计算终点
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);//计算p2
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);//计算P3
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;
					balls[k].t = 0;
					balls[k].timer = 0;
				}
			}
		}
	}
}

void updateSunshine() {//更新阳光信息
	
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	//ExMessage msg;
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;//实现图片帧在1-29循环
			if (balls[i].status == SUNSHINE_DOWN) {//阳光自然下落
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {//落地后重置生命周期，状态改变为停留
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {//阳光停留
				balls[i].timer++;
				if (balls[i].timer > 500) {
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {//收集阳光
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) {
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {//向日葵生产阳光
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2,  sun->p3, sun->p4);
				//printf("向日葵在坐标(%d,%d)产生阳光\n",balls[i].pCur.x,balls[i].pCur.y);
				//printf("t的值为%f", balls[i].t);
				if (sun->t > 1) {
					//printf("改变阳光生命周期\n");
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
					//MessageBox(NULL, "over", "over", 0);//待优化
					//exit(0);//待优化
					gameStatus = FAIL;
				}
			}
		}
	}
	//更新位置
	static int count2 = 0;
	count2++;
	if (count2 > 3) {
		count2 = 0;
		for (int i = 0; i < zmMAX; i++) {
			if (zms[i].used) {
				if (zms[i].dead) {//僵尸处于死亡状态，播放一遍动画后改变使用状态为false
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
				//僵尸处于存活状态，由0-21帧循环播放
				}
			}
		}
	}
	
}

void shoot() {
	static int count = 0;
	if (++count <= 1)return;
	count = 0;
	int lines[3] = { 0 };//是否有僵尸越过警戒线
	int zmCount = sizeof(zms) / sizeof(zms[0]);//僵尸数量
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//子弹数量
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();//警戒线
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {//判断是否有僵尸越过警戒线
			lines[zms[i].row] = 1;
		}
	}
	for (int i = 0; i < 3; i++) {//遍历27个种植位置
		for (int j = 0; j < 9; j++) {
			if ((map[i][j].type == WAN_DOU + 1)&&lines[i]) {//&&zms[i].x>(j*81+256),添加的判断，无法实现
				//找到豌豆，而且前面有僵尸，僵尸不在豌豆后面
				//优化过后，不确定bug存在性
				//static int count = 0;//设置子弹发射间隔
				//count++;
				map[i][j].shoottime++;
				if (map[i][j].shoottime > 25) {
					map[i][j].shoottime = 0;//重置发射
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 4;
						int zwx = 256-112 + j * 81;//计算植物坐标
						int zwy = 179 + i * 102 + 14;
						bullets[k].x = zwx + imgZhiWu[map[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = zwy + 5;//计算子弹坐标
						bullets[k].blast = false;
						bullets[i].frameIndex = 0;
					}
				}
			}
		}
	}
}

void updateBullets() {//更新子弹位置
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

void checkBulletZm() {//子弹对僵尸的碰撞检测
	int bCount = sizeof(bullets) / sizeof(bullets[0]);//子弹数量
	int zCount = sizeof(zms) / sizeof(zms[0]);//僵尸数量

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

void checkZmZhiWu() {//僵尸对植物的碰撞检测
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

void collisionCheck() {//碰撞监测
	checkBulletZm();
	checkZmZhiWu();
}

void updatezhiwu() {//更新场上植物动作
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

void updategame() {//更新游戏数据
	//static int count = 0;
	//if (++count < 2)return;
	//count = 0;
	updatezhiwu();

	createSunshine();
	updateSunshine();

	createZM();
	updateZM();

	shoot();
	updateBullets();//更新子弹

	collisionCheck();
}



void startUI() {//启动菜单
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

void viewScence() {//实现游戏进入时的巡场
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
	//停留
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
	//巡回
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

void basrdown() {//植物列表向下滑出
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

bool checkOver() {//判断游戏的胜利与否
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