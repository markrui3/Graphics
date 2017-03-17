#define GLUT_DISABLE_ATEXIT_HACK
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <gl/glew.h>
#include <gl/glext.h>
#include <gl/glut.h>
#include <gl/GLAUX.h>

#pragma comment(lib,"WinMM.Lib")
#pragma comment( lib, "glew32.lib")        // GLaux连接库
#pragma comment( lib, "glaux.lib")        // GLaux连接库

#define PI 3.141592
#define pa 8
#define MAXTEXTURE       4            // 定义纹理贴图数量
#define MAX_PARTICLES     10000         // 定义粒子数量
#define MAX_METEOR        7
#define MAX_DROP          100
using namespace std;

static int slices = 150;
static int stacks = 150;
static float c= PI/180.0f; //弧度和角度转换参数
static int du = 90,oldmy = -1,oldmx = -1; //du是视点绕y轴的角度,opengl里默认y轴是上方向
static float r = 2.35,h = 0.0f; //r是视点绕y轴的半径,h是视点高度即在y轴
static float width = 1365, height = 767;
bool isPers = true;
int isRun = 0;
static GLdouble xRot = 0.0, yRot = 0.0;
static double rotate_forward = 0, rotate_back = 0;
static int counts = 0, cycle = 0, steps = 0;
GLfloat speedFator = 1.0f;            // 粒子运动速度的控制因子 
GLfloat angle_Z;              // 粒子旋转
float pp = 0.1f ; 
GLfloat params[]={1.0,1.0,1.0,0.0};
GLuint ImageIDs[30];
AUX_RGBImageRec *Image0, *Image1, *Image2, *Image3, *Image4, *Image5, *Image6, *Image7, *Image8, *Image9, *Image10, *Image11, *Image12, *Image13, *Image14, *Image15, *Image16, *Image17, *Image18, *Image19, *Image20, *Image21, *Image22, *Image23, *Image24, *Image25, *Image26, *Image27, *Image28, *Image29;
GLuint v,f,f2,p;

PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = NULL;

typedef struct
{
	float life;              // 粒子生命
	float fade;              // 粒子衰减速度
	float r,g,b;              // 粒子的颜色
	float x,y,z;              // 粒子位置
	float xSpeed,ySpeed,zSpeed;          // 粒子的速度
	float xg,yg,zg;             // 粒子的运动加速度
} PARTICLES;

PARTICLES particle[MAX_PARTICLES] , meteor[MAX_METEOR] , flow[MAX_DROP];// 粒子系统数组

float m[2][100];

static void setortho();
static void setperspective();
static void resize(int, int);
void ParticleInit(void);
void DrawParticle(void);
void DrawMeteor(void);
void DrawFlow(void);
static void sweep();
int zuhe(int n,int k);
float fang(float n,int k);
float benkn(int n,int k,float t);
void move();
static void display();
void texture();
void ShutdownRC(void);
void keyboard1 (int key, int x, int y);
void keyboard2 (unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void onMouseMove(int x,int y);
static void idle(void);
static void setLight(void);

//正交投影函数
static void setortho()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho (-1.33, 1.33, -1, 1, 0, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//透视投影函数
static void setperspective()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.33, 1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	resize(width, height);
}

//设置人物随屏幕大小而变换
static void resize(int w, int h)
{
	const float ar = (float) w / (float) h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, ar, 1, 10);
	glFrustum(-ar, ar, -1, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// 初始化粒子系统
void ParticleInit(void)
{
	int i;
	for (int loop=0; loop<MAX_PARTICLES; ++loop)
	{
		particle[loop].life = 1.0f;          // 赋予每个粒子完整生命
		particle[loop].fade = float(rand()%100)/1000.0f+0.003f;   // 随机生命衰减速度
		particle[loop].r = 1.0f;      // 粒子颜色
		particle[loop].g = 1.0f;      // 粒子颜色
		particle[loop].b = 1.0f;      // 粒子颜色
		particle[loop].xSpeed = (float)100*((float)cos( rand()%MAX_PARTICLES * PI/360));   
		particle[loop].ySpeed = 600.0f;    
		particle[loop].zSpeed =  (float)100*((float)sin(rand()%MAX_PARTICLES * PI/360));   
		particle[loop].xg = 0.0f;          // 沿x轴的加速度
		particle[loop].yg = -6.0f;          // 沿y轴的加速度
		particle[loop].zg = 0.0f;          // 沿z轴的加速度
	}
	for ( i = 0 ; i < MAX_METEOR ; ++ i )
	{
		meteor[i].life = 1.0f;          // 赋予每个粒子完整生命
		meteor[i].fade = float(rand()%100)/1000.0f+0.003f;   // 随机生命衰减速度
		meteor[i].r = 1.0f;      // 粒子颜色的红色分量
		meteor[i].g = 1.0f;      // 粒子颜色的绿色分量
		meteor[i].b = 1.0f;      // 粒子颜色的蓝色分量
		meteor[i].xSpeed = float((rand()%300)-150.0f);    // 沿x轴的随机速度(-150,150)
		meteor[i].ySpeed = float((rand()%300)-150.0f);    // 沿y轴的随机速度(-150,150)
		meteor[i].zSpeed = float(rand()%150);      // 沿z轴的随机速度(0,150)
		meteor[i].xg = 4.0f;          // 沿x轴的加速度
		meteor[i].yg = 4.0f;          // 沿y轴的加速度
		meteor[i].zg = 2.0f;          // 沿z轴的加速度
	}
	for ( i = 0 ; i < MAX_DROP ; ++ i )
	{
		flow[i].life = 1.0f;          // 赋予每个粒子完整生命
		flow[i].fade = float(rand()%100)/1000.0f+0.003f;   // 随机生命衰减速度
		flow[i].r = 1.0f;      // 粒子颜色的红色分量
		flow[i].g = 1.0f;      // 粒子颜色的绿色分量
		flow[i].b = 1.0f;      // 粒子颜色的蓝色分量
		flow[i].xg = 0.0f;          // 沿x轴的加速度
		flow[i].yg = 0.0f;          // 沿y轴的加速度
		flow[i].zg = 0.0f;          // 沿z轴的加速度
	}

}

void DrawParticle(void)
{

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);         // 选择混合方式
	float x,y,z;
	for (int loop=0; loop<MAX_PARTICLES; ++loop)      // 循环绘制
	{
		x = particle[loop].x;           // 定义粒子坐标
		y = particle[loop].y;
		z = particle[loop].z;
		// 用glColor4f绘制粒子的颜色和透明度(生命),用life减fade即可得到粒子的生命变化
		glColor4f(particle[loop].r, particle[loop].g, particle[loop].b, particle[loop].life);

		glEnable(GL_TEXTURE_2D); 
		glBindTexture(GL_TEXTURE_2D,ImageIDs[21]);
		glBegin(GL_TRIANGLE_STRIP);          // 绘制粒子
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + 0.5f, y + 0.5f, z);
		glTexCoord2d(0.0f, 1.0f); glVertex3f(x - 0.5f, y + 0.5f, z);
		glTexCoord2d(1.0f, 0.0f); glVertex3f(x + 0.5f, y - 0.5f, z);
		glTexCoord2d(0.0f, 0.0f); glVertex3f(x - 0.5f, y - 0.5f, z);
		glEnd();
		glDisable(GL_TEXTURE_2D); 

		// 粒子位置更新  
		if( particle[loop].y > -12.0f ){
			particle[loop].x += particle[loop].xSpeed/(speedFator*1000.0f); // x轴上的位置
			particle[loop].y += particle[loop].ySpeed/(speedFator*1000.0f); // y轴上的位置
			particle[loop].z += particle[loop].zSpeed/(speedFator*1000.0f); // z轴上的位置
		}else{
			particle[loop].x += 0.0f;                      // x轴上的位置
			particle[loop].y += 0.0f ;                     // y轴上的位置
			particle[loop].z += 0.0f;                       // z轴上的位置
		}


		// 粒子速度更新
		particle[loop].xSpeed += particle[loop].xg;
		particle[loop].ySpeed += particle[loop].yg;
		particle[loop].zSpeed += particle[loop].zg;
		particle[loop].life -= particle[loop].fade;      // 粒子生命的衰减
		if (particle[loop].life <= 0.0f)        // 如果粒子衰减到0
		{
			particle[loop].life = 1.0f;         // 赋予新生命
			particle[loop].fade = float(rand()%100)/1000.0f+0.003f;  // 随机衰减速度
			particle[loop].x = 0.0f;         // 新粒子在(0.0f,0,0f,0.0f)处
			particle[loop].y = -10.0f;
			particle[loop].z = 0.0f;
			particle[loop].xSpeed = (float)100*((float) cos( rand()%MAX_PARTICLES * PI / 360 ) ) ;   
			particle[loop].ySpeed = 600.0f;    
			particle[loop].zSpeed = (float)100*( (float) cos( rand()%MAX_PARTICLES * PI / 360 ) );     // z轴速度与方向
			particle[loop].r = 1.0f ;    
			particle[loop].g = 1.0f ;
			particle[loop].b = 1.0f ;
		}
	}
}

