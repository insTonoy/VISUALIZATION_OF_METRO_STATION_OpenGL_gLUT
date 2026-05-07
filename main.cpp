
#ifdef __APPLE__
#else
  #include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// GLOBAL SCENE SWITCH
static int currentScene = 1;   // 1 = outdoor,  2 = ticket machine,  3 = platform
static const int WIN_W = 1100;
static const int WIN_H = 680;


//scene-1 animation state
static float s1_car1Pos  = -15.0f;
static float s1_car2Pos  =  15.0f;
static float s1_metroPos =  -14.0f;
static int   s1_metroDir =  1;

#define S1_MAX_CLOUDS 3
typedef struct { float x,y,size,speed,alpha; } S1_Cloud;
static S1_Cloud s1_clouds[S1_MAX_CLOUDS];

#define S1_MAX_PEOPLE 8
typedef struct { float pos,speed,r,g,b,size; } S1_Person;
static S1_Person s1_people[S1_MAX_PEOPLE];

#define S1_MAX_LAMPS 6
typedef struct { float x,y; } S1_Lamp;
static S1_Lamp s1_lamps[S1_MAX_LAMPS];

//random window colors
static GLfloat s1_winColors[256][3];
static int     s1_winColorCount = 0;

//overhead wire height
static const float S1_WIRE_Y = 6.1f;

//scene-1 init helpers
static void s1_generateWindowColors(int n)
{
    srand(42);
    for(int i=0;i<n;i++)
        {
        int type=rand()%3;
        if(type==0)
        {
            s1_winColors[i][0]=0.9f+(rand()%10)/100.0f;
            s1_winColors[i][1]=0.5f+(rand()%40)/100.0f;
            s1_winColors[i][2]=0.2f+(rand()%30)/100.0f;
        }
        else if(type==1)
        {
            s1_winColors[i][0]=0.2f+(rand()%30)/100.0f;
            s1_winColors[i][1]=0.5f+(rand()%50)/100.0f;
            s1_winColors[i][2]=0.8f+(rand()%20)/100.0f;
        }
        else
        {
            s1_winColors[i][0]=0.8f+(rand()%20)/100.0f;
            s1_winColors[i][1]=0.8f+(rand()%20)/100.0f;
            s1_winColors[i][2]=0.7f+(rand()%30)/100.0f;
        }
    }
    s1_winColorCount=n;
}

static void s1_initClouds()
{
    s1_clouds[0]={-8.0f,7.5f,1.5f,0.015f,0.7f};
    s1_clouds[1]={ 2.0f,8.0f,1.8f,0.02f, 0.6f};
    s1_clouds[2]={10.0f,7.0f,1.3f,0.018f,0.7f};
}

static void s1_initPeople()
{
    s1_people[0]={-10.0f, 0.025f,0.3f,0.4f,0.8f,0.9f};
    s1_people[1]={ -6.0f, 0.03f, 0.9f,0.3f,0.3f,1.0f};
    s1_people[2]={  5.0f,-0.022f,0.2f,0.7f,0.4f,0.85f};
    s1_people[3]={  2.0f,-0.028f,0.8f,0.6f,0.2f,0.95f};
    s1_people[4]={-12.0f, 0.022f,0.7f,0.3f,0.6f,1.1f};
    s1_people[5]={ -2.0f, 0.018f,0.4f,0.8f,0.4f,0.88f};
    s1_people[6]={  7.0f,-0.026f,0.5f,0.5f,0.7f,0.92f};
    s1_people[7]={ -8.5f, 0.029f,0.9f,0.7f,0.2f,1.05f};
}

static void s1_initLamps()
{
    float lx[]={-13.0f,-9.0f,-4.5f,0.5f,6.0f,11.0f};
    for(int i=0;i<S1_MAX_LAMPS;i++){s1_lamps[i].x=lx[i]; s1_lamps[i].y=-.40f;}
}

//scene-1 primitive helpers
static void s1_drawText(const char*str,float x,float y)
{
    glRasterPos2f(x,y);
    for(int i=0;str[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,str[i]);
}

static void s1_rect(float x,float y,float w,float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x-w/2,y); glVertex2f(x+w/2,y);
    glVertex2f(x+w/2,y+h); glVertex2f(x-w/2,y+h);
    glEnd();
}

static void s1_circ(float cx,float cy,float r,int seg)
{
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(int i=0;i<=seg;i++)
    {
        float a=i*(2.0f*3.14159f)/seg;
        glVertex2f(cx+cosf(a)*r,cy+sinf(a)*r);
    }
    glEnd();
}

//scene-1 drawing functions
static void s1_drawSkyGradient()
{
    glDisable(GL_BLEND);
    glBegin(GL_QUADS);
    glColor3f(0.35f,0.55f,0.85f); glVertex2f(-15.0f,10.0f); glVertex2f(15.0f,10.0f);
    glColor3f(0.6f,0.8f,1.0f);   glVertex2f(15.0f,3.0f);   glVertex2f(-15.0f,3.0f);
    glEnd();
    glEnable(GL_BLEND);
}

static void s1_drawSun()
{
    glColor4f(1.0f,0.9f,0.6f,0.3f); s1_circ(11.0f,8.5f,1.2f,32);
    glColor4f(1.0f,0.85f,0.4f,0.5f);s1_circ(11.0f,8.5f,0.9f,32);
    glColor3f(1.0f,0.8f,0.2f);      s1_circ(11.0f,8.5f,0.6f,32);
}

static void s1_drawCloud(float x,float y,float size,float alpha)
{
    glColor4f(1.0f,1.0f,1.0f,alpha);
    s1_circ(x,           y,            size*0.6f,20);
    s1_circ(x+size*0.4f, y-size*0.1f,  size*0.5f,20);
    s1_circ(x-size*0.4f, y-size*0.1f,  size*0.5f,20);
    s1_circ(x+size*0.7f, y-size*0.2f,  size*0.4f,20);
    s1_circ(x-size*0.7f, y-size*0.2f,  size*0.4f,20);
    s1_rect(x,           y-size*0.2f,   size*1.3f, size*0.4f);
}

static void s1_drawBackground()
{
    glDisable(GL_BLEND);
    glBegin(GL_QUADS);
    glColor3f(0.45f,0.55f,0.35f); glVertex2f(-15.0f,-2.0f); glVertex2f(15.0f,-2.0f);
    glColor3f(0.55f,0.65f,0.4f);  glVertex2f(15.0f,0.0f);   glVertex2f(-15.0f,0.0f);
    glEnd();
    glEnable(GL_BLEND);

    glColor3f(0.7f,0.68f,0.65f); s1_rect(0,-0.21f,30.0f,0.2f);

    glColor3f(0.2f,0.2f,0.22f);
    glBegin(GL_QUADS);
    glVertex2f(-15.0f,-2.3f); glVertex2f(15.0f,-2.3f);
    glVertex2f(15.0f,-0.4f);  glVertex2f(-15.0f,-0.4f);
    glEnd();

    glColor3f(1.0f,1.0f,0.9f);
    for(float x=-14.5f;x<15.0f;x+=1.2f) s1_rect(x,-1.25f,0.6f,0.08f);
}

static void s1_drawBuilding(float x,float w,float h,float r,float g,float b,int colorBase)
{
    glColor3f(r,g,b); s1_rect(x,0,w,h);
    glColor3f(r*1.2f,g*1.2f,b*1.2f); glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x-w/2,0); glVertex2f(x-w/2,h);
    glVertex2f(x+w/2,h); glVertex2f(x+w/2,0);
    glEnd();

    float winW=0.28f,winH=0.35f;
    int rows=(int)(h/1.1f); if(rows<1)rows=1;
    int cols=(int)(w/0.9f); if(cols<1)cols=1;
    float startX=x-w/2+0.5f;

    for(int row=0;row<rows;row++){
        for(int col=0;col<cols;col++){
            float wy=0.5f+row*1.0f;
            float wx=startX+col*0.9f;
            if(wy>h-0.6f||wx>x+w/2-0.3f) continue;
            int idx=(colorBase+row*cols*2+col*3)%s1_winColorCount;
            if((row+col)%4==0)       glColor3f(1.0f,0.9f,0.5f);
            else if((row+col)%7==0)  glColor3f(0.3f,0.3f,0.5f);
            else                     glColor3fv(s1_winColors[idx]);
            s1_rect(wx,wy,winW,winH);
            glColor3f(0.2f,0.2f,0.25f); glLineWidth(1.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(wx-winW/2,wy); glVertex2f(wx+winW/2,wy);
            glVertex2f(wx+winW/2,wy+winH); glVertex2f(wx-winW/2,wy+winH);
            glEnd();
        }
    }
}

static void s1_drawTree(float x,float y)
{
    glColor3f(0.55f,0.35f,0.2f);  s1_rect(x,y,0.25f,0.5f);
    glColor3f(0.2f,0.6f,0.2f);    s1_circ(x,y+0.7f,0.45f,16);
    glColor3f(0.25f,0.65f,0.25f); s1_circ(x,y+1.0f,0.4f,16);
    glColor3f(0.3f,0.7f,0.3f);    s1_circ(x,y+1.25f,0.35f,16);
}

static void s1_drawLampPost(float x,float y)
{
    glColor3f(0.4f,0.4f,0.45f); s1_rect(x,y,0.1f,1.2f);
    glBegin(GL_LINES); glVertex2f(x,y+1.2f); glVertex2f(x+0.4f,y+1.2f); glEnd();
    glColor4f(1.0f,0.9f,0.4f,0.3f); s1_circ(x+0.4f,y+1.2f,0.25f,16);
    glColor3f(1.0f,0.8f,0.3f);      s1_circ(x+0.4f,y+1.2f,0.12f,12);
}

static void s1_drawStairs()
{
    float startX=-7.0f,endX=-3.8f,startY=0.0f,endY=2.4f;
    glColor3f(0.5f,0.5f,0.55f);
    glBegin(GL_QUADS);
    glVertex2f(startX,startY); glVertex2f(endX,endY);
    glVertex2f(endX,endY-0.25f); glVertex2f(startX+0.4f,startY);
    glEnd();
    int ns=12;
    float sw=(endX-startX)/ns, sh=(endY-startY)/ns;
    for(int i=0;i<ns;i++)
    {
        float br=0.6f+(i/(float)ns)*0.3f;
        glColor3f(br,br,br+0.05f);
        s1_rect(startX+i*sw+sw/2.0f, startY+i*sh, sw*0.9f, sh);
    }
    glColor3f(0.2f,0.2f,0.22f); glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(startX-0.1f,startY+0.9f); glVertex2f(endX+0.1f,endY+0.9f);
    glEnd();
    for(int i=0;i<=ns;i+=3)
        s1_rect(startX+i*sw-0.05f, startY+i*sh, 0.07f, 0.85f);
}

static void s1_drawConnectingWalkway()
{
    float pSX=-3.6f,pEX=3.7f,pY=2.4f,pW=0.6f;
    glColor3f(0.65f,0.65f,0.7f);
    s1_rect((pSX+pEX)*0.5f, pY, pEX-pSX, pW);
    glColor3f(0.3f,0.3f,0.35f); glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(pSX,pY);    glVertex2f(pEX,pY);
    glVertex2f(pSX,pY+pW); glVertex2f(pEX,pY+pW);
    glEnd();
    glColor3f(0.4f,0.4f,0.45f); glLineWidth(2.0f);
    glBegin(GL_LINES);
    for(float x=pSX;x<=pEX;x+=0.9f)
    {
        glVertex2f(x,pY+pW); glVertex2f(x,pY+pW+0.35f);
    }
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(pSX,pY+pW+0.35f); glVertex2f(pEX,pY+pW+0.35f);
    glEnd();
}

static void s1_drawBillboard()
{
    float bX=-13.8f,bY=-0.2f;
    glColor3f(0.4f,0.35f,0.25f); s1_rect(bX,bY,0.12f,2.2f);
    float boardX=bX+0.65f, boardY=bY+2.0f;
    glColor3f(0.0f,0.25f,0.0f); s1_rect(boardX,boardY,3.3f,0.85f);
    glColor3f(1.0f,1.0f,1.0f);
    s1_drawText("Uttara Center 700M",boardX-1.5f,boardY+0.6f);
    s1_drawText("Mirpur 11KM",  boardX-0.9f,boardY+0.38f);
    s1_drawText("N0 U-Turn",  boardX-0.9f,boardY+0.15f);
}

static void s1_drawStation()
{
    float xPos=6.3f, topY=2.4f;
    glColor3f(0.3f,0.3f,0.35f); s1_rect(xPos,0.0f,5.2f,topY);
    glColor3f(0.25f,0.4f,0.3f); s1_rect(0.0f,topY+1.2f,8.5f,0.35f);
    glColor3f(0.3f,0.45f,0.35f);s1_rect(0.0f,topY+1.4f,8.0f,0.15f);
    glColor3f(0.3f,0.3f,0.35f);
    s1_rect(-3.5f,0,0.4f,topY+1.2f);
    s1_rect( 3.1f,0,0.4f,topY+0.2f);
    glColor3f(0.8f,0.75f,0.65f); s1_rect(xPos,topY,5.2f,5.2f);
    for(int i=-1;i<=1;i++){
        glColor3f(0.7f,0.8f,0.9f);
        s1_rect(xPos+i*1.2f,topY+2.0f,0.9f,1.4f);
        glColor3f(0.1f,0.1f,0.15f); glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xPos+i*1.2f-0.45f,topY+2.0f);
        glVertex2f(xPos+i*1.2f+0.45f,topY+2.0f);
        glVertex2f(xPos+i*1.2f+0.45f,topY+3.4f);
        glVertex2f(xPos+i*1.2f-0.45f,topY+3.4f);
        glEnd();
    }
    glColor3f(0.7f,0.25f,0.35f); s1_rect(xPos,topY+4.0f,3.6f,0.8f);
    glColor3f(0.9f,0.85f,0.7f);
    s1_drawText("Uttara North",xPos-.90f,topY+4.5f);
    glColor3f(0.9f,0.9f,0.6f);
    s1_drawText("| MRT-6", xPos-.90f,topY+4.25f);
}

static void s1_drawMetroTrack()
{
    float tY=4.3f;
    float pX[]={-13.0f,-7.0f,-1.0f,7.0f,11.0f};
    for(int i=0;i<5;i++){
        glColor3f(0.6f,0.6f,0.65f); s1_rect(pX[i],0,0.85f,tY);
    }
    glColor3f(0.45f,0.45f,0.5f); s1_rect(0,tY,35.0f,0.35f);
    glColor3f(0.7f,0.7f,0.75f); glLineWidth(3.0f);
    glBegin(GL_LINES);
    for(float x=-14.5f;x<=14.5f;x+=0.5f){
        glVertex2f(x,tY+0.1f); glVertex2f(x,tY+0.25f);
    }
    glEnd();
    glColor3f(0.3f,0.2f,0.2f); glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-15.0f,S1_WIRE_Y); glVertex2f(15.0f,S1_WIRE_Y);
    glEnd();
    glColor3f(0.4f,0.4f,0.4f);
    for(int i=0;i<5;i++){
        glBegin(GL_LINES);
        glVertex2f(pX[i],tY); glVertex2f(pX[i],S1_WIRE_Y);
        glEnd();
        glColor3f(0.45f,0.45f,0.45f);
        s1_rect(pX[i],S1_WIRE_Y-0.1f,0.15f,0.2f);
    }
}

static void s1_drawMetroTrain(float x)
{
    float y=4.45f, tW=3.5f, tH=0.7f;
    glColor3f(0.2f,0.6f,0.8f);  s1_rect(x,y,tW,tH);
    glColor3f(0.9f,0.9f,0.9f);  s1_rect(x,y+0.25f,tW-0.2f,0.08f);
    glColor3f(0.6f,0.8f,1.0f);
    for(float dx=-1.2f;dx<=1.2f;dx+=0.8f) s1_rect(x+dx,y+0.35f,0.5f,0.25f);
    glColor3f(1.0f,0.8f,0.2f);
    s1_circ(x+tW/2-0.15f,y+0.15f,0.08f,8);
    s1_circ(x-tW/2+0.15f,y+0.15f,0.08f,8);
    glColor3f(0.3f,0.3f,0.3f); glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(x,y+tH); glVertex2f(x,S1_WIRE_Y);
    glEnd();
    glColor3f(0.2f,0.2f,0.2f); s1_rect(x,y+tH-0.05f,0.08f,0.1f);
    glColor3f(0.8f,0.2f,0.2f); s1_rect(x,S1_WIRE_Y-0.02f,0.5f,0.04f);
}

static void s1_drawCar(float x,float y,float r,float g,float b)
{
    glColor3f(r,g,b);         s1_rect(x,y+0.25f,1.8f,0.4f);
    glColor3f(r*0.7f,g*0.7f,b*0.7f); s1_rect(x,y+0.55f,1.0f,0.35f);
    glColor3f(0.6f,0.75f,0.9f);
    s1_rect(x-0.4f,y+0.6f,0.45f,0.25f);
    s1_rect(x+0.4f,y+0.6f,0.45f,0.25f);
    glColor3f(0.1f,0.1f,0.12f);
    s1_circ(x-0.6f,y+0.15f,0.22f,16); s1_circ(x+0.6f,y+0.15f,0.22f,16);
    glColor3f(0.4f,0.4f,0.45f);
    s1_circ(x-0.6f,y+0.15f,0.12f,8);  s1_circ(x+0.6f,y+0.15f,0.12f,8);
    glColor3f(1.0f,0.9f,0.3f); s1_circ(x+0.85f,y+0.3f,0.08f,8);
    glColor3f(0.9f,0.2f,0.2f); s1_circ(x-0.85f,y+0.3f,0.08f,8);
}

static void s1_drawPerson(float x,float y,float r,float g,float b,float size)
{
    glColor3f(0.2f,0.2f,0.25f);
    s1_rect(x-0.08f*size,y, 0.08f*size,0.35f*size);
    s1_rect(x+0.08f*size,y, 0.08f*size,0.35f*size);
    glColor3f(r,g,b); s1_rect(x,y+0.35f*size, 0.28f*size,0.5f*size);
    glColor3f(0.95f,0.85f,0.7f); s1_circ(x,y+0.95f*size,0.14f*size,16);
    glColor3f(r,g,b); glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(x-0.14f*size,y+0.75f*size); glVertex2f(x-0.35f*size,y+0.6f*size);
    glVertex2f(x+0.14f*size,y+0.75f*size); glVertex2f(x+0.35f*size,y+0.6f*size);
    glEnd();
    glColor3f(0.95f,0.85f,0.7f);
    s1_circ(x-0.35f*size,y+0.6f*size,0.06f*size,8);
    s1_circ(x+0.35f*size,y+0.6f*size,0.06f*size,8);
}

//scene-1 draw overlay hint
static void s1_drawHint()
{
    glColor4f(0.0f,0.0f,0.0f,0.55f);
    glBegin(GL_QUADS);
    glVertex2f(-15.0f,-2.0f); glVertex2f(15.0f,-2.0f);
    glVertex2f(15.0f,-1.65f); glVertex2f(-15.0f,-1.65f);
    glEnd();
    glColor3f(1.0f,1.0f,0.6f);
    s1_drawText("SCENE 1: Outdoor  |  Press SPACE or N for Scene 2 (Ticket Machine)",
                -8.5f, -1.93f);
}