void DrawMeteor(void)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);         // 选择混合方式
	float x,y,z;
	for (int loop=0; loop<MAX_METEOR; ++loop)      // 循环绘制
	{
		x = meteor[loop].x;           // 定义粒子坐标
		y = meteor[loop].y;
		z = meteor[loop].z;

		// 用glColor4f绘制粒子的颜色和透明度(生命),用life减fade即可得到粒子的生命变化
		glColor4f(meteor[loop].r, meteor[loop].g, meteor[loop].b, meteor[loop].life);

		glEnable(GL_TEXTURE_2D); 
		glBindTexture(GL_TEXTURE_2D,ImageIDs[22]);
		glBegin(GL_TRIANGLE_STRIP);          // 绘制粒子
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + 1.5f, y + 1.5f, z);
		glTexCoord2d(0.0f, 1.0f); glVertex3f(x - 1.5f, y + 1.5f, z);
		glTexCoord2d(1.0f, 0.0f); glVertex3f(x + 1.5f, y - 1.5f, z);
		glTexCoord2d(0.0f, 0.0f); glVertex3f(x - 1.5f, y - 1.5f, z);
		glEnd();
		glDisable(GL_TEXTURE_2D); 

		// 粒子位置更新
		meteor[loop].x += meteor[loop].xSpeed/(speedFator*1000.0f); // x轴上的位置
		meteor[loop].y += meteor[loop].ySpeed/(speedFator*1000.0f); // y轴上的位置
		meteor[loop].z += meteor[loop].zSpeed/(speedFator*1000.0f); // z轴上的位置

		// 粒子速度更新
		meteor[loop].xSpeed += meteor[loop].xg;
		meteor[loop].ySpeed += meteor[loop].yg;
		meteor[loop].zSpeed += meteor[loop].zg;
		meteor[loop].life -= meteor[loop].fade;      // 粒子生命的衰减

		if (meteor[loop].life <= 0.0f)        // 如果粒子衰减到0
		{
			meteor[loop].life = 1.0f;         // 赋予新生命
			meteor[loop].fade = float(rand()%100)/1000.0f+0.003f;  // 随机衰减速度
			meteor[loop].x = 2.0f;         // 新粒子在(0.0f,0,0f,0.0f)处
			meteor[loop].y = 2.0f;
			meteor[loop].z = -1.0f;
			meteor[loop].xSpeed = -float(rand()%60);   // x轴速度与方向
			meteor[loop].ySpeed = -float(rand()%60);   // y轴速度与方向
			meteor[loop].zSpeed = float(rand()%10);     // z轴速度与方向
			meteor[loop].r = 1.0f;     // 选择随机颜色的(r,g,b)值
			meteor[loop].g = 1.0f;
			meteor[loop].b = 1.0f;
		}
	}


}
void DrawFlow(void)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);         // 选择混合方式
	float x,y,z;
	for (int loop=0; loop < MAX_DROP; ++loop)      // 循环绘制
	{
		x = flow[loop].x;           // 定义粒子坐标
		y = flow[loop].y;
		z = flow[loop].z;

		// 用glColor4f绘制粒子的颜色和透明度(生命),用life减fade即可得到粒子的生命变化
		glColor4f(flow[loop].r, flow[loop].g, flow[loop].b, flow[loop].life);


		for( int j = -80 ;j < -20 ; ++ j )
		{
			glEnable(GL_TEXTURE_2D); 
			glBindTexture(GL_TEXTURE_2D,ImageIDs[21]);
			glBegin(GL_TRIANGLE_STRIP);          // 绘制粒子
			glTexCoord2f(1.0f, 1.0f); glVertex3f(x + 0.5f, y + 0.5f, j);
			glTexCoord2d(0.0f, 1.0f); glVertex3f(x - 0.5f, y + 0.5f, j);
			glTexCoord2d(1.0f, 0.0f); glVertex3f(x + 0.5f, y - 0.5f, j);
			glTexCoord2d(0.0f, 0.0f); glVertex3f(x - 0.5f, y - 0.5f, j);
			glEnd();
			glDisable(GL_TEXTURE_2D); 
		}


		flow[loop].x = -27.0f + loop * 0.54 ;        // x轴上的位置  

		flow[loop].y = 0.5 * cos(  flow[loop].x / 1.5 + pp) -4.0f ;            // y轴上的位置

		pp+= 0.001f;

		// 粒子更新

		flow[loop].life -= flow[loop].fade;      // 粒子生命的衰减
		if (flow[loop].life <= 0.0f)        // 如果粒子衰减到0
		{
			flow[loop].life = 1.0f;         // 赋予新生命
			flow[loop].fade = float(rand()%100)/1000.0f+0.003f;  // 随机衰减速度

			flow[loop].r = 1.0f ;    
			flow[loop].g = 1.0f ;
			flow[loop].b = 1.0f ;
		}
	}

}

//小黄人的嘴部连线
static void sweep()
{
	float p0[] = {-0.12, 0.1, 0.28};
	float p1[] = {-0.08, 0.084, 0.29};
	float p2[] = {-0.04, 0.077, 0.3};
	float p3[] = {-0.03, 0.072, 0.3};
	float p4[] = {0, 0.07, 0.3};
	float p5[] = {0.03, 0.072, 0.3};
	float p6[] = {0.04, 0.077, 0.3};
	float p7[] = {0.08, 0.084, 0.29};
	float p8[] = {0.12, 0.1, 0.28};

	glVertex3fv(p0);
	glVertex3fv(p1);
	glVertex3fv(p2);
	glVertex3fv(p3);
	glVertex3fv(p4);
	glVertex3fv(p5);
	glVertex3fv(p6);
	glVertex3fv(p7);
	glVertex3fv(p8);
}

int zuhe(int n,int k)
{
	int i,s1,s2;
	s1=1;
	s2=1;
	if(k==0) return 1;
	for(i=n;i>=n-k+1;i--) s1=s1*i;
	for(i=k;i>=2;i--) s2=s2*i;
	return s1/s2;
}
float fang(float n,int k)
{
	if(k==0) return 1;
	return pow(n,k);
}

float benkn(int n,int k,float t)
{
	return zuhe(n,k)*fang(t,k)*fang(1-t,n-k);
}