//scene-1 display
static void s1_display()
{
    glClearColor(0.5f,0.7f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    s1_drawSkyGradient();
    s1_drawSun();
    for(int i=0;i<S1_MAX_CLOUDS;i++)
        s1_drawCloud(s1_clouds[i].x,s1_clouds[i].y,s1_clouds[i].size,s1_clouds[i].alpha);

    s1_drawBackground();

    s1_drawBuilding(-11.0f,8.2f,5.5f,0.5f,0.55f,0.6f,40);
    s1_drawBuilding( -4.5f,4.8f,7.8f,0.6f,0.5f, 0.45f,30);
    s1_drawBuilding( -1.0f,3.0f,6.5f,0.35f,0.55f,0.5f,0);
    s1_drawBuilding(  5.5f,10.5f,3.0f,0.55f,0.45f,0.5f,55);
    s1_drawBuilding( 13.0f,4.5f,7.0f,0.7f, 0.55f,0.45f,40);

    s1_drawMetroTrack();
    s1_drawMetroTrain(s1_metroPos);
    s1_drawStation();
    s1_drawStairs();
    s1_drawConnectingWalkway();

    for(int i=0;i<S1_MAX_LAMPS;i++) s1_drawLampPost(s1_lamps[i].x,s1_lamps[i].y);

    float treeX[]={-11.0f,-2.0f,2.5f,8.0f,13.0f};
    for(int i=0;i<5;i++) s1_drawTree(treeX[i],-0.3f);

    s1_drawBillboard();

    for(int i=0;i<S1_MAX_PEOPLE;i++)
        s1_drawPerson(s1_people[i].pos,0.0f,
                      s1_people[i].r,s1_people[i].g,s1_people[i].b,
                      s1_people[i].size);

    s1_drawCar(s1_car1Pos,-1.0f,0.25f,0.65f,0.85f);
    s1_drawCar(s1_car2Pos,-2.1f,0.85f,0.35f,0.25f);

    s1_drawHint();
    glutSwapBuffers();
}

//scene-1 update (timer tick)
static void s1_step(){
    s1_car1Pos += 0.1f;  if(s1_car1Pos> 16.0f) s1_car1Pos=-16.0f;
    s1_car2Pos -= 0.08f; if(s1_car2Pos<-16.0f) s1_car2Pos= 16.0f;

    s1_metroPos += 0.045f*s1_metroDir;
    if(s1_metroPos>14.0f||s1_metroPos<-14.0f){
        s1_metroDir*=-1; s1_metroPos+=s1_metroDir*0.045f;
    }
    for(int i=0;i<S1_MAX_CLOUDS;i++)
    {
        s1_clouds[i].x+=s1_clouds[i].speed;
        if(s1_clouds[i].x> 18.0f) s1_clouds[i].x=-18.0f;
        if(s1_clouds[i].x<-18.0f) s1_clouds[i].x= 18.0f;
    }
    for(int i=0;i<S1_MAX_PEOPLE;i++)
    {
        s1_people[i].pos+=s1_people[i].speed;
        if(s1_people[i].pos>14.5f||s1_people[i].pos<-14.5f)
            s1_people[i].speed*=-1;
    }
}

//scene-1 projection
static void s1_setProjection(int w,int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(-15.0,15.0,-2.0,10.0);
    glMatrixMode(GL_MODELVIEW);
}


/* ============================================================
   ============================================================
   SCENE 2 – MRT-6 TICKET VENDING MACHINE INTERIOR
   (All identifiers prefixed  s2_  to avoid clashes)
   ============================================================
   ============================================================ */

static const int   S2_WW = 1100;
static const int   S2_WH = 680;
static const float S2_PI = 3.14159265f;
static float s2_gTime=0.0f, s2_lastTime=0.0f;

static const float S2_FLOOR_Y = 200.f;
static const float S2_MW      = 112.f;
static const float S2_MH      = 240.f;
static const float S2_MY      = S2_FLOOR_Y;
static const float S2_MGAP    = 178.f;
static float s2_MCX[3];

//scene-2 primitive wrappers
static void s2_col3(float r,float g,float b){glColor3f(r,g,b);}
static void s2_col4(float r,float g,float b,float a){glColor4f(r,g,b,a);}

static void s2_quad(float x,float y,float w,float h){
    glBegin(GL_QUADS);
    glVertex2f(x,y);   glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}
static void s2_gquad(float x,float y,float w,float h,
                     float r0,float g0,float b0,
                     float r1,float g1,float b1){
    glBegin(GL_QUADS);
    glColor3f(r0,g0,b0); glVertex2f(x,y);   glVertex2f(x+w,y);
    glColor3f(r1,g1,b1); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}
static void s2_circ(float cx,float cy,float r,int s=24)
{
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(int i=0;i<=s;i++){float a=2*S2_PI*i/s;
        glVertex2f(cx+cosf(a)*r,cy+sinf(a)*r);}
    glEnd();
}
static void s2_ocirc(float cx,float cy,float r,int s,float lw){
    glLineWidth(lw); glBegin(GL_LINE_LOOP);
    for(int i=0;i<s;i++){float a=2*S2_PI*i/s;
        glVertex2f(cx+cosf(a)*r,cy+sinf(a)*r);}
    glEnd(); glLineWidth(1);
}
static void s2_rrcorner(float cx,float cy,float r,float sa){
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(int i=0;i<=8;i++){float a=sa+i*(S2_PI/2)/8;
        glVertex2f(cx+cosf(a)*r,cy+sinf(a)*r);}
    glEnd();
}
static void s2_rrect(float x,float y,float w,float h,float r){
    if(r<=0){s2_quad(x,y,w,h);return;}
    s2_quad(x+r,y,w-2*r,h); s2_quad(x,y+r,r,h-2*r); s2_quad(x+w-r,y+r,r,h-2*r);
    s2_rrcorner(x+r,y+r,r,S2_PI);       s2_rrcorner(x+w-r,y+r,r,1.5f*S2_PI);
    s2_rrcorner(x+w-r,y+h-r,r,0);       s2_rrcorner(x+r,y+h-r,r,0.5f*S2_PI);
}
static void s2_txt(float x,float y,const char*s,void*f=GLUT_BITMAP_HELVETICA_12){
    glRasterPos2f(x,y); for(;*s;s++) glutBitmapCharacter((void*)f,*s);
}

//scene-2 structs
struct S2_Person {
    float x,y,stopX;
    int   machIdx,phase;
    float timer,wc;
    float armR,armL,headTilt;
    bool  hasTicket,billVis;
    float billX,billY,billTargX;
    float procAngle,screenGlow;
    float sr,sg,sb,skR;
    const char*dest,*fare;
};

static const int S2_NP=3;
static S2_Person s2_P[S2_NP];

struct S2_Queuer{
    float x,y,cr,cg,cb,skR,fidgetT;
    bool  fidget;
    float walkWC;
    int   walkDir;
    float minX,maxX;
};
static S2_Queuer s2_QP[2];

static void s2_initPerson(int i){
    S2_Person &p=s2_P[i];
    p.machIdx=i;
    p.stopX=s2_MCX[i]-68.f;
    p.x=-80.f-i*165.f;
    p.y=S2_FLOOR_Y-18.f;
    p.phase=0; p.timer=0; p.wc=0;
    p.armR=0; p.armL=0; p.headTilt=0;
    p.hasTicket=false; p.billVis=false;
    p.billX=0; p.billY=0; p.billTargX=0;
    p.procAngle=0; p.screenGlow=0;
    static const float shirts[3][3]={
        {0.16f,0.26f,0.56f},{0.52f,0.16f,0.16f},{0.14f,0.40f,0.22f}};
    p.sr=shirts[i][0]; p.sg=shirts[i][1]; p.sb=shirts[i][2];
    static const float skins[3]={0.72f,0.65f,0.76f};
    p.skR=skins[i];
    static const char*dests[3]={"Motijheel","Farmgate","Pallabi"};
    static const char*fares[3]={"40 BDT","30 BDT","15 BDT"};
    p.dest=dests[i]; p.fare=fares[i];
}

static void s2_initQueuers(){
    s2_QP[0]={120.f,S2_FLOOR_Y-18.f,0.50f,0.28f,0.55f,0.68f,0,false,0,  1,60.f,300.f};
    s2_QP[1]={220.f,S2_FLOOR_Y-18.f,0.72f,0.54f,0.16f,0.73f,0,false,1.6f,-1,60.f,300.f};
}

//scene-2 drawing
static void s2_drawTicket(float tx,float ty,float ang,
                          const char*dest="MRT",const char*fare="BDT"){
    glPushMatrix();
    glTranslatef(tx,ty,0); glRotatef(ang,0,0,1);
    s2_col3(0.96f,0.96f,0.89f); s2_rrect(-19,-9,28,8,2);
    s2_col3(0.04f,0.48f,0.20f); s2_quad(-19,0,30,5);
    s2_col3(0.08f,0.08f,0.08f);
    for(int b=0;b<9;b++) s2_quad(-17+b*3.4f,-8,2.f,7);
    glPopMatrix();
}

static void s2_drawBill(float bx,float by){
    s2_col3(0.48f,0.76f,0.48f); s2_rrect(bx-24,by-10,48,20,2);
    s2_col3(0.36f,0.62f,0.36f); s2_quad(bx-24,by-3,48,6);
    s2_col3(0.09f,0.26f,0.09f);
    s2_txt(bx-20,by+3,"50", GLUT_BITMAP_HELVETICA_12);
    s2_txt(bx+2, by-6,"BDT",GLUT_BITMAP_HELVETICA_10);
    s2_col4(0,0.22f,0,0.38f);
    for(int i=0;i<4;i++) s2_quad(bx-22+i*11,by+7,8,2);
}

static void s2_drawArm(float ox,float oy,
                       float uAng,float fAng,
                       float sr,float sg,float sb,float skR,
                       bool holdTicket,int phase,
                       const char*dest="MRT",const char*fare="BDT")
{
    float skG=skR*0.71f, skB=skR*0.50f;
    float UA=22.f, FA=19.f;
    glPushMatrix();
    glTranslatef(ox,oy,0); glRotatef(uAng,0,0,1);
    s2_col3(sr,sg,sb);
    s2_quad(-4,-UA,8,UA);
    s2_col3(sr*0.82f,sg*0.82f,sb*0.82f); s2_circ(0,-UA,4,8);
    glPushMatrix();
    glTranslatef(0,-UA,0); glRotatef(fAng,0,0,1);
    s2_col3(sr,sg,sb); s2_quad(-3,-FA,7,FA);
    s2_col3(skR,skG,skB); s2_circ(0.5f,-FA,2.8f,8);
    s2_col3(skR,skG,skB); s2_rrect(-4,-FA-7,8,7,2);
    for(int f=0;f<4;f++){
        float fx=-3.5f+f*2.4f;
        s2_col3(skR,skG,skB);  s2_quad(fx,-FA-7-5,1.8f,5);
        s2_col3(skR*0.88f,skG*0.88f,skB*0.88f);
        s2_circ(fx+0.9f,-FA-12,1.0f,5);
    }
    glPushMatrix();
    glTranslatef(4,-FA-3,0); glRotatef(-35,0,0,1);
    s2_col3(skR,skG,skB); s2_quad(-1,-5,3,5); s2_circ(0.5f,-5,1.2f,5);
    glPopMatrix();
    if(holdTicket&&(phase==6||phase==7))
        s2_drawTicket(0,-FA-7-6,0,dest,fare);
    glPopMatrix();
    glPopMatrix();
}

static void s2_drawPerson(float px,float py,
                          float wc,int ph,
                          float armR_,float armL_,float ht,
                          float sr,float sg,float sb,float skR,
                          bool walking,bool hasTicket,
                          const char*dest="MRT",const char*fare="BDT")
{
    float skG=skR*0.71f, skB=skR*0.50f;
    float pr=0.14f,pg=0.14f,pb=0.20f;
    float thigh=30.f,shin=26.f;
    float hipY=py+62.f;
    float legSw=walking?sinf(wc)*18.f:0.f;
    float lSwing=legSw, rSwing=-legSw;
    float lKnee=walking?(lSwing<0?-lSwing*0.5f:0):0;
    float rKnee=walking?(rSwing<0?-rSwing*0.5f:0):0;
    float armSw=walking?sinf(wc)*14.f:0.f;
    float laU=walking?-armSw:armL_;
    float raU=walking? armSw:armR_;
    float laF=0,raF=0;
    if(!walking){
        switch(ph){
        case 2: laF=6;   raF=15; break;
        case 3: raF=(armR_>16)?30:6; laF=5; break;
        case 4: raF=25;  laF=5;  break;
        case 5: laF=6;   raF=6;  break;
        case 6: raF=(armR_>36)?36:15; laF=5; break;
        default:laF=6;   raF=6;  break;
        }
    }
    else
    {
        laF=(armSw<-4)?-armSw*0.3f:0;
        raF=(armSw> 4)? armSw*0.3f:0;
    }
    s2_col4(0.35f,0.32f,0.29f,0.28f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(px,py+1);
    for(int i=0;i<=16;i++)
    {
        float a=(float)i/16*2*S2_PI;
        glVertex2f(px+cosf(a)*17,py+sinf(a)*4);
    }
    glEnd();
    /* left leg */
    glPushMatrix();
    glTranslatef(px-7,hipY,0); glRotatef(lSwing,0,0,1);
    s2_gquad(-5,-thigh,10,thigh,pr*0.88f,pg*0.88f,pb*0.88f,pr,pg,pb);
    glTranslatef(0,-thigh,0); glRotatef(lKnee,0,0,1);
    s2_gquad(-4,-shin,8,shin,pr*0.78f,pg*0.78f,pb*0.78f,pr*0.88f,pg*0.88f,pb*0.88f);
    glTranslatef(0,-shin,0);
    s2_col3(0.08f,0.06f,0.05f); s2_rrect(-6,-6,14,6,2);
    s2_col3(0.14f,0.10f,0.09f); s2_rrect(4,-5,4,5,1);
    glPopMatrix();
    /* right leg */
    glPushMatrix();
    glTranslatef(px+7,hipY,0); glRotatef(rSwing,0,0,1);
    s2_gquad(-5,-thigh,10,thigh,pr*0.88f,pg*0.88f,pb*0.88f,pr,pg,pb);
    glTranslatef(0,-thigh,0); glRotatef(rKnee,0,0,1);
    s2_gquad(-4,-shin,8,shin,pr*0.78f,pg*0.78f,pb*0.78f,pr*0.88f,pg*0.88f,pb*0.88f);
    glTranslatef(0,-shin,0);
    s2_col3(0.08f,0.06f,0.05f); s2_rrect(-6,-6,14,6,2);
    s2_col3(0.14f,0.10f,0.09f); s2_rrect(4,-5,4,5,1);
    glPopMatrix();
    /* torso */
    float torsoBot=62.f,torsoH=44.f,torsoTop=torsoBot+torsoH;
    s2_gquad(px-12,py+torsoBot,24,torsoH,
             sr*0.86f,sg*0.86f,sb*0.86f,sr,sg,sb);
    s2_rrect(px-12,py+torsoBot,24,torsoH,4);
    s2_col3(sr*0.68f,sg*0.68f,sb*0.68f); s2_quad(px-1.5f,py+torsoBot+2,3,28);
    s2_col3(0.80f,0.80f,0.82f);
    for(int b=0;b<3;b++) s2_circ(px,py+torsoBot+5+b*8,1.4f,5);
    /* collar */
    s2_col3(0.92f,0.92f,0.90f);
    glBegin(GL_TRIANGLES);
    glVertex2f(px-5,py+torsoTop); glVertex2f(px+5,py+torsoTop);
    glVertex2f(px,  py+torsoTop-12);
    glEnd();
    s2_col3(0.78f,0.78f,0.76f); glLineWidth(0.7f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(px-5,py+torsoTop); glVertex2f(px,py+torsoTop-12);
    glVertex2f(px+5,py+torsoTop);
    glEnd(); glLineWidth(1);
    /* arms */
    float shY=py+torsoTop-4;
    s2_drawArm(px-11,shY,laU,laF,sr,sg,sb,skR,false,    ph,dest,fare);
    s2_drawArm(px+11,shY,raU,raF,sr,sg,sb,skR,hasTicket,ph,dest,fare);
    /* neck */
    s2_col3(skR,skG,skB); s2_rrect(px-3,py+torsoTop,6,9,2);
    /* head */
    float hcY=py+torsoTop+9+12.f;
    s2_col3(skR,skG,skB); s2_circ(px,hcY,12.5f,20);
    s2_col3(skR*1.03f,skG*1.03f,skB*1.03f); s2_circ(px,hcY-3,9,16);
    glPushMatrix();
    glTranslatef(px,hcY,0); glRotatef(ht,0,0,1);
    s2_col3(0.07f,0.05f,0.05f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0,0);
    for(int i=0;i<=14;i++){float a=S2_PI*0.08f+(float)i/14*S2_PI*1.04f;
        glVertex2f(cosf(a)*12.5f,sinf(a)*12.5f);}
    glEnd();
    s2_col3(0.08f,0.05f,0.04f); glLineWidth(1.7f);
    glBegin(GL_LINE_STRIP);glVertex2f(-6.5f,2.5f);glVertex2f(-4,3.5f);glVertex2f(-1.5f,2.5f);glEnd();
    glBegin(GL_LINE_STRIP);glVertex2f( 1.5f,2.5f);glVertex2f( 4,3.5f);glVertex2f( 6.5f,2.5f);glEnd();
    glLineWidth(1);
    s2_col3(0.94f,0.94f,0.92f);
    s2_rrect(-7,-1,5.5f,4,2); s2_rrect(1.5f,-1,5.5f,4,2);
    s2_col3(0.22f,0.14f,0.07f); s2_circ(-4.2f,1,2.f,8); s2_circ(4.2f,1,2.f,8);
    s2_col3(0.04f,0.03f,0.03f); s2_circ(-4.2f,1,1.1f,7); s2_circ(4.2f,1,1.1f,7);
    s2_col3(1,1,1); s2_circ(-3.5f,1.6f,0.5f,5); s2_circ(4.9f,1.6f,0.5f,5);
    s2_col3(skR*0.86f,skG*0.86f,skB*0.86f);
    s2_circ(-1.4f,-3,1.2f,7); s2_circ(1.4f,-3,1.2f,7);
    s2_col3(0.50f,0.28f,0.20f); glLineWidth(1.4f);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=8;i++){
        float a=S2_PI+(float)i/8*S2_PI;
        float sm=(ph==7)?1.8f:0.4f;
        glVertex2f(cosf(a)*3.5f,-7+sinf(a)*sm);
    }
    glEnd(); glLineWidth(1);
    s2_col3(skR,skG,skB); s2_circ(-12,0,3,7);
    s2_col3(skR*0.88f,skG*0.88f,skB*0.88f); s2_circ(-12,0,1.6f,6);
    glPopMatrix();
}

static void s2_drawQueuer(S2_Queuer*q){
    float x=q->x,y=q->y;
    float skG=q->skR*0.71f,skB=q->skR*0.50f;
    float pr=0.14f,pg=0.14f,pb=0.20f;
    float thigh=28.f,shin=24.f;
    float hipY=y+thigh+shin;
    float legSw=sinf(q->walkWC)*14.f;
    float lLeg= legSw*q->walkDir, rLeg=-legSw*q->walkDir;
    float lKnee=(lLeg<0)?-lLeg*0.45f:0;
    float rKnee=(rLeg<0)?-rLeg*0.45f:0;
    float armSw=sinf(q->walkWC)*11.f;
    s2_col4(0.35f,0.32f,0.29f,0.22f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(x,y+1);
    for(int i=0;i<=12;i++){float a=(float)i/12*2*S2_PI;
        glVertex2f(x+cosf(a)*14,y+sinf(a)*3);}
    glEnd();
    glPushMatrix();
    glTranslatef(x-5,hipY,0); glRotatef(lLeg,0,0,1);
    s2_gquad(-4,-thigh,8,thigh,pr*0.88f,pg*0.88f,pb*0.88f,pr,pg,pb);
    glTranslatef(0,-thigh,0); glRotatef(lKnee,0,0,1);
    s2_gquad(-3,-shin,7,shin,pr*0.78f,pg*0.78f,pb*0.78f,pr*0.88f,pg*0.88f,pb*0.88f);
    glTranslatef(0,-shin,0);
    s2_col3(0.08f,0.06f,0.05f); s2_rrect(-5,-5,12,5,2);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(x+5,hipY,0); glRotatef(rLeg,0,0,1);
    s2_gquad(-4,-thigh,8,thigh,pr*0.88f,pg*0.88f,pb*0.88f,pr,pg,pb);
    glTranslatef(0,-thigh,0); glRotatef(rKnee,0,0,1);
    s2_gquad(-3,-shin,7,shin,pr*0.78f,pg*0.78f,pb*0.78f,pr*0.88f,pg*0.88f,pb*0.88f);
    glTranslatef(0,-shin,0);
    s2_col3(0.08f,0.06f,0.05f); s2_rrect(-5,-5,12,5,2);
    glPopMatrix();
    float bBot=hipY-y,bTop=bBot+38.f;
    s2_gquad(x-10,y+bBot,20,38,q->cr*0.86f,q->cg*0.86f,q->cb*0.86f,q->cr,q->cg,q->cb);
    s2_rrect(x-10,y+bBot,20,38,3);
    float shY2=y+bTop-4;
    for(int s=-1;s<=1;s+=2){
        float sw=s*armSw*q->walkDir;
        glPushMatrix();
        glTranslatef(x+s*9,shY2,0); glRotatef(sw,0,0,1);
        s2_col3(q->cr,q->cg,q->cb); s2_quad(-3,-17,7,17);
        glTranslatef(0,-17,0); glRotatef(sw*0.3f,0,0,1);
        s2_col3(q->cr,q->cg,q->cb); s2_quad(-3,-15,7,15);
        s2_col3(q->skR,skG,skB); s2_rrect(-3,-22,7,7,1);
        glPopMatrix();
    }
    s2_col3(q->skR,skG,skB); s2_rrect(x-3,y+bTop,6,8,2);
    float hcY=y+bTop+8+10.f;
    s2_col3(q->skR,skG,skB); s2_circ(x,hcY,11.f,20);
    s2_col3(0.07f,0.05f,0.05f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(x,hcY);
    for(int i=0;i<=13;i++){float a=S2_PI*0.08f+(float)i/13*S2_PI*1.04f;
        glVertex2f(x+cosf(a)*11.f,hcY+sinf(a)*11.f);}
    glEnd();
    s2_col3(0.04f,0.03f,0.03f); s2_circ(x-3,hcY,1.4f,7); s2_circ(x+3,hcY,1.4f,7);
    s2_col3(0.48f,0.26f,0.18f); glLineWidth(1.2f);
    glBegin(GL_LINES); glVertex2f(x-2,hcY-5); glVertex2f(x+2,hcY-5); glEnd();
    glLineWidth(1);
}

static void s2_drawStation(){
    s2_gquad(0,570,S2_WW,110, 0.86f,0.86f,0.86f, 0.96f,0.96f,0.96f);
    s2_col3(0.75f,0.75f,0.75f);
    for(int i=0;i<8;i++) s2_quad(i*155.f,602,118,68);
    s2_col3(0.91f,0.91f,0.91f);
    for(int i=0;i<8;i++) s2_quad(i*155.f+118,585,37,85);
    for(int i=0;i<10;i++){
        float lx=55+i*105;
        s2_col3(0.72f,0.72f,0.72f); s2_quad(lx-20,612,40,14);
        float gl=0.91f+sinf(s2_gTime*0.4f+i*0.8f)*0.04f;
        s2_col4(gl,gl,gl*0.90f,0.85f); s2_quad(lx-16,613,32,11);
        glBegin(GL_TRIANGLES);
        s2_col4(1,1,0.95f,0.06f); glVertex2f(lx,613);
        s2_col4(1,1,0.95f,0);     glVertex2f(lx-75,500); glVertex2f(lx+75,500);
        glEnd();
    }
    s2_gquad(0,S2_FLOOR_Y,S2_WW,380, 0.89f,0.88f,0.85f, 0.94f,0.93f,0.91f);
    s2_col3(0.04f,0.52f,0.22f); s2_quad(0,385,S2_WW,18);
    s2_col3(0.03f,0.40f,0.17f); s2_quad(0,381,S2_WW,5);
    s2_col3(0.04f,0.52f,0.22f); s2_quad(0,558,S2_WW,12);
    glLineWidth(0.5f); s2_col4(0,0,0,0.05f);
    for(int tx=0;tx<S2_WW;tx+=52){
        glBegin(GL_LINES);glVertex2f(tx,S2_FLOOR_Y);glVertex2f(tx,410);glEnd();}
    for(int ty=(int)S2_FLOOR_Y;ty<410;ty+=46){
        glBegin(GL_LINES);glVertex2f(0,ty);glVertex2f(S2_WW,ty);glEnd();}
    glLineWidth(1);
    for(int i=0;i<4;i++){
        float px2=130+i*270;
        s2_gquad(px2-17,S2_FLOOR_Y,34,400, 0.79f,0.79f,0.77f, 0.87f,0.87f,0.85f);
        s2_col3(0.04f,0.52f,0.22f); s2_quad(px2-17,383,34,20);
        s2_col4(0,0,0,0.07f); s2_quad(px2-17,S2_FLOOR_Y,4,400); s2_quad(px2+13,S2_FLOOR_Y,4,400);
    }
    s2_gquad(0,0,S2_WW,S2_FLOOR_Y, 0.75f,0.73f,0.70f, 0.84f,0.83f,0.80f);
    for(int tx=0;tx<S2_WW/72+1;tx++)
        for(int ty=0;ty<5;ty++){
            float sh=0.79f+((tx+ty)%2)*0.05f;
            s2_col3(sh,sh*0.99f,sh*0.97f); s2_quad(tx*72,ty*50,71,49);
        }
    s2_col4(1,1,1,0.09f); s2_quad(0,S2_FLOOR_Y-6,S2_WW,6);
    s2_col3(0.93f,0.77f,0.04f); s2_quad(0,S2_FLOOR_Y-14,S2_WW,14);
    s2_col3(0.75f,0.61f,0.03f);
    for(int d=0;d<56;d++) s2_quad(d*20+4,S2_FLOOR_Y-12,9,10);
    /* station sign */
    s2_col3(0.03f,0.45f,0.18f); s2_rrect(14,588,200,34,5);
    s2_col3(1,1,1); s2_txt(36,601,"Uttara North | MRT-6",GLUT_BITMAP_HELVETICA_12);
    s2_col3(0.50f,0.50f,0.50f); glLineWidth(1.4f);
    glBegin(GL_LINES);
    glVertex2f(S2_WW/2-62,628); glVertex2f(S2_WW/2-62,585);
    glVertex2f(S2_WW/2+62,628); glVertex2f(S2_WW/2+62,585);
    glEnd(); glLineWidth(1);
    s2_col3(0.03f,0.45f,0.18f); s2_rrect(S2_WW/2-84,564,168,26,4);
    s2_col3(1,1,1); s2_txt(S2_WW/2-76,572,"Ticket Vending Machine",GLUT_BITMAP_HELVETICA_12);
    s2_col3(0.70f,0.04f,0.04f); s2_rrect(S2_WW-160,588,94,28,4);
    s2_col3(1,1,1); s2_txt(S2_WW-146,598,"EXIT -->",GLUT_BITMAP_HELVETICA_12);
    s2_col3(0.24f,0.24f,0.24f); s2_rrect(S2_WW-58,600,40,18,3);
    s2_circ(S2_WW-22,609,6,8); s2_col3(0.08f,0.08f,0.08f); s2_circ(S2_WW-22,609,3,8);
    s2_col4(0.8f,0.1f,0.1f,0.8f+sinf(s2_gTime*1.5f)*0.2f); s2_circ(S2_WW-45,604,3,6);
}

static void s2_drawMachine(float cx,float sGlow,float pAngle,bool active){
    float x=cx-S2_MW/2, y=S2_MY;
    s2_col4(0,0,0,0.12f);
    for(int s=6;s>0;s--) s2_rrect(x+s,y-s,S2_MW,S2_MH,9);
    s2_gquad(x,y,S2_MW,S2_MH, 0.20f,0.20f,0.21f, 0.28f,0.28f,0.29f);
    s2_rrect(x,y,S2_MW,S2_MH,9);
    s2_col3(0.35f,0.35f,0.36f); s2_quad(x,y,3,S2_MH); s2_quad(x+S2_MW-3,y,3,S2_MH);
    s2_col3(0.25f,0.25f,0.26f); s2_rrect(x+5,y+5,S2_MW-10,S2_MH-10,7);
    float hy=y+S2_MH-46;
    s2_col3(0.04f,0.50f,0.20f); s2_rrect(x+5,hy,S2_MW-10,41,5);
    s2_col3(1,1,1); s2_circ(x+18,hy+21,8,16);
    s2_col3(0.04f,0.50f,0.20f); s2_circ(x+18,hy+21,5,16);
    s2_col3(1,1,1); s2_txt(x+13,hy+18,"M",GLUT_BITMAP_HELVETICA_10);
    s2_col3(1,1,1);
    s2_txt(x+30,hy+30,"Ticket Vending",GLUT_BITMAP_HELVETICA_10);
    s2_txt(x+38,hy+17,"Machine",       GLUT_BITMAP_HELVETICA_10);
    float sx=x+8,sy=y+118,sw=S2_MW-16,sh=88;
    s2_col3(0.09f,0.09f,0.10f); s2_rrect(sx-2,sy-2,sw+4,sh+4,4);
    float sg2=active?(0.10f+sGlow*0.5f):0.07f;
    if(active && pAngle>0.05f){
        s2_col3(sg2*0.2f,sg2*0.85f,sg2*0.5f); s2_rrect(sx,sy,sw,sh,3);
        s2_col3(0.88f,0.95f,0.88f);
        s2_txt(sx+5,sy+70,"Processing...",GLUT_BITMAP_HELVETICA_10);
        s2_txt(sx+8,sy+55,"Please wait",  GLUT_BITMAP_HELVETICA_10);
        glLineWidth(2.8f); s2_col3(0.10f,0.90f,0.45f);
        glBegin(GL_LINE_STRIP);
        for(int i=0;i<=22;i++){float a=pAngle+(float)i/22*(S2_PI*1.6f);
            glVertex2f(sx+sw/2+cosf(a)*13,sy+28+sinf(a)*13);}
        glEnd(); glLineWidth(1);
        s2_col3(0.82f,1.f,0.85f);
        s2_txt(sx+6,sy+10,"Amount: 40 BDT",GLUT_BITMAP_HELVETICA_10);
    } else {
        s2_col3(sg2*0.25f,sg2*0.72f,sg2*0.44f); s2_rrect(sx,sy,sw,sh,3);
        s2_col3(0.03f,0.38f,0.16f); s2_quad(sx,sy+sh-14,sw,14);
        s2_col3(0.82f,1.f,0.85f); s2_txt(sx+4,sy+sh-11,"Select Destination",GLUT_BITMAP_HELVETICA_10);
        struct Btn{const char*d,*t;float r,g,b;};
        static const Btn btns[4]={
            {"Motijheel","40 BDT",0.04f,0.38f,0.16f},
            {"Farmgate", "30 BDT",0.10f,0.28f,0.55f},
            {"Agargaon", "20 BDT",0.45f,0.28f,0.05f},
            {"Pallabi",  "15 BDT",0.38f,0.06f,0.38f}
        };
        for(int b=0;b<4;b++){
            float bx2=sx+3+(b%2)*(sw/2-2), by2=sy+3+(b/2)*34;
            float bw=sw/2-5, bh=30;
            s2_col4(0,0,0,0.28f); s2_rrect(bx2+1,by2-1,bw,bh,3);
            s2_col3(btns[b].r,btns[b].g,btns[b].b); s2_rrect(bx2,by2,bw,bh,3);
            s2_col4(1,1,1,0.11f); s2_quad(bx2,by2+bh-5,bw,5);
            s2_col3(1,1,1);        s2_txt(bx2+3,by2+bh-10,btns[b].d,GLUT_BITMAP_HELVETICA_10);
            s2_col3(0.82f,1,0.85f);s2_txt(bx2+3,by2+4,    btns[b].t,GLUT_BITMAP_HELVETICA_10);
        }
        if(active&&(int)(s2_gTime*2)%2==0){
            s2_col4(1,1,0.5f,0.75f);
            s2_txt(sx+6,sy+2,"Touch to select",GLUT_BITMAP_HELVETICA_10);
        }
    }
    s2_col3(0.15f,0.15f,0.16f); s2_rrect(x+8,y+111,S2_MW-16,6,2);
    s2_col3(0.52f,0.52f,0.52f); s2_txt(x+10,y+113,"EN | Help",GLUT_BITMAP_HELVETICA_10);
    s2_col3(0.13f,0.13f,0.14f); s2_rrect(x+8,y+78,S2_MW-16,28,4);
    s2_col4(0,0,0,0.88f); s2_quad(x+14,y+85,S2_MW-28,11);
    s2_col4(1,1,1,0.07f); s2_quad(x+14,y+85,S2_MW-28,2);
    s2_col3(0.55f,0.55f,0.55f); s2_txt(x+10,y+95,"INSERT NOTE",GLUT_BITMAP_HELVETICA_10);
    s2_col3(0.13f,0.13f,0.14f); s2_rrect(x+8,y+57,S2_MW-16,18,4);
    s2_col4(0,0,0,0.88f); s2_circ(cx,y+66,7,14);
    s2_ocirc(cx,y+66,7,14,1.4f);
    s2_col3(0.52f,0.52f,0.52f); s2_txt(x+10,y+59,"COIN",GLUT_BITMAP_HELVETICA_10);
    s2_col3(0.11f,0.11f,0.12f); s2_rrect(x+8,y+17,S2_MW-16,34,4);
    s2_col4(0,0,0,0.83f); s2_quad(x+16,y+27,S2_MW-32,10);
    s2_col3(0.33f,0.33f,0.33f);
    for(int r=0;r<5;r++) s2_quad(x+18+r*13,y+27,7,10);
    s2_col3(0.52f,0.52f,0.52f); s2_txt(x+10,y+19,"COLLECT TICKET",GLUT_BITMAP_HELVETICA_10);
    s2_col3(0.16f,0.16f,0.17f); s2_rrect(x-5,y-8,S2_MW+10,12,3);
    s2_col3(0.12f,0.12f,0.13f); s2_quad(cx-14,y-14,28,8);
    float ledG=active?(0.82f+sinf(s2_gTime*1.8f)*0.1f):0.50f;
    s2_col3(active?0.08f:0.02f, ledG, 0.18f);
    s2_circ(x+S2_MW-13,y+S2_MH-12,4,10);
}

static void s2_drawBubble(S2_Person&p){
    if(p.phase<2||p.phase>5) return;
    float bx=p.x+16, by=p.y+140;
    s2_col4(1,1,1,0.92f); s2_rrect(bx,by,114,28,5);
    glBegin(GL_TRIANGLES);
    s2_col4(1,1,1,0.92f);
    glVertex2f(bx+8,by); glVertex2f(bx+20,by); glVertex2f(bx+10,by-8);
    glEnd();
    s2_col4(0.05f,0.50f,0.20f,0.65f); glLineWidth(1.3f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx,by);     glVertex2f(bx+114,by);
    glVertex2f(bx+114,by+28); glVertex2f(bx,by+28);
    glEnd(); glLineWidth(1);
    char topbuf[64],botbuf[64];
    if(p.phase==2){
        snprintf(topbuf,sizeof(topbuf),"Going %s!",p.dest);
        snprintf(botbuf,sizeof(botbuf),"(%s select)",p.dest);
    } else if(p.phase==3){
        snprintf(topbuf,sizeof(topbuf),"%s confirmed!",p.fare);
        snprintf(botbuf,sizeof(botbuf),"(Confirming fare)");
    } else {
        snprintf(topbuf,sizeof(topbuf),"Inserting %s",p.fare);
        snprintf(botbuf,sizeof(botbuf),"(Note inserting)");
    }
    s2_col3(0.05f,0.05f,0.10f); s2_txt(bx+5,by+17,topbuf,GLUT_BITMAP_HELVETICA_12);
    s2_col3(0.20f,0.20f,0.50f); s2_txt(bx+5,by+5, botbuf,GLUT_BITMAP_HELVETICA_10);
}

static const char*s2_phLbl[]=
    {"Walking in","Approaching","Selecting","Confirming",
     "Inserting note","Processing","Collecting","Walking away","Done"};

static void s2_drawHUD(){
    s2_col4(0.04f,0.04f,0.07f,0.90f); s2_quad(0,0,S2_WW,24);
    s2_col3(0.05f,0.70f,0.28f);
    char buf[300];
    snprintf(buf,sizeof(buf),
        "Dhaka MRT-6  |  B1:%s  B2:%s  B3:%s  |  2 in queue  |  SPACE=Scene1  R=Restart  ESC=Exit",
        s2_phLbl[s2_P[0].phase<9?s2_P[0].phase:8],
        s2_phLbl[s2_P[1].phase<9?s2_P[1].phase:8],
        s2_phLbl[s2_P[2].phase<9?s2_P[2].phase:8]);
    s2_txt(10,7,buf,GLUT_BITMAP_HELVETICA_10);
}

/* ---------- scene-2 update ---------- */
static void s2_updatePerson(S2_Person&p,float dt){
    p.timer+=dt;
    switch(p.phase){
    case 0:
        p.wc+=dt*3.0f; p.x+=52.f*dt;
        if(p.x>=p.stopX-28.f){p.phase=1;p.timer=0;}
        break;
    case 1:{float rem=p.stopX-p.x;
        p.wc+=dt*1.5f;
        if(rem>0.8f){p.x+=rem*dt*4.f;}
        else{p.x=p.stopX;p.wc=0;p.phase=2;p.timer=0;}}
        break;
    case 2:
        p.headTilt+=(12.f-p.headTilt)*dt*2.f;
        p.armR    +=(14.f-p.armR)    *dt*1.5f;
        p.armL    +=(-5.f-p.armL)    *dt*1.5f;
        p.screenGlow+=(0.30f-p.screenGlow)*dt*2.f;
        if(p.timer>2.5f){p.phase=3;p.timer=0;}
        break;
    case 3:{float tgt=(p.timer<0.5f)?26.f:12.f;
        p.armR+=(tgt-p.armR)*dt*4.f;
        p.screenGlow+=(0.70f-p.screenGlow)*dt*5.f;
        if(p.timer>1.6f){p.screenGlow=0.25f;p.phase=4;p.timer=0;}}
        break;
    case 4:
        p.armR+=(36.f-p.armR)*dt*2.5f;
        if(p.timer>0.6f&&!p.billVis){
            p.billVis=true;
            p.billY=S2_MY+91.f;
            p.billX=p.x+18.f;
            p.billTargX=s2_MCX[p.machIdx]-S2_MW*0.5f+14.f;
        }
        if(p.billVis){
            float spd=(p.billTargX-p.billX);
            p.billX+=spd*dt*5.f;
            if(fabsf(p.billX-p.billTargX)<1.5f){
                p.billVis=false;p.phase=5;p.timer=0;p.procAngle=0;
            }
        }
        break;
    case 5:
        p.procAngle+=dt*3.5f;
        p.armR+=(5.f-p.armR)*dt*1.5f;
        p.armL+=(-6.f-p.armL)*dt*1.5f;
        p.screenGlow=0.15f+sinf(p.procAngle*0.4f)*0.07f;
        if(p.timer>3.2f){p.procAngle=0;p.screenGlow=0;p.phase=6;p.timer=0;}
        break;
    case 6:{float tgt=(p.timer<0.9f)?45.f:8.f;
        p.armR+=(tgt-p.armR)*dt*2.5f;
        if(p.timer>0.9f&&!p.hasTicket) p.hasTicket=true;
        p.headTilt+=(0.f-p.headTilt)*dt*2.f;
        if(p.timer>2.8f){p.phase=7;p.timer=0;}}
        break;
    case 7:
        p.wc+=dt*3.0f; p.x+=54.f*dt;
        p.headTilt+=(0.f-p.headTilt)*dt*2.f;
        p.screenGlow=0;
        if(p.x>S2_WW+110){p.phase=8;p.timer=0;}
        break;
    case 8:
        if(p.timer>1.5f) s2_initPerson(p.machIdx);
        break;
    }
}

static void s2_updateQueuers(float dt){
    for(int i=0;i<2;i++){
        S2_Queuer&q=s2_QP[i];
        q.walkWC+=dt*2.5f;
        q.x+=q.walkDir*18.f*dt;
        if(q.x>=q.maxX){q.x=q.maxX;q.walkDir=-1;}
        if(q.x<=q.minX){q.x=q.minX;q.walkDir= 1;}
    }
}

static void s2_step(float dt){
    s2_gTime+=dt;
    for(int i=0;i<S2_NP;i++) s2_updatePerson(s2_P[i],dt);
    s2_updateQueuers(dt);
}

/* ---------- scene-2 display ---------- */
static void s2_display(){
    glClearColor(0.89f,0.88f,0.86f,1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    s2_drawStation();

    for(int i=0;i<S2_NP;i++){
        bool act=(s2_P[i].phase>=2&&s2_P[i].phase<=6);
        s2_drawMachine(s2_MCX[i], act?s2_P[i].screenGlow:0.f,
                       act?s2_P[i].procAngle:0.f, act);
    }
    for(int i=0;i<S2_NP;i++)
        if(s2_P[i].phase==6&&s2_P[i].timer<0.85f)
            s2_drawTicket(s2_MCX[i],S2_MY+28,0,s2_P[i].dest,s2_P[i].fare);
    for(int i=0;i<S2_NP;i++)
        if(s2_P[i].billVis) s2_drawBill(s2_P[i].billX,s2_P[i].billY);

    s2_drawQueuer(&s2_QP[1]); s2_drawQueuer(&s2_QP[0]);

    for(int i=0;i<S2_NP;i++){
        bool walk=(s2_P[i].phase==0||s2_P[i].phase==1||s2_P[i].phase==7);
        s2_drawPerson(s2_P[i].x,s2_P[i].y,
                      s2_P[i].wc,s2_P[i].phase,
                      s2_P[i].armR,s2_P[i].armL,s2_P[i].headTilt,
                      s2_P[i].sr,s2_P[i].sg,s2_P[i].sb,s2_P[i].skR,
                      walk,s2_P[i].hasTicket,s2_P[i].dest,s2_P[i].fare);
        s2_drawBubble(s2_P[i]);
    }
    for(int i=0;i<S2_NP;i++){
        if(s2_P[i].phase==6&&s2_P[i].timer>2.f){
            float al=sinf((s2_P[i].timer-2.f)*2.5f)*0.5f+0.5f;
            s2_col4(0.04f,0.55f,0.22f,al);
            s2_txt(s2_P[i].x-14,s2_P[i].y+165,"Got ticket!",GLUT_BITMAP_HELVETICA_18);
        }
    }
    s2_drawHUD();
    glutSwapBuffers();
}

/* ---------- scene-2 projection ---------- */
static void s2_setProjection(int w,int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0,S2_WW,0,S2_WH);
    glMatrixMode(GL_MODELVIEW);
}


/* ============================================================
   ============================================================
   SCENE 3 – MRT-6 PLATFORM / TRAIN ARRIVAL
   (All identifiers prefixed  s3_  to avoid clashes)
   ============================================================
   ============================================================ */

/* ---------- scene-3 state ---------- */
static float s3_trainX     = -2.0f;
static int   s3_state      = 0;   /* 0=waiting, 1=arriving, 2=boarding, 3=departing */
static int   s3_timerCount = 0;
static bool  s3_doorOpen   = false;
static float s3_textOffset = 0.8f;
static bool  s3_glassOpen  = false;
static bool  s3_trainStopped = false;

struct S3Person {
    float px;           /* NDC x (feet)                  */
    float wc;           /* walk cycle                    */
    int   dir;          /* pacing direction               */
    bool  inTrain;      /* has boarded                   */
    bool  boarding;     /* currently walking to door     */
    float boardTargX;   /* door NDC x to reach           */
    /* appearance (set in init) */
    float sr,sg,sb,skR;
    const char*dest,*fare;
};
static S3Person s3_persons[4];

static float s3_doorNdcX(){
    return s3_trainX + 0.0f;  /* centre of the train door opening */
}

static void s3_initPersons(){
    /* Starting positions spread across platform, pacing */
    static float startX[4] = {-0.70f, -0.35f, 0.20f, 0.55f};
    static int   startDir[4]= {1, -1, 1, -1};
    static const float shirts[4][3]={
        {0.16f,0.26f,0.56f}, {0.52f,0.16f,0.16f},
        {0.14f,0.40f,0.22f}, {0.45f,0.30f,0.55f}
    };
    static const float skins[4]={0.72f,0.65f,0.76f,0.68f};
    static const char*dests[4]={"Motijheel","Farmgate","Pallabi","Agargaon"};
    static const char*fares[4]={"40 BDT","30 BDT","15 BDT","20 BDT"};
    for(int i=0;i<4;i++){
        s3_persons[i].px         = startX[i];
        s3_persons[i].wc         = i*0.8f;
        s3_persons[i].dir        = startDir[i];
        s3_persons[i].inTrain    = false;
        s3_persons[i].boarding   = false;
        s3_persons[i].boardTargX = 0.f;
        s3_persons[i].sr = shirts[i][0];
        s3_persons[i].sg = shirts[i][1];
        s3_persons[i].sb = shirts[i][2];
        s3_persons[i].skR= skins[i];
        s3_persons[i].dest = dests[i];
        s3_persons[i].fare = fares[i];
    }
}

/* ---------- scene-3 helpers ---------- */
static void s3_drawRect(float x, float y, float w, float h){
    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}
static void s3_drawCircle(float cx, float cy, float r, int seg=60){
    glBegin(GL_POLYGON);
    for(int i=0;i<seg;i++){
        float a=i*3.14159f/180.0f*(360.0f/seg);
        glVertex2f(cx+r*cosf(a), cy+r*sinf(a));
    }
    glEnd();
}

/* ---------- scene-3 drawing functions ---------- */
static void s3_drawSky(){
    glBegin(GL_QUADS);
    glColor3f(0.55f,0.80f,1.0f); glVertex2f(-1,1); glVertex2f(1,1);
    glColor3f(0.75f,0.90f,1.0f); glVertex2f(1,0.18f); glVertex2f(-1,0.18f);
    glEnd();
}

static void s3_drawCloud(float cx, float cy){
    glColor3f(1,1,1);
    for(int k=0;k<5;k++){
        float offset=(k-2)*0.06f;
        glBegin(GL_POLYGON);
        for(int i=0;i<360;i++){
            float a=i*3.14159f/180.0f;
            glVertex2f(cx+offset+0.07f*cosf(a), cy+0.04f*sinf(a));
        }
        glEnd();
    }
}

static void s3_drawPillars(){
    float positions[]={-0.7f,0.0f,0.7f};
    for(int i=0;i<3;i++){
        float x=positions[i];
        glColor3f(0.3f,0.3f,0.35f);
        glBegin(GL_QUADS);
        glVertex2f(x,0.18f); glVertex2f(x+0.05f,0.18f);
        glVertex2f(x+0.05f,0.7f); glVertex2f(x,0.7f);
        glEnd();
    }
}

static void s3_drawShed(){
    glColor3f(0.15f,0.15f,0.18f);
    glBegin(GL_QUADS);
    glVertex2f(-1,0.7f); glVertex2f(1,0.7f);
    glVertex2f(1,0.85f); glVertex2f(-1,0.85f);
    glEnd();
    glColor3f(0.45f,0.45f,0.48f);
    glBegin(GL_QUADS);
    glVertex2f(-1,0.68f); glVertex2f(1,0.68f);
    glVertex2f(1,0.72f);  glVertex2f(-1,0.72f);
    glEnd();
    glEnable(GL_BLEND);
    glColor4f(0.0f,0.0f,0.0f,0.25f);
    glBegin(GL_QUADS);
    glVertex2f(-1,0.65f); glVertex2f(1,0.65f);
    glVertex2f(1,0.68f);  glVertex2f(-1,0.68f);
    glEnd();
}

static void s3_drawClock(){
    float cx=-0.7f, cy=0.5f;
    time_t now=time(0); tm *ltm=localtime(&now);
    int hour=ltm->tm_hour%12, minute=ltm->tm_min;
    float second=(float)ltm->tm_sec+(s3_timerCount%60)/60.0f;
    float hourAngle=-(hour+minute/60.0f)*30.0f;
    float minAngle =-(minute+second/60.0f)*6.0f;
    float secAngle =-second*6.0f;
    glColor3f(0,0,0);
    glBegin(GL_POLYGON);
    for(int i=0;i<360;i++){float a=i*3.14159f/180.f;
        glVertex2f(cx+0.1f*cosf(a),cy+0.1f*sinf(a));}
    glEnd();
    glColor3f(1,1,1);
    for(int i=0;i<12;i++){
        float angle=i*30*3.14159f/180.f;
        glBegin(GL_LINES);
        glVertex2f(cx+0.08f*cosf(angle),cy+0.08f*sinf(angle));
        glVertex2f(cx+0.10f*cosf(angle),cy+0.10f*sinf(angle));
        glEnd();
    }
    glColor3f(1.0f,0.84f,0.0f); s3_drawCircle(cx,cy,0.005f,20);
    glPushMatrix(); glTranslatef(cx,cy,0); glRotatef(hourAngle,0,0,1);
    glLineWidth(3); glBegin(GL_LINES); glVertex2f(0,0); glVertex2f(0.04f,0); glEnd();
    glPopMatrix();
    glPushMatrix(); glTranslatef(cx,cy,0); glRotatef(minAngle,0,0,1);
    glLineWidth(2); glBegin(GL_LINES); glVertex2f(0,0); glVertex2f(0.07f,0); glEnd();
    glPopMatrix();
    glPushMatrix(); glTranslatef(cx,cy,0); glRotatef(secAngle,0,0,1);
    glColor3f(1.0f,0.84f,0.0f); glLineWidth(1);
    glBegin(GL_LINES); glVertex2f(0,0); glVertex2f(0.09f,0); glEnd();
    glPopMatrix();
    glLineWidth(1);
}

static void s3_drawRailing(){
    for(float y=0.0f;y<0.18f;y+=0.04f){
        for(float x=-1.0f;x<1.0f;x+=0.16f){
            float offset=((int)(y*100)%2)?0.08f:0.0f;
            glColor3f(0.55f,0.27f,0.20f);
            glBegin(GL_QUADS);
            glVertex2f(x+offset,y); glVertex2f(x+offset+0.15f,y);
            glVertex2f(x+offset+0.15f,y+0.035f); glVertex2f(x+offset,y+0.035f);
            glEnd();
            glColor3f(0.35f,0.15f,0.10f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x+offset,y); glVertex2f(x+offset+0.15f,y);
            glVertex2f(x+offset+0.15f,y+0.035f); glVertex2f(x+offset,y+0.035f);
            glEnd();
        }
    }
    glColor3f(0.12f,0.12f,0.12f);
    glBegin(GL_QUADS);
    glVertex2f(-1,0.18f); glVertex2f(1,0.18f); glVertex2f(1,0.22f); glVertex2f(-1,0.22f);
    glEnd();
    glColor3f(1.0f,0.84f,0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1,0.175f); glVertex2f(1,0.175f); glVertex2f(1,0.18f); glVertex2f(-1,0.18f);
    glEnd();
}

static void s3_drawPlatform(){
    glColor3f(0.25f,0.25f,0.25f);
    glBegin(GL_QUADS);
    glVertex2f(-1,-0.3f); glVertex2f(1,-0.3f);
    glVertex2f(1,-0.6f);  glVertex2f(-1,-0.6f);
    glEnd();
    glColor3f(0.35f,0.35f,0.38f);
    glBegin(GL_QUADS);
    glVertex2f(-1,-0.31f); glVertex2f(1,-0.31f);
    glVertex2f(1,-0.33f);  glVertex2f(-1,-0.33f);
    glEnd();
    glColor3f(1.0f,0.84f,0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1,-0.4f); glVertex2f(1,-0.4f);
    glVertex2f(1,-0.42f); glVertex2f(-1,-0.42f);
    glEnd();
}

static void s3_drawGlass(){
    for(float x=-1.0f;x<=1.0f;x+=0.25f){
        float mid=x+0.125f;
        glColor3f(0.2f,0.2f,0.2f);
        s3_drawRect(x,-0.3f,0.01f,0.2f);
        s3_drawRect(x+0.24f,-0.3f,0.01f,0.2f);
        float leftX =s3_glassOpen?-0.08f:0.0f;
        float rightX=s3_glassOpen? 0.08f:0.0f;
        glEnable(GL_BLEND);
        glColor4f(0.65f,0.85f,0.95f,0.35f);
        glBegin(GL_QUADS);
        glVertex2f(x+0.01f+leftX,-0.3f); glVertex2f(mid+leftX,-0.3f);
        glVertex2f(mid+leftX,-0.1f);     glVertex2f(x+0.01f+leftX,-0.1f);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2f(mid+rightX,-0.3f);     glVertex2f(x+0.24f+rightX,-0.3f);
        glVertex2f(x+0.24f+rightX,-0.1f); glVertex2f(mid+rightX,-0.1f);
        glEnd();
        glColor3f(0.1f,0.1f,0.1f);
        if(!s3_glassOpen){
            glBegin(GL_LINES);
            glVertex2f(mid,-0.3f); glVertex2f(mid,-0.1f);
            glEnd();
        }
    }
}

/* ---------- scene-3 people walk cycles (for s2_drawPerson animation) ---------- */

/* Helper: draw a Scene-2-quality person inside Scene 3's NDC coordinate system. */
static void s3_drawPersonS2Style(float ndc_x, float wc, int ph,
                                  float armR_, float armL_, float ht,
                                  float sr, float sg_, float sb, float skR,
                                  bool walking, bool hasTicket,
                                  const char*dest="MRT", const char*fare="BDT")
{
    glPushMatrix();
    glScalef(2.0f/1100.f, 2.0f/680.f, 1.0f);
    glTranslatef(-550.f, -340.f, 0.f);
    float px_s2 = (ndc_x + 1.0f) * 550.f;
    /* Platform surface is at NDC y = -0.30.
       py_s2 = (-0.30 + 1.0) * 340 = 238  → person feet on platform top */
    float py_s2 = 238.f;
    s2_drawPerson(px_s2, py_s2, wc, ph, armR_, armL_, ht,
                  sr, sg_, sb, skR, walking, hasTicket, dest, fare);
    glPopMatrix();
}

static void s3_drawPeople(){
    for(int k=0;k<4;k++){
        S3Person&p = s3_persons[k];
        if(p.inTrain) continue;   /* already boarded – hidden inside train */

        bool walking = (s3_state==0 && !p.boarding)
                    || (s3_state==2 &&  p.boarding);
        int  ph      = walking ? 0 : 5;   /* 0=walk, 5=idle */
        float armR=0,armL=0,ht=0;

        /* While waiting for train: head tilts slightly toward arriving train */
        if(s3_state==1) ht = 8.f * p.dir;

        s3_drawPersonS2Style(p.px, p.wc, ph, armR, armL, ht,
                             p.sr, p.sg, p.sb, p.skR,
                             walking, false, p.dest, p.fare);
    }
}

static void s3_drawTrack(){
    glColor3f(0.3f,0.3f,0.32f);
    for(float i=-1;i<=1;i+=0.12f){
        glBegin(GL_QUADS);
        glVertex2f(i,-0.75f); glVertex2f(i+0.08f,-0.75f);
        glVertex2f(i+0.08f,-0.65f); glVertex2f(i,-0.65f);
        glEnd();
    }
    glColor3f(0.2f,0.2f,0.2f);
    glBegin(GL_QUADS);
    glVertex2f(-1,-0.665f); glVertex2f(1,-0.665f);
    glVertex2f(1,-0.675f);  glVertex2f(-1,-0.675f);
    glVertex2f(-1,-0.725f); glVertex2f(1,-0.725f);
    glVertex2f(1,-0.735f);  glVertex2f(-1,-0.735f);
    glEnd();
}

static void s3_drawTrain(float x){
    float y=-0.48f;
    glPushMatrix();
    glTranslatef(x,y,0);
    glColor3f(0.0f,0.75f,1.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.8f,0.0f); glVertex2f(0.8f,0.0f);
    glVertex2f(0.8f,0.3f);  glVertex2f(-0.8f,0.3f);
    glEnd();
    glColor3f(0.25f,0.25f,0.28f);
    s3_drawRect(-0.68f,0.12f,0.22f,0.16f);
    s3_drawRect(-0.38f,0.12f,0.22f,0.16f);
    s3_drawRect( 0.16f,0.12f,0.22f,0.16f);
    s3_drawRect( 0.46f,0.12f,0.22f,0.16f);
    glColor3f(1.0f,0.85f,0.0f);
    s3_drawRect(-0.68f,0.08f,0.22f,0.02f);
    s3_drawRect(-0.38f,0.08f,0.22f,0.02f);
    s3_drawRect( 0.16f,0.08f,0.22f,0.02f);
    s3_drawRect( 0.46f,0.08f,0.22f,0.02f);
    float offset=s3_doorOpen?0.10f:0.0f;
    glColor3f(0.25f,0.27f,0.30f);
    s3_drawRect(-0.15f-offset,0.02f,0.12f,0.30f);
    s3_drawRect( 0.03f+offset,0.02f,0.12f,0.30f);
    glColor3f(0.5f,0.5f,0.55f);
    s3_drawRect(-0.14f-offset,0.02f,0.01f,0.30f);
    s3_drawRect( 0.14f+offset,0.02f,0.01f,0.30f);
    glColor3f(1.0f,0.85f,0.2f);
    s3_drawCircle(0.7f,0.1f,0.06f,30);
    s3_drawCircle(-0.7f,0.1f,0.06f,30);
    glColor3f(0.1f,0.1f,0.1f);
    for(float i=-0.5f;i<=0.5f;i+=0.5f){
        glBegin(GL_POLYGON);
        for(int j=0;j<360;j++){float a=j*3.14159f/180.f;
            glVertex2f(i+0.08f*cosf(a),-0.08f+0.08f*sinf(a));}
        glEnd();
    }
    glPopMatrix();
}

static void s3_drawBench(){
    float y=-0.12f;
    glColor3f(0.45f,0.25f,0.1f);
    s3_drawRect(-0.5f,y,0.5f,0.05f);
    glColor3f(0.4f,0.22f,0.1f);
    s3_drawRect(-0.5f,y+0.05f,0.5f,0.08f);
    glColor3f(0.2f,0.2f,0.2f);
    s3_drawRect(-0.48f,y-0.08f,0.03f,0.08f);
    s3_drawRect(-0.05f,y-0.08f,0.03f,0.08f);
}

static void s3_drawBillboard(){
    glColor3f(0.1f,0.1f,0.1f);
    s3_drawRect(0.3f,0.72f,0.6f,0.12f);
    glColor3f(0.0f,0.0f,0.0f);
    s3_drawRect(0.32f,0.74f,0.56f,0.08f);
    glColor3f(0.0f,1.0f,0.0f);
    glRasterPos2f(s3_textOffset,0.77f);
    const char* msg="Next Train Will Come After 2 Min   ";
    for(int i=0;msg[i]!='\0';i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,msg[i]);
}

static void s3_drawLeftBillboard(){
    glColor3f(0.1f,0.1f,0.1f);
    s3_drawRect(-0.95f,0.72f,0.5f,0.12f);
    glColor3f(0.0f,0.0f,0.0f);
    s3_drawRect(-0.93f,0.74f,0.46f,0.08f);
    glColor3f(0.0f,1.0f,0.0f);
    glRasterPos2f(-0.90f,0.79f);
    const char* line1="Current Station";
    for(int i=0;line1[i]!='\0';i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,line1[i]);
    glRasterPos2f(-0.80f,0.73f);
    const char* line2="UTTARA NORTH";
    for(int i=0;line2[i]!='\0';i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,line2[i]);
}

static void s3_drawHint(){
    glColor4f(0.0f,0.0f,0.0f,0.55f);
    glBegin(GL_QUADS);
    glVertex2f(-1,-1); glVertex2f(1,-1);
    glVertex2f(1,-0.88f); glVertex2f(-1,-0.88f);
    glEnd();
    glColor3f(1.0f,1.0f,0.6f);
    glRasterPos2f(-0.45f,-0.96f);
    const char* hint="SCENE 3: Platform  |  Press SPACE or N for Scene 1";
    for(int i=0;hint[i]!='\0';i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,hint[i]);
}

/* ---------- scene-3 display ---------- */
static void s3_display(){
    glClearColor(0.9f,0.9f,0.9f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    s3_drawSky();
    s3_drawCloud(-0.6f,0.7f);
    s3_drawCloud(0.3f,0.75f);
    s3_drawShed();
    s3_drawBillboard();
    s3_drawLeftBillboard();
    s3_drawPillars();
    s3_drawClock();
    s3_drawRailing();
    s3_drawPlatform();
    s3_drawBench();
    s3_drawTrack();
    s3_drawPeople();
    s3_drawTrain(s3_trainX);
    s3_drawGlass();
    s3_drawHint();

    glutSwapBuffers();
}

/* ---------- scene-3 update ---------- */
static void s3_step(){
    s3_timerCount++;

    /* ── Train state machine ─────────────────────────────────────────── */
    if(s3_state==0){
        /* Waiting – train off screen to the left, countdown then depart */
        if(s3_timerCount>600){ s3_state=1; s3_timerCount=0; }

    } else if(s3_state==1){
        /* Train arriving from left */
        s3_trainX += 0.012f;
        if(s3_trainX > -0.2f){
            s3_trainX = -0.2f;   /* stop exactly */
            s3_trainStopped = true;
            s3_doorOpen  = true;
            s3_glassOpen = true;
            s3_state=2; s3_timerCount=0;
            /* Tell each person to start boarding toward the door */
            float doorX = s3_doorNdcX();
            /* Spread 4 people across 2 door openings with slight offsets */
            float targets[4] = { doorX-0.08f, doorX-0.03f,
                                  doorX+0.03f, doorX+0.08f };
            for(int i=0;i<4;i++){
                s3_persons[i].boarding   = true;
                s3_persons[i].boardTargX = targets[i];
                /* Set walk direction toward target */
                s3_persons[i].dir = (targets[i] > s3_persons[i].px) ? 1 : -1;
            }
        }

    } else if(s3_state==2){
        /* Doors open – people walk to door and board */
        bool allBoarded = true;
        for(int i=0;i<4;i++){
            S3Person&p = s3_persons[i];
            if(p.inTrain) continue;
            allBoarded = false;
            if(p.boarding){
                float step = 0.006f;
                if(p.px < p.boardTargX - step)      p.px += step;
                else if(p.px > p.boardTargX + step)  p.px -= step;
                else {
                    /* Reached door — disappear into train */
                    p.inTrain  = true;
                    p.boarding = false;
                }
                p.wc += 0.09f;  /* walking animation */
            }
        }
        /* Once everyone has boarded wait a moment then depart */
        if(allBoarded && s3_timerCount > 80){
            s3_state=3; s3_timerCount=0;
            s3_doorOpen  = false;
            s3_glassOpen = false;
            s3_trainStopped = false;
        }

    } else if(s3_state==3){
        /* Train departing to the right */
        s3_trainX += 0.012f;
        if(s3_trainX > 2.0f){
            /* Reset everything for next cycle */
            s3_trainX = -2.0f;
            s3_state  = 0;
            s3_timerCount = 0;
            s3_initPersons();  // fresh set of people appear on platform
        }
    }

    //  People pacing (only while waiting, state 0)
    if(s3_state==0){
        for(int i=0;i<4;i++){
            S3Person&p = s3_persons[i];
            p.px += 0.003f * p.dir;
            /* bounce within platform bounds */
            if(p.px >  0.75f){ p.px =  0.75f; p.dir = -1; }
            if(p.px < -0.80f){ p.px = -0.80f; p.dir =  1; }
            p.wc += 0.09f;
        }
    }

    // Billboard ticker
    s3_textOffset -= 0.005f;
    if(s3_textOffset < 0.3f) s3_textOffset = 0.85f;
}

//scene-3 projection
static void s3_setProjection(int w, int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(-1.0,1.0,-1.0,1.0);
    glMatrixMode(GL_MODELVIEW);
}


static void display_cb(){
    if(currentScene==1)      s1_display();
    else if(currentScene==2) s2_display();
    else                     s3_display();
}

static void reshape_cb(int w,int h){
    if(currentScene==1)      s1_setProjection(w,h);
    else if(currentScene==2) s2_setProjection(w,h);
    else                     s3_setProjection(w,h);
}

static void timer_cb(int){
    static float lastT=0.0f;
    float now=glutGet(GLUT_ELAPSED_TIME)/1000.f;
    float dt=now-lastT; if(dt>0.05f)dt=0.05f;
    lastT=now;

    if(currentScene==1)      s1_step();
    else if(currentScene==2) s2_step(dt);
    else                     s3_step();

    glutPostRedisplay();
    glutTimerFunc(30,timer_cb,0);
}

static void key_cb(unsigned char k,int,int){
    if(k==27) exit(0);   /* ESC = quit */

    if(k==' '||k=='n'||k=='N'){
        /* cycle through scenes: 1 → 2 → 3 → 1 */
        currentScene=(currentScene%3)+1;
        /* re-apply correct projection for new scene */
        int w=glutGet(GLUT_WINDOW_WIDTH);
        int h=glutGet(GLUT_WINDOW_HEIGHT);
        if(currentScene==1)      s1_setProjection(w,h);
        else if(currentScene==2) s2_setProjection(w,h);
        else                     s3_setProjection(w,h);
        glutPostRedisplay();
        return;
    }
    if(currentScene==2){
        if(k=='r'||k=='R'){
            for(int i=0;i<S2_NP;i++) s2_initPerson(i);
            s2_initQueuers();
        }
    }
}

/* ============================================================
   MAIN
   ============================================================ */
int main(int argc,char**argv){
    /* --- scene-2 machine centres --- */
    s2_MCX[0]=(float)S2_WW/2 - S2_MGAP;
    s2_MCX[1]=(float)S2_WW/2;
    s2_MCX[2]=(float)S2_WW/2 + S2_MGAP;

    /* --- scene-1 init --- */
    s1_generateWindowColors(256);
    s1_initClouds();
    s1_initPeople();
    s1_initLamps();

    /* --- scene-2 init --- */
    for(int i=0;i<S2_NP;i++) s2_initPerson(i);
    s2_initQueuers();

    /* --- scene-3 init --- */
    s3_initPersons();

    /* --- GLUT setup --- */
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(WIN_W,WIN_H);
    glutCreateWindow("Dhaka Metro  |  SPACE / N = Switch Scene  |  ESC = Exit");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    glLineWidth(1.2f);

    /* set initial projection for scene 1 */
    s1_setProjection(WIN_W,WIN_H);

    glutDisplayFunc(display_cb);
    glutReshapeFunc(reshape_cb);
    glutKeyboardFunc(key_cb);
    glutTimerFunc(30,timer_cb,0);
    glutMainLoop();
    return 0;
}