void move(){
	float t[11]={0},x1[11],y1[11],s=0.0;
	float x[4]={-0.1,-0.05,0.05,0.1},y[4]={-0.07,-0.1,-0.11,-0.07};
	int i;
	for(i=1;i<11;i++)  {s=s+0.1;t[i]=s;}

	for(i=0;i<11;i++)
	{
		x1[i]=x[0]*benkn(3,0,t[i])+x[1]*benkn(3,1,t[i])+x[2]*benkn(3,2,t[i])+x[3]*benkn(3,3,t[i]);
		y1[i]=y[0]*benkn(3,0,t[i])+y[1]*benkn(3,1,t[i])+y[2]*benkn(3,2,t[i])+y[3]*benkn(3,3,t[i]);
	}
	printf("%f,%f,%f,%f\n",x[0],x[1],x[2],x[3]);
	printf("%f,%f,%f,%f\n",y[0],y[1],y[2],y[3]);
	for(i=0;i<11;i++) 
	{
		printf("%5.2f",t[i]);
	}
	printf("\n");
	for(i=0;i<11;i++) 
	{
		printf("%5.2f",x1[i]);
	}
	printf("\n");
	for(i=0;i<11;i++) 
	{
		printf("%5.2f",y1[i]);
	}
	for(int i = 0; i < 11; i++){
		m[0][i] = x1[i];
		m[1][i] = y1[i];
	}




}
//人物、背景
static void display()
{
	glActiveTextureARB  = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	GLUquadricObj *obj0 = gluNewQuadric();
	GLUquadricObj *obj1 = gluNewQuadric();
	GLUquadricObj *obj2 = gluNewQuadric();
	GLUquadricObj *obj3 = gluNewQuadric();
	GLUquadricObj *obj4 = gluNewQuadric();
	GLdouble color[] = {1, 0.74, 0};

	gluLookAt( r * cos(c * du), h, r * sin(c * du),
		0, 0, 0,
		0, 1, 0);
	int time= glutGet(GLUT_ELAPSED_TIME) / 100.0;   // 获取当前时间
	time = time % (3*6*2+11*4);
	glPushMatrix();//最外面的pushmatrix，把所有模型都包含在内（不包含背景和地板），把模型一起进行的动作写在这即可。
	//animeWithBezier(rehave,numoftranslate);



	//第一个人物——小黄人
	glPushMatrix();
	glTranslated(-0.3, -0.48, 0.5);
	if( time > 3*6*2 && time <= 3*6*2+6){
		glRotated((time-3*2*6)/6.0*90, 0, 1, 0);
	}
	if(time >3*6*2+6){
		glRotated(90, 0, 1, 0);
	}
	if( time > 3*6*2+11*3+5){
		glRotated((time-(3*6*2+11*3+5))/6.0*-90, 0, 1, 0);
	}
	glScaled(0.55, 0.55, 0.55);

	glRotated(xRot, 1, 0, 0);
	glRotated(yRot, 0, 1, 0);
	//头发
	glPushMatrix();
	if((time % 4 == 1||time % 4 == 2)&&  time < 3*6*2){
		glRotated(-10, 0, 0, 1);
	}
	glPushMatrix();
	glBegin(GL_LINES);
	glColor3d(0, 0, 0);
	GLfloat x,y,z,angle;
	z = 0.0f;
	for(angle = 0.0f; angle <= PI; angle += (PI / 20.0f))
	{
		// 上半部
		y = 0.2f*sin(angle);
		x = 0.3f*cos(angle);
		glVertex3f(x, y+0.5, z);

		// 下半部
		y = 0.0f*sin(angle+PI);
		x = 0.0f*cos(angle+PI);
		glVertex3f(x, y+0.5, z);	
	}
	glEnd();
	glPopMatrix();

	//头
	glPushMatrix();
	glColor3dv(color);
	glTranslated(0, 0.3, 0);
	glutSolidSphere(0.3, slices, stacks);
	glPopMatrix();

	//眼镜腿
	glPushMatrix();
	GLfloat mat_ambient1[]    = { 0.3f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_diffuse1[]    = { 0.3f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_specular1[]   = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat high_shininess1[] = { 100.0f };
	GLfloat high_emission1[] = { 0.05f, 0.05f, 0.05f, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient1);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse1);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular1);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess1);
	glMaterialfv(GL_FRONT, GL_EMISSION, high_emission1);
	glColor3d(0, 0, 0);
	glTranslated(0, 0.32, 0);
	glRotated(-90, 1, 0, 0);
	glutSolidTorus(0.03, 0.3, slices, stacks);
	glPopMatrix();

	//眼镜框
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	GLfloat mat_ambient2[]    = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_diffuse2[]    = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_specular2[]   = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat high_shininess2[] = { 80.0f };
	GLfloat high_emission2[] = { 0.1f, 0.05f, 0.05f, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient2);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse2);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular2);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess2);
	glMaterialfv(GL_FRONT, GL_EMISSION,  high_emission2);
	glTranslated(0, 0.32, 0.3);
	glRotated(-10, 1, 0, 0);
	glutSolidTorus(0.03, 0.13, slices, stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_S); 
	glDisable(GL_TEXTURE_GEN_T);
	glPopMatrix();

	//眼睛
	glPushMatrix();
	GLfloat mat_ambient3[]    = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_diffuse3[]    = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_specular3[]   = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat high_shininess3[] = { 120.0f };
	GLfloat high_emission3[] = { 0.1f, 0.05f, 0.05f, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient3);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse3);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular3);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess3);
	glMaterialfv(GL_FRONT, GL_EMISSION,  high_emission3);
	glColor3d(1, 1, 1);
	glTranslated(0, 0.32, 0.25);
	glutSolidSphere(0.12, slices, stacks);
	glPopMatrix();

	//眼仁
	glPushMatrix();
	GLfloat mat_ambient4[]    = { 0.6f, 0.2f, 0.5f, 1.0f };
	GLfloat mat_diffuse4[]    = { 0.6f, 0.2f, 0.5f, 1.0f };
	GLfloat mat_specular4[]   = { 0.8f, 0.5f, 0.4f, 1.0f };
	GLfloat high_shininess4[] = { 10.0f };
	GLfloat high_emission4[] = { 0.05f, 0.05f, 0.05f, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient4);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse4);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular4);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess4);
	glMaterialfv(GL_FRONT, GL_EMISSION, high_emission4);
	glColor3d(0, 0, 0);
	glTranslated(0, 0.32, 0.335);
	glutSolidSphere(0.05, slices, stacks);
	glPopMatrix();

	//嘴
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	sweep();
	glEnd();
	glPopMatrix();

	//身体
	glPushMatrix();
	glColor3dv(color);
	glTranslated(0, 0.3, 0);
	glRotated(90, 1, 0, 0);  
	gluCylinder(obj0, 0.3, 0.3, 0.3, slices, stacks);
	glPopMatrix();

	//下半身和裤子
	glPushMatrix();
	GLfloat mat_ambient5[]    = { 0.3f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_diffuse5[]    = { 0.3f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_specular5[]   = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat high_shininess5[] = { 100.0f };
	GLfloat high_emission5[] = { 0.05f, 0.05f, 0.05f, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient5);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse5);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular5);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess5);
	glMaterialfv(GL_FRONT, GL_EMISSION, high_emission5);
	glColor3dv(color);
	glTranslated(0, 0, 0);
	glRotated(-45, 0, 1, 0);
	glRotated(90, 1, 0, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	gluQuadricNormals(obj1, GLU_SMOOTH);//使用平滑法线
	gluQuadricTexture(obj1,GL_TRUE);
	gluQuadricDrawStyle(obj1,GLU_FILL);
	gluCylinder(obj1, 0.3, 0.3, 0.3, slices, stacks);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//胃
	glPushMatrix();
	GLfloat mat_ambient6[]    = { 0.3f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_diffuse6[]    = { 0.3f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_specular6[]   = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat high_shininess6[] = { 100.0f };
	GLfloat high_emission6[] = { 0.05f, 0.05f, 0.05f, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient6);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse6);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular6);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess6);
	glMaterialfv(GL_FRONT, GL_EMISSION, high_emission6);
	GLdouble cut1[]= {0, -1, 0, -0.3};
	glColor4d(0.25, 0.39, 0.49, 1);
	glClipPlane(GL_CLIP_PLANE0, cut1);
	glEnable(GL_CLIP_PLANE0);
	glTranslated(0, 0.035, 0);
	glutSolidSphere(0.45, slices, stacks);
	glPopMatrix();
	glDisable(GL_CLIP_PLANE0);

	glPushMatrix();
	glRotated(rotate_forward, 1, 0, 0);
	if((time % 4 == 1||time % 4 == 2)&&  time < 3*6*2){
		glRotated(-40, 0, 0, 1);
	}
	if(time >3*6*2+6 && time < 3*6*2+6+5+11+5+11){
		glRotated(-60, 1, 0, 0);
		if((time % 4 == 1||time % 4 == 2)){
			glRotated(-10, 1, 0, 0);
		}

	}
	//左胳膊
	glPushMatrix();
	glColor3dv(color);
	glTranslated(-0.27, 0, 0);
	glRotated(90, 1, 0, 0);
	glRotated(-30, 0, 1, 0);
	gluCylinder(obj2, 0.04, 0.04, 0.25, slices, stacks);
	glPopMatrix();

	//左手
	glPushMatrix();
	glColor3d(0, 0, 0);
	glTranslated(-0.4, -0.23, 0);
	glutSolidSphere(0.05, slices, stacks);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	if(time >3*6*2+6 && time < 3*6*2+6+5+11+5+11){
		glRotated(-60, 1, 0, 0);
		if((time % 4 == 1||time % 4 == 2)){
			glRotated(-10, 1, 0, 0);
		}

	}
	glRotated(rotate_back, 1, 0, 0);
	//右胳膊
	glPushMatrix();
	glColor3dv(color);
	glTranslated(0.27, 0, 0);
	glRotated(90, 1, 0, 0);
	glRotated(30, 0, 1, 0);
	gluCylinder(obj2, 0.04, 0.04, 0.25, slices, stacks);
	glPopMatrix();

	//右手
	glPushMatrix();
	glColor3d(0, 0, 0);
	glTranslated(0.4, -0.23, 0);
	glutSolidSphere(0.05, slices, stacks);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();


	glPushMatrix();
	glRotated(rotate_back, 1, 0, 0);
	//左腿
	glPushMatrix();
	GLfloat mat_ambient7[]    = { 0.5f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_diffuse7[]    = { 0.5f, 0.3f, 0.5f, 1.0f };
	GLfloat mat_specular7[]   = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat high_shininess7[] = { 90.0f };
	GLfloat high_emission7[] = { 0.05f, 0.1f, 0.05f, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient7);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse7);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular7);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess7);
	glMaterialfv(GL_FRONT, GL_EMISSION, high_emission7);
	glColor3dv(color);
	glTranslated(-0.15, -0.35, 0);
	glRotated(180, 0, 1, 0);
	glRotated(90, 1, 0, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	gluQuadricNormals(obj4, GLU_SMOOTH);//使用平滑法线
	gluQuadricTexture(obj4,GL_TRUE);
	gluQuadricDrawStyle(obj4,GLU_FILL);
	gluCylinder(obj4, 0.07, 0.05, 0.2, slices, stacks);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//左脚
	glPushMatrix();
	glColor3d(0, 0, 0);
	glTranslated(-0.15, -0.55, 0.03);
	glRotated(-8, 1, 0, 0);
	glScaled(1, 0.8, 2);
	glutSolidSphere(0.05, slices, stacks);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glRotated(rotate_forward, 1, 0, 0);
	//右腿
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient7);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse7);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular7);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess7);
	glMaterialfv(GL_FRONT, GL_EMISSION, high_emission7);
	glColor3dv(color);
	glTranslated(0.15, -0.35, 0);
	glRotated(180, 0, 1, 0);
	glRotated(90, 1, 0, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	gluQuadricNormals(obj4, GLU_SMOOTH);//使用平滑法线
	gluQuadricTexture(obj4,GL_TRUE);
	gluQuadricDrawStyle(obj4,GLU_FILL);
	gluCylinder(obj4, 0.07, 0.05, 0.2, slices, stacks);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//右脚
	glPushMatrix();
	glColor3d(0, 0, 0);
	glTranslated(0.15, -0.55, 0.03);
	glRotated(-8, 1, 0, 0);
	glScaled(1, 0.8, 2);
	glutSolidSphere(0.05, slices, stacks);
	glPopMatrix();
	glPopMatrix();

	glPopMatrix();


	//第二个人物——
	const GLfloat no_mat[]    = { 0.0f, 0.0f, 0.0f, 1.0f };
	const GLfloat mat_emission[]    = { 0.3f, 0.2f, 0.2f, 0.0f };
	const GLfloat low_shininess[] = { 30.0f };
	const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
	const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 0.1f };
	const GLfloat high_shininess[] = { 100.0f };

	glPushMatrix();

	glTranslated(1,-0.54,0);
	glScaled(0.4, 0.4, 0.4);
	glRotated(xRot, 1, 0, 0);
	glRotated(yRot, 0, 1, 0);

	if(time%4==1){
		glTranslated(0.05,0,0);
	}
	if(time%4==3){
		glTranslated(-0.05,0,0);
	}
	// 帽子
	GLUquadricObj *quadratic;
	glPushMatrix();

	glPushMatrix();

	glMaterialfv(GL_FRONT, GL_AMBIENT,   no_mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION,   no_mat);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[6]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	if(time >3*6*2 && time <= 3*6*2+11){
		glTranslated(m[0][time-3*6*2],m[1][time-3*6*2],0);
	}
	if(time >3*6*2+11 && time <= 3*6*2+11*2){
		glTranslated(m[0][11-(time-3*6*2-11)],m[1][11-(time-3*6*2-11)],0);
	}
	//// 帽顶
	glPushMatrix();
	glPushMatrix();
	glColor3d(0,0.3,0);
	glTranslated(0,0.7,-0.0);
	glutSolidSphere(0.55,slices,stacks);
	glPopMatrix();
	// 帽檐
	glPushMatrix();
	glTranslated(0,0.67,0.0);
	glRotated(90,1,0,0);
	glScalef(1,1,0.4);
	glutSolidTorus(0.09,0.55,slices,stacks);
	glPopMatrix();	
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	// 脸
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT,   no_mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION,   no_mat);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[8]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glPushMatrix();
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(0,0.6,0.0);
	glutSolidSphere(0.55,slices,stacks);
	glPopMatrix();
	// 耳朵1
	glPushMatrix();
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(-0.5,0.54,0.0);
	glScaled(1.1,1.1,0.3);
	glutSolidSphere(0.15,slices,stacks);
	glPopMatrix();
	// 耳朵2
	glPushMatrix();
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(0.5,0.54,0.0);
	glScaled(1.1,1.1,0.3);
	glutSolidSphere(0.15,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	//帽子装饰
	glPushMatrix();
	//帽子装饰1
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[7]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glPushMatrix();
	glColor3d(0,0,1);
	glTranslated(0.06,1.1,0.25);
	glutSolidSphere(0.1,slices,stacks);
	glPopMatrix();
	//帽子装饰2
	glPushMatrix();
	glColor3d(0,0,1);
	glTranslated(-0.038,0.75,0.238);
	glRotated(8,0,1,0);
	glutSolidTorus(0.1,0.33,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	//眼睛1
	glPushMatrix();
	//眼睛10
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[12]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_S,GL_OBJECT_PLANE,params);
	glColor3d(0,0,0);
	glTranslated(-0.10,0.537,0.275);
	glutSolidSphere(0.26,slices,stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	//眼睛11
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[10]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glPushMatrix();
	glColor3d(1,1,1);
	glTranslated(-0.18,0.52,0.495);
	glutSolidSphere(0.034,slices,stacks);
	glPopMatrix();
	//眼睛12
	glPushMatrix();
	glColor3d(1,1,1);
	glTranslated(-0.2,0.45,0.473);
	glutSolidSphere(0.035,slices,stacks);
	glPopMatrix();
	//眼睛13
	glPushMatrix();
	glColor3d(1,1,1);
	glTranslated(-0.13,0.48,0.496);
	glutSolidSphere(0.034,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 

	glPopMatrix();
	glPopMatrix();

	//眼睛2
	glPushMatrix();
	//眼睛20
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[12]);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_S,GL_OBJECT_PLANE,params);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glColor3d(0,0,0);
	glTranslated(0.10,0.537,0.275);
	glutSolidSphere(0.26,slices,stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_S); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	glPushMatrix();
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[10]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	//眼睛21
	glPushMatrix();
	glColor3d(1,1,1);
	glTranslated(0.18,0.52,0.495);
	glutSolidSphere(0.034,slices,stacks);
	glPopMatrix();
	//眼睛22
	glPushMatrix();
	glColor3d(1,1,1);
	glTranslated(0.2,0.45,0.473);
	glutSolidSphere(0.035,slices,stacks);
	glPopMatrix();
	//眼睛23
	glPushMatrix();
	glColor3d(1,1,1);
	glTranslated(0.13,0.48,0.496);
	glutSolidSphere(0.034,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 

	glPopMatrix();
	glPopMatrix();
	//腮红
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[9]);
	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_S,GL_OBJECT_PLANE,params);
	//腮红1
	glPushMatrix();
	glColor3d(1,0,1);
	glTranslated(-0.28,0.37,0.38);
	glRotated(-40,0,1,0);
	glScaled(2.5,1,0.7);
	glutSolidSphere(0.04,slices,stacks);
	glPopMatrix();
	//腮红2
	glPushMatrix();
	glColor3d(1,0,1);
	glTranslated(0.28,0.37,0.38);
	glRotated(40,0,1,0);
	glScaled(2.5,1,0.7);
	glutSolidSphere(0.04,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_S); 


	glPopMatrix();
	//嘴
	glPushMatrix();
	//嘴1
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[11]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glColor3d(1,0,0);
	glTranslated(0.0,0.35,0.22);
	glRotated(15,1,0,0);
	glScaled(1.3,0.79,0.75);
	glutSolidSphere(0.3,slices,stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	//嘴2
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[10]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glColor3d(1,1,1);
	glTranslated(0.0,0.34,0.23);
	glScaled(1.3,0.70,0.75);
	glutSolidSphere(0.29,slices,stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	//嘴3
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[9]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glColor3d(1,0,1);
	glTranslated(0.0,0.26,0.24);
	glScaled(1.4,0.79,0.75);
	glutSolidSphere(0.2,slices,stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();


	// 身体
	glPushMatrix();
	if(time >6*3*2+11*2 && time <= 6*3*2+11*3){
		glTranslated(m[0][time-6*3*2-11*2]/2,0,m[1][time-72-11*2]/2);
		glRotated(10,1,0,0);
	}
	if(time >72+11*3 && time <= 6*3*2+11*4){
		glTranslated(m[0][11-(time-6*3*2-11*3)]/2,0,m[1][11-(time-6*3*2-11*3)]/2);
		glRotated(10,1,0,0);
	}
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[8]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(0,-0.05,0.0);
	glRotated(60,1,0,0);
	glScalef(0.86,1,1);
	glutSolidSphere(0.25,slices,stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	// 内裤
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT,   no_mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   no_mat);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION,   no_mat);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[7]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glColor3d(0,0,1);
	glTranslated(0,-0.20,0.0);
	glRotated(60,1,0,0);
	glScalef(1,1.1,1);
	glutSolidSphere(0.22,slices,stacks);
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();
	glPopMatrix();

	// 胳膊1
	glPushMatrix();
	if(time <= 6*3){
		glRotated(time*60,1,0,0);
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[8]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glPushMatrix();
	glTranslated(0,0.2,0.0);
	glRotated(90,1,0,0);
	glRotated(40,0,-1,0);
	glColor3d(249.0/255,200.0/255,160.0/255);
	quadratic=gluNewQuadric();
	gluCylinder(quadratic,0.05,0.05,0.5,slices,stacks);
	glPopMatrix();
	//手1
	glPushMatrix();
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(-0.34,-0.21,0.0);
	glRotated(20,1,0,0);
	glutSolidSphere(0.06,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();

	// 胳膊2
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[8]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	if(time > 6*3 && time <= 6*3*2){
		glRotated(60*time, 1, 0, 0);
	}
	glPushMatrix();
	glTranslated(0,0.2,0.0);
	glRotated(90,1,0,0);
	glRotated(40,0,1,0);
	glColor3d(249.0/255,200.0/255,160.0/255);
	quadratic=gluNewQuadric();
	gluCylinder(quadratic,0.05,0.05,0.5,slices,stacks);
	glPopMatrix();
	//手2
	glPushMatrix();
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(0.34,-0.21,0.0);
	glRotated(20,1,0,0);
	glutSolidSphere(0.06,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();

	// 腿1
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[8]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glPushMatrix();
	glTranslated(0,-0.09,0.0);
	glRotated(90,1,0,0);
	glRotated(17,0,-1,0);
	glColor3d(249.0/255,200.0/255,160.0/255);
	quadratic=gluNewQuadric();
	gluCylinder(quadratic,0.05,0.05,0.55,slices,stacks);
	glPopMatrix();
	//脚1
	glPushMatrix();
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(-0.13,-0.6,0.02);
	glRotated(90,1,0,0);
	glScaled(1.2,1.7,1);
	glutSolidSphere(0.06,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D); 
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();

	//腿2
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[8]);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T,GL_TEXTURE_ENV_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_T,GL_OBJECT_PLANE,params);
	glPushMatrix();
	glTranslated(0,-0.09,0.0);
	glRotated(90,1,0,0);
	glRotated(17,0,1,0);
	glColor3d(249.0/255,200.0/255,160.0/255);
	quadratic=gluNewQuadric();
	gluCylinder(quadratic,0.05,0.05,0.55,slices,stacks);
	glPopMatrix();
	//脚2
	glPushMatrix();
	glColor3d(249.0/255,200.0/255,160.0/255);
	glTranslated(0.13,-0.6,0.02);
	glRotated(90,1,0,0);
	glScaled(1.2,1.7,1);
	glutSolidSphere(0.06,slices,stacks);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_T); 
	glPopMatrix();


	glPopMatrix();



	//第三个人物——哆啦A梦
	GLUquadricObj *pObj = gluNewQuadric();
	glPushMatrix();
	if(time%4==1){
		glTranslated(0.05,0,0);
	}
	if(time%4==3){
		glTranslated(-0.05,0,0);
	}
	glTranslatef(-1.0f, -0.455f, 0.0f);
	glScalef(0.35,0.35,0.35);
	glRotatef(xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);
	quadratic = gluNewQuadric();

	//身体大部件
	glPushMatrix();
	glPushMatrix();
	if(time >3*6*2 && time <= 3*6*2+11){
		glTranslated(m[0][time-3*6*2],m[1][time-3*6*2],0);
	}
	if(time >3*6*2+11 && time <= 3*6*2+11*2){
		glTranslated(m[0][11-(time-3*6*2-11)],m[1][11-(time-3*6*2-11)],0);
	}
	glPushMatrix();//蓝头壳
	glColor3d(0,0,1);
	glTranslated(0,0.45,0);
	glutSolidSphere(0.7,slices,stacks);
	glPopMatrix();
	glPushMatrix();//白头壳
	glColor3d(1,1,1);
	glTranslated(0,0.45,0.23);
	glScalef(1.03,1,0.8);
	glutSolidSphere(0.59,slices,stacks);
	glPopMatrix();
	glPushMatrix();//鼻子
	glColor3d(1, 0, 0);
	glTranslated(0, 0.7, 0.7);
	glutSolidSphere(0.09,slices,stacks);
	glPopMatrix();

	//脸上的线
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT,   no_mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION,   no_mat);
	glColor3d(0, 0, 0);
	glPushMatrix();//鼻子线
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.7f,0.7f);
	glVertex3f(0.0f, 0.2f,0.71f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//嘴巴线1
	glBegin(GL_LINES);
	glVertex3f(-0.35f, 0.3f,0.68f);
	glVertex3f(0.0f, 0.2f,0.68f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//嘴巴线2
	glBegin(GL_LINES);
	glVertex3f(0.35f, 0.3f,0.68f);
	glVertex3f(0.0f, 0.2f,0.68f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//胡子左一
	glBegin(GL_LINES);
	glVertex3f(-0.15f, 0.6f,0.665f);
	glVertex3f(-0.55f, 0.7f,0.55f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//胡子左二
	glBegin(GL_LINES);
	glVertex3f(-0.15f, 0.5f,0.67f);
	glVertex3f(-0.55f, 0.5f,0.6f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//胡子左三
	glBegin(GL_LINES);
	glVertex3f(-0.15f, 0.4f,0.67f);
	glVertex3f(-0.55f, 0.33f,0.6f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//胡子右一
	glBegin(GL_LINES);
	glVertex3f(0.15f, 0.6f,0.665f);
	glVertex3f(0.55f, 0.7f,0.55f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//胡子右二
	glBegin(GL_LINES);
	glVertex3f(0.15f, 0.5f,0.67f);
	glVertex3f(0.55f, 0.5f,0.6f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//胡子右三
	glBegin(GL_LINES);
	glVertex3f(0.15f, 0.4f,0.67f);
	glVertex3f(0.55f, 0.33f,0.6f);
	glEnd();
	glPopMatrix();
	glPopMatrix();
	//眼睛
	glPushMatrix();
	glPushMatrix();//左眼
	glColor3d(1, 1, 1);
	glTranslated(-0.13, 0.9, 0.58);
	glScalef(0.69,0.9,0.4);
	glRotated(90, 0, 0, 1);
	glutSolidSphere(0.183,slices,stacks);
	glPopMatrix();
	glPushMatrix();//左眼球
	glColor3d(0, 0, 0);
	glTranslated(-0.1, 0.9, 0.65);
	glScalef(1,1,0.1);
	glutSolidSphere(0.06,slices,stacks);
	glPopMatrix();
	glPushMatrix();//左瞳孔
	glColor3d(1, 1, 1);
	glTranslated(-0.1, 0.9, 0.655);
	glScalef(1,1,0.1);
	glutSolidSphere(0.02,slices,stacks);
	glPopMatrix();
	glPushMatrix();//右眼
	glColor3d(1, 1, 1);
	glTranslated(0.13, 0.9, 0.58);
	glScalef(0.69,0.9,0.4);
	glRotated(90, 0, 0, 1);
	glutSolidSphere(0.183,slices,stacks);
	glPopMatrix();
	glPushMatrix();//右眼球
	glColor3d(0, 0, 0);
	glTranslated(0.1, 0.9, 0.65);
	glScalef(1,1,0.1);
	glutSolidSphere(0.06,slices,stacks);
	glPopMatrix();
	glPushMatrix();//右瞳孔
	glColor3d(1, 1, 1);
	glTranslated(0.1, 0.9, 0.655);
	glScalef(1,1,0.1);
	glutSolidSphere(0.02,slices,stacks);
	glPopMatrix();
	glPopMatrix();
	//眼睛
	glPopMatrix();
	glPushMatrix();//左腿
	glColor3d(0, 0, 1);
	glTranslated(-0.2, -0.6, 0);
	glRotated(90, 1, 0, 0);
	gluCylinder(quadratic, 0.1, 0.1, 0.2, slices, stacks);
	glPopMatrix();
	glPushMatrix();//右腿
	glColor3d(0, 0, 1);
	glTranslated(0.2, -0.6, 0);
	glRotated(90, 1, 0, 0);
	gluCylinder(quadratic, 0.1, 0.1, 0.2, slices, stacks);
	glPopMatrix();
	glPushMatrix();//左脚
	glColor3d(1, 1, 1);
	glTranslated(-0.2, -0.883, 0);
	glScalef(0.8,0.6,1);
	glutSolidSphere(0.18,slices,stacks);
	glPopMatrix();
	glPushMatrix();//右脚
	glColor3d(1, 1, 1);
	glTranslated(0.2, -0.883, 0);
	glScalef(0.8,0.6,1);
	glutSolidSphere(0.18,slices,stacks);
	glPopMatrix();

	glPushMatrix();
	glTranslated(-0.4, -0.4, 0);
	if(time <= 6*3){
		glRotated(20*time, 1, 0, 0);
	}
	glPushMatrix();//左手臂

	glColor3d(0, 0, 1);

	//glRotated(50, 0, 0, 1);
	//glRotated(90, 0, 1, 0);

	glRotated(-50, 1, 0, 0);
	glRotated(-50, 0, 1, 0);

	gluCylinder(quadratic, 0.08, 0.08, 0.4, slices, stacks);
	glPopMatrix();
	glPushMatrix();//左手
	glColor3d(1, 1, 1);
	glTranslated(-0.3, 0.19, 0.15);
	glutSolidSphere(0.13,slices,stacks);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.4, -0.4, 0);
	if(time > 6*3 && time <= 6*3*2){
		glRotated(60*time, 1, 0, 0);
	}
	glPushMatrix();//右手臂
	glColor3d(0, 0, 1);

	glRotated(50, 0, 1, 0);
	glRotated(-50, 1, 0, 0);
	//glRotated(-50, 1, 0, 0);
	gluCylinder(quadratic, 0.08, 0.08, 0.4, slices, stacks);
	glPopMatrix();
	glPushMatrix();//右手
	glTranslated(0.18, 0.32, 0.15);
	glColor3d(1, 1, 1);
	glutSolidSphere(0.13,slices,stacks);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	if(time >6*3*2+11*2 && time <= 6*3*2+11*3){
		glTranslated(m[0][time-6*3*2-11*2]/2,0,m[1][time-72-11*2]/2);
		glRotated(10,1,0,0);
	}
	if(time >72+11*3 && time <= 6*3*2+11*4){
		glTranslated(m[0][11-(time-6*3*2-11*3)]/2,0,m[1][11-(time-6*3*2-11*3)]/2);
		glRotated(10,1,0,0);
	}
	glPushMatrix();//尾巴
	glBegin(GL_LINES);
	glVertex3f(0.0f, -0.4f,0);
	glVertex3f(0.0f, -0.35f,-0.5f);
	glEnd();
	glPopMatrix();
	glPushMatrix();//尾巴头
	glColor3d(1,0,0);
	glTranslated(0,-0.35,-0.53);
	glutSolidSphere(0.09,slices,stacks);
	glPopMatrix();

	glPushMatrix();//蓝身体
	glColor3d(0,0,1);
	glTranslated(0,-0.38,0);
	glScalef(1.6,1,1);
	glutSolidSphere(0.35,slices,stacks);
	glPopMatrix();
	glPushMatrix();//白肚皮
	glColor3d(1,1,1);
	glTranslated(0,-0.3,0.2);
	glScalef(1.2,1,0.7);
	glutSolidSphere(0.3,slices,stacks);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	//身体大部件




	//脸上的线

	glPushMatrix();//项圈
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

	glColor3d(1,0,0);
	glTranslated(0,-0.15,0.05);
	glRotatef(90.,1.0,0,0.0);
	glScalef(1,1,0.7);
	glutSolidTorus(0.05,0.38,slices,stacks);
	glPopMatrix();

	glPushMatrix();//铃铛
	glMaterialfv(GL_FRONT, GL_AMBIENT,   no_mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION,   no_mat);
	glColor3d(0.9,1,0);
	//glColor3d(1,1,1);
	glTranslated(0,-0.15,0.5);
	glutSolidSphere(0.06,slices,stacks);
	glPopMatrix();

	glPopMatrix();


	//第四个人物——QQ企鹅
	glPushMatrix();
	glTranslated(0.32, -0.43, 0.45);
	if( time > 3*6*2 && time <= 3*6*2+6){
		glRotated((time-3*2*6)/6.0*-90, 0, 1, 0);
	}
	if(time >3*6*2+6){
		glRotated(-90, 0, 1, 0);
	}
	if( time > 3*6*2+11*3+5){
		glRotated((time-(3*6*2+11*3+5))/6.0*90, 0, 1, 0);
	}
	glScaled(0.65, 0.65, 0.65);
	glRotated(xRot, 1, 0, 0);
	glRotated(yRot, 0, 1, 0);
	// 组成嘴的六个三角形
	static int list = 0; 
	// 嘴的数组
	GLfloat PointA[] = {0, 0.0, 0.28},
		PointB[] = {-0.15, 0.1, 0.28},
		PointC[] = {0.15, 0.1, 0.28},
		PointD[] = {0, 0.1, 0.45},
		PointE[] = {0, 0.15, 0.28};
	GLfloat ColorR[] = {0, 0, 1};
	glNewList(list, GL_COMPILE);
	glColor3f(1, 0.6, 0);
	glBegin(GL_TRIANGLES);   
	// 平面BDC
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointC);
	glColor3f(1, 1.0, 0);
	glVertex3fv(PointD);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointB);
	// 平面ADB
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointA);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointB);
	glColor3f(1, 0.8, 0);
	glVertex3fv(PointD);
	// 平面ACD
	glColor3f(1, 1.0, 0);
	glVertex3fv(PointD);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointC);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointA);

	glColor3f(1, 0.6, 0);
	glVertex3fv(PointE);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointB);
	glColor3f(1, 1.0, 0);
	glVertex3fv(PointD);

	glColor3f(1, 0.6, 0);
	glVertex3fv(PointE);
	glColor3f(1, 1.0, 0);
	glVertex3fv(PointD);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointC);

	glColor3f(1, 1.0, 0);
	glVertex3fv(PointD);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointB);
	glColor3f(1, 0.6, 0);
	glVertex3fv(PointC);
	glEnd();
	glEndList();
	glPushMatrix();
	if((time % 4 == 1||time % 4 == 2)&&  time < 3*6*2){
		glRotated(10, 0, 0, 1);
	}
	glPushMatrix();
	// 嘴
	// mat 0
	const GLfloat mat0_ambient[]    = { 0.5f, 0.5f, 0.5f, 0.5f };
	const GLfloat mat0_diffuse[]    = { 0.5f, 0.5f, 0.5f, 1.0f };
	const GLfloat mat0_specular[]   = { 0.5f, 0.5f, 0.5f, 1.0f };
	const GLfloat high0_shininess[] = { 100.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat0_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat0_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat0_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high0_shininess);
	glPushMatrix();  
	glCallList(list);
	glPopMatrix();  

	//围脖
	// mat 1
	const GLfloat mat1_ambient[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat1_diffuse[]    = { 0.1f, 0.1f, 0.1f, 1.0f };
	const GLfloat mat1_specular[]   = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat high1_shininess[] = { 100.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat1_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat1_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat1_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high1_shininess);
	glPushMatrix();
	glColor3d(1, 0.0, 0);
	glTranslated(0.0, 0.0, 0.0);
	glRotated(90, 1.0, 0.0, 0.0);
	glutSolidTorus(0.16, 0.17, 10, 10);
	glPopMatrix();

	// 头
	// mat 2
	const GLfloat mat2_ambient[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat2_diffuse[]    = { 0.1f, 0.1f, 0.1f, 1.0f };
	const GLfloat mat2_specular[]   = { 0.5f, 0.5f, 0.5f, 1.0f };
	const GLfloat high2_shininess[] = { 100.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat2_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat2_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat2_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high2_shininess);

	glPushMatrix();
	glTranslated(0.0, 0.15, 0.0);
	glColor3d(0, 0, 0);
	gluQuadricNormals(pObj, GLUT_NORMAL); // 使用平滑法线
	gluQuadricTexture(pObj, GL_TRUE); 
	gluSphere(pObj, 0.3, 40, 40 );
	gluQuadricTexture(pObj, GL_FALSE);
	glPopMatrix();
	glDisable(GL_LIGHTING);

	// 身体
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslated(0.0, -0.2, 0.0);
	glutSolidSphere( 0.35, 40, 40 ); 
	glPopMatrix();

	// 左手臂
	glPushMatrix();
	if(time >3*6*2+6 && time < 3*6*2+6+5+11+5+11){
		glRotated(-60, 1, 0, 0);
		if((time % 4 == 1||time % 4 == 2)){
			glRotated(-10, 1, 0, 0);
		}

	}
	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslated(-0.22, 0.0, 0.1);
	glRotated(40, 0, 0, 1);
	glRotated(-90, 0, 1, 0);
	GLUquadricObj *objCylinder = gluNewQuadric();
	gluCylinder(objCylinder, 0.08, 0.03, 0.3, 32, 5);
	glPopMatrix();
	//左手
	glPushMatrix();
	glColor3f(0.9, 0.9, 1);
	glTranslated(-0.44, -0.19, 0.1);
	glutSolidSphere(0.04, 40, 40);
	glPopMatrix();
	glPopMatrix();

	// 右手臂
	glPushMatrix();
	if((time % 4 == 1||time % 4 == 2)&&  time < 3*6*2){
		glRotated(40, 0, 0, 1);
	}
	if(time >3*6*2+6 && time < 3*6*2+6+5+11+5+11){
		glRotated(-60, 1, 0, 0);
		if((time % 4 == 1||time % 4 == 2)){
			glRotated(-10, 1, 0, 0);
		}

	}

	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslated(0.22, 0.0, 0.1);
	glRotated(-40, 0, 0, 1);
	glRotated(90, 0, 1, 0);
	GLUquadricObj *objCylinder1 = gluNewQuadric();
	gluCylinder(objCylinder1, 0.08, 0.03, 0.3, 32, 5);
	glPopMatrix();
	//右手
	glPushMatrix();
	glColor3f(0.9, 0.9, 1);
	glTranslated(0.44, -0.19, 0.1);
	glutSolidSphere(0.04, 40, 40);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	// 左脚
	// mat 3
	const GLfloat mat3_ambient[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat3_diffuse[]    = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat mat3_specular[]   = { 0.5f, 0.5f, 0.5f, 1.0f };
	const GLfloat high3_shininess[] = { 100.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat3_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat3_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat3_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high3_shininess);
	glPushMatrix();
	glColor3f(1, 0.5, 0);
	glTranslated(-0.2, -0.45, 0);
	glScaled(1.0,1.0,1.6);
	glutSolidSphere(0.13, 8, 5);
	glPopMatrix();

	// 右脚
	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat3_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat3_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat3_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high3_shininess);
	glPushMatrix();
	glColor3f(1, 0.5, 0);
	glTranslated(0.2, -0.45, 0);
	glScaled(1.0,1.0,1.6);
	glutSolidSphere(0.13, 8, 5);
	glPopMatrix();

	// 左眼睛
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslated(-0.1, 0.2, 0.3);
	glScaled(1.0,1.8,0.1);
	glutSolidSphere(0.05, 20, 10);
	glPopMatrix();

	// 右眼睛
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslated(0.1, 0.2, 0.3);
	glScaled(1.0,1.8,0.1);
	glutSolidSphere(0.05, 20, 10);
	glPopMatrix();

	// 左眼珠
	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslated(0.1, 0.2, 0.31);
	glScaled(1.0, 1.0, 0.01);
	glutSolidSphere(0.03, 20, 10);
	glPopMatrix();

	// 右眼珠
	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslated(-0.1, 0.2, 0.31);
	glScaled(1.0, 1.0, 0.01);
	glutSolidSphere(0.03, 20, 10);
	glPopMatrix();
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();


	glPopMatrix();//描绘人物结束


	//承载物和背景
	glPushMatrix();

	glPushMatrix();//承载物
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ImageIDs[3]);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBegin(GL_QUADS);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-2.0f, -0.8f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-2.0f, -0.8f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(2.0f, -0.8f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(2.0f, -0.8f, 1.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//大屏幕
	// 激活纹理对象
	glPushMatrix();
	glActiveTextureARB (GL_TEXTURE0_ARB); // 选中号纹理
	glEnable(GL_TEXTURE_2D); // 使纹理可用
	glBindTexture(GL_TEXTURE_2D, ImageIDs[5]); // 激活纹理
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // 设置纹理贴图方式

	// 绘制正方形
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 0.0);
	glVertex3f(-0.3, 0.7, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 1.0);
	glVertex3f(-0.3, 1.3, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 1.0);
	glVertex3f(0.3, 1.3, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 0.0);
	glVertex3f(0.3, 0.7, -0.3);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// 激活纹理对象
	glActiveTextureARB (GL_TEXTURE0_ARB); // 选中号纹理
	glEnable(GL_TEXTURE_2D); // 使纹理可用
	glBindTexture(GL_TEXTURE_2D, ImageIDs[5]); // 激活纹理
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // 设置纹理贴图方式

	// 绘制正方形
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 0.0);
	glVertex3f(0.3, 0.7, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 1.0);
	glVertex3f(0.3, 1.3, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 1.0);
	glVertex3f(0.3, 1.3, 0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 0.0);
	glVertex3f(0.3, 0.7, 0.3);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// 激活纹理对象
	glActiveTextureARB (GL_TEXTURE0_ARB); // 选中号纹理
	glEnable(GL_TEXTURE_2D); // 使纹理可用
	glBindTexture(GL_TEXTURE_2D, ImageIDs[5]); // 激活纹理
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // 设置纹理贴图方式

	// 绘制正方形
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 0.0);
	glVertex3f(-0.3, 0.7, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 1.0);
	glVertex3f(-0.3, 1.3, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 1.0);
	glVertex3f(-0.3, 1.3, 0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 0.0);
	glVertex3f(-0.3, 0.7, 0.3);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// 激活纹理对象
	glActiveTextureARB (GL_TEXTURE0_ARB); // 选中号纹理
	glEnable(GL_TEXTURE_2D); // 使纹理可用
	glBindTexture(GL_TEXTURE_2D, ImageIDs[5]); // 激活纹理
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // 设置纹理贴图方式

	// 绘制正方形
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 0.0);
	glVertex3f(-0.3, 0.7, 0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 1.0);
	glVertex3f(-0.3, 1.3, 0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 1.0);
	glVertex3f(0.3, 1.3, 0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 0.0);
	glVertex3f(0.3, 0.7, 0.3);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// 激活纹理对象
	glActiveTextureARB (GL_TEXTURE0_ARB); // 选中号纹理
	glEnable(GL_TEXTURE_2D); // 使纹理可用
	glBindTexture(GL_TEXTURE_2D, ImageIDs[5]); // 激活纹理
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // 设置纹理贴图方式

	// 绘制正方形
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 0.0);
	glVertex3f(-0.3, 1.3, -0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 0.0, 1.0);
	glVertex3f(-0.3, 1.3, 0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 1.0);
	glVertex3f(0.3, 1.3, 0.3);
	glMultiTexCoord2fARB (GL_TEXTURE0_ARB, 1.0, 0.0);
	glVertex3f(0.3, 1.3, -0.3);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPopMatrix();

	
	//开始画粒子（流星、喷泉）
	glTranslated(0, 0, 0);
	//流星
	glPushMatrix();
	glTranslatef(1.0f, 1.0f, -2.0f); 
	glRotatef(180.0f , 0.0f , 0.0f , 1.0f);
	glScaled(0.03, 0.03, 0.03);
	DrawMeteor();    
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 2.0f, -2.0f); 
	glRotatef(160.0f , 0.0f , 0.0f , 1.0f);
	glScaled(0.03, 0.03, 0.03);
	DrawMeteor();    
	glPopMatrix();

	//左侧喷泉
	glPushMatrix();
	glTranslatef(-0.5f, -0.5f, -0.8f);
	glScaled(0.03, 0.03, 0.03);
	DrawParticle();
	glPopMatrix();
	glPopMatrix();

	//右侧喷泉
	glPushMatrix();
	glTranslatef(0.5f, -0.5f, -0.8f);
	glScaled(0.03, 0.03, 0.03);
	DrawParticle();
	glPopMatrix();
	glPopMatrix();

	glFlush();
	glLoadIdentity();
	glutSwapBuffers();
}

void texture()
{
	glEnable(GL_DEPTH_TEST);

	glGenTextures(30, ImageIDs);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[0]);
	Image0 = auxDIBImageLoad("..\\data\\glasses.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Image0->sizeX, Image0->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image0->data);
	free(Image0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[1]);
	Image1 = auxDIBImageLoad("..\\data\\pants.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Image1->sizeX, Image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image1->data);
	free(Image1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[2]);
	Image2 = auxDIBImageLoad("..\\data\\legs.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Image2->sizeX, Image2->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image2->data);
	free(Image2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[3]);
	Image3 = auxDIBImageLoad("..\\data\\court.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Image3->sizeX, Image3->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image3->data);
	free(Image3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[4]);
	Image4 = auxDIBImageLoad("..\\data\\back.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image4->sizeX, Image4->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image4->data);
	free(Image4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[5]);
	Image5 = auxDIBImageLoad("..\\data\\light.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image5->sizeX, Image5->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image5->data);
	free(Image5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);


	glBindTexture(GL_TEXTURE_2D, ImageIDs[14]);
	Image14 = auxDIBImageLoad("..\\data\\木纹.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image14->sizeX, Image14->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image14->data);
	free(Image14);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[15]);
	Image15 = auxDIBImageLoad("..\\data\\金铃铛.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image15->sizeX, Image15->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image15->data);
	free(Image15);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[16]);
	Image16 = auxDIBImageLoad("..\\data\\气球纹.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image16->sizeX, Image16->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image16->data);
	free(Image16);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[17]);
	Image17 = auxDIBImageLoad("..\\data\\eye.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image17->sizeX, Image17->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image17->data);
	free(Image17);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[18]);
	Image18 = auxDIBImageLoad("..\\data\\floor.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image18->sizeX, Image18->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image18->data);
	free(Image18);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[19]);
	Image19 = auxDIBImageLoad("..\\data\\6.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image19->sizeX, Image19->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image19->data);
	free(Image19);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[20]);
	Image20 = auxDIBImageLoad("..\\data\\6.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image20->sizeX, Image20->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image20->data);
	free(Image20);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[21]);
	Image21 = auxDIBImageLoad("..\\data\\water.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image21->sizeX, Image21->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image21->data);
	free(Image21);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[22]);
	Image22 = auxDIBImageLoad("..\\data\\meteor.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image22->sizeX, Image22->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image22->data);
	free(Image22);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[23]);
	Image23 = auxDIBImageLoad("..\\data\\zhuan.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image23->sizeX, Image23->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image23->data);
	free(Image23);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glBindTexture(GL_TEXTURE_2D, ImageIDs[24]);
	Image24 = auxDIBImageLoad("..\\data\\pearl.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image24->sizeX, Image24->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image24->data);
	free(Image24);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glEnable(GL_TEXTURE_COMPONENTS);
}

void ShutdownRC(void)
{
	glDeleteTextures(25, ImageIDs);
}

void keyboard1 (int key, int x, int y)
{
	switch (key)
	{

	case GLUT_KEY_UP:
		xRot-= 5.0f;
		xRot = (GLdouble)((const int)xRot % 360);
		break;

	case GLUT_KEY_DOWN:
		xRot += 5.0f;
		xRot = (GLdouble)((const int)xRot % 360);
		break;

	case GLUT_KEY_LEFT:
		yRot -= 5.0f;
		yRot = (GLdouble)((const int)yRot % 360);
		break;

	case GLUT_KEY_RIGHT:
		yRot += 5.0f;
		yRot = (GLdouble)((const int)yRot % 360);
		break;

	default:
		break;
	}

	glutPostRedisplay();
}

void keyboard2 (unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'o':
		du = 90;
		oldmy = -1;
		oldmx = -1;
		r = 2.0;
		h = 0.0;
		isPers = false;
		setortho();
		break;

	case 'p':
		du = 90;
		oldmy = -1;
		oldmx = -1;
		r = 2.0;
		h = 0.0;
		isPers = true;
		setperspective();
		break;

	case 'a':
		slices++;
		stacks++;
		break;

	case 'b':
		if (slices>3 && stacks>3)
		{
			slices--;
			stacks--;
		}
		break;

	case 'w':
		if (cycle == 0)
		{
			steps += 2;
			rotate_forward += 2;
			rotate_back -= 2;
			counts++;
			if (counts >= 10)
			{
				cycle = 1;
			}
		}
		else
		{
			steps += 2;
			rotate_forward -= 2;
			rotate_back += 2;
			counts--;
			if (counts <= -10)
			{
				cycle = 0;
			}
		}
		break;

	case 's':
		if (cycle == 0)
		{
			steps -= 2;
			rotate_forward -= 2;
			rotate_back += 2;
			counts--;
			if (counts <= -10)
			{
				cycle = 1;
			}
		}
		else
		{
			steps -= 2;
			rotate_forward += 2;
			rotate_back -= 2;
			counts++;
			if (counts >= 10)
			{
				cycle = 0;
			}
		}
		break;

	case 'q':
		exit(0);
		break;

	case 32:
		isRun ++;
		break;

	case '1':
		PlaySound(  "1.wav",   NULL, ( SND_APPLICATION | SND_ASYNC | SND_FILENAME | SND_LOOP | SND_NODEFAULT ) );
		break;

	case '2':
		PlaySound(  "2.wav",   NULL, ( SND_APPLICATION | SND_ASYNC | SND_FILENAME | SND_LOOP | SND_NODEFAULT ) );
		break;

	case '3':
		PlaySound(  "3.wav",   NULL, ( SND_APPLICATION | SND_ASYNC | SND_FILENAME | SND_LOOP | SND_NODEFAULT ) );
		break;

	default:
		break;
	}

	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y) //处理鼠标点击
{
	if(state == GLUT_DOWN) //第一次鼠标按下时,记录鼠标在窗口中的初始坐标
		oldmx = x,oldmy = y;
}

void onMouseMove(int x,int y) //处理鼠标拖动
{
	du += x - oldmx; //鼠标在窗口x轴方向上的增量加到视点绕y轴的角度上，这样就左右转了
	h += 0.03f * ( y - oldmy ); //鼠标在窗口y轴方向上的改变加到视点的y坐标上，就上下转了
	if(h > 5.0f)
		h = 5.0f; //视点y坐标作一些限制，不会使视点太奇怪
	else if(h < -5.0f)
		h = -5.0f;
	oldmx = x, oldmy = y; //把此时的鼠标坐标作为旧值，为下一次计算增量做准备

	display();
}

static void idle(void)
{
	glutPostRedisplay();
}

static void setLight(void)
{
	//设置平行光源
	const GLfloat global_light_ambient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
	const GLfloat global_light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat global_light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat global_light_position[] = { 0.5f, 0.5f, -0.5f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT,  global_light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  global_light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, global_light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, global_light_position);

	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);

	//设置聚光灯
	const GLfloat collect_light_ambient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
	const GLfloat collect_light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat collect_light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat collect_light_position[] = { -0.5f, 0.5f, -0.5f, 1.0f };
	const GLfloat collect_light_direction[] = {0, 0, -1};
	const GLfloat collect_light_exponent[] = {1.0f};
	const GLfloat collect_light_cutoff[] = {90.0f};

	glLightfv(GL_LIGHT1, GL_AMBIENT,  collect_light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  collect_light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, collect_light_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, collect_light_position);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, collect_light_direction);
	glLightfv(GL_LIGHT1, GL_SPOT_EXPONENT, collect_light_exponent);
	glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, collect_light_cutoff);

	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
}

int main(int argc, char** argv)
{
	cout << " \n\n\n\n               Instruction Introduce          \n\n"
		<< "left mouse press           for change the angle of view \n"
		<< "o/p                        for perspective/orthogonality projection\n"
		<< "up/down/left/right arrow   for rotation\n"
		<< "a/b                        for increase/decrease stacks and slices \n"
		<< "space                      for control the model stop at the first place \n\n";

	ParticleInit();
	glShadeModel(GL_SMOOTH);           // 阴暗处理采用平滑方式
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);     // 最精细的透视计算 

	glutInit(&argc, argv);
	glutInitWindowSize (width, height);
	glutInitWindowPosition (0, 0);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutCreateWindow ("Yellow");

	//glClearColor(0.5, 0.2, 0.8, 1.0);
	glClearColor(0, 0, 0, 1.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	move();
	setLight();

	//关闭背面顶点的计算
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE , GL_FALSE);
	texture();
	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutSpecialFunc(keyboard1);
	glutKeyboardFunc(keyboard2);
	glutMouseFunc(Mouse);
	glutMotionFunc(onMouseMove);
	glutIdleFunc(idle);

	glewInit();
	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}


	glutMainLoop();

	ShutdownRC();

	return EXIT_SUCCESS;
}
