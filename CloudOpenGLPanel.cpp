#include "CloudOpenGLPanel.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include <QPainter>

#include "DebugUtil.h"
#include <limits>
#include <stdint.h>
#include <QPixmap>

CloudOpenGLPanel::CloudOpenGLPanel(QWidget *parent) :
    QGLWidget(parent), painter()
{
    setFormat(QGLFormat(QGL::SampleBuffers));
    setFocusPolicy(Qt::StrongFocus);
    calculateDepthLookup(-0.000005,0);

    mode = PAINT_DEPTH;
    mx=my=-1;
    rotangles[0]=0;
    rotangles[1]=0;
    zoom=1;

    //fill in index vector
    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 320; x++)
        {
            uint16_t k = y*320+x;
            indices[y][x] = k;
        }
    }
}

//TODO As you get closer the values change faster.
void CloudOpenGLPanel::calculateDepthLookup(double mult, double add)
{
    for(uint16_t i=0; i<2048; i++)
    {
        depthLookup[i]= 1/(i*mult+add);
    }
}

void CloudOpenGLPanel::initializeGL()
{
    dprintf("Initialising GL: %d\n",this->paintingActive());
    glShadeModel(GL_SMOOTH);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClearDepth(1.0f);

    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void CloudOpenGLPanel::resizeGL(int w, int h)
{
    dprintf("Resizing GL: %d\n",this->paintingActive());
    GLdouble size;
    GLdouble aspect;


    //Use the whole window.
    glViewport(0, 0, w, h);

    //We are going to do some 2-D orthographic drawing.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    size = (GLdouble) ((w >= h) ? w : h) / 2.0;

    if (w <= h)
    {
        aspect = (GLdouble) h / (GLdouble) w;
        glOrtho(-size, size, -size * aspect, size * aspect, -100000.0,
                100000.0);
    }
    else
    {
        aspect = (GLdouble) w / (GLdouble) h;
        glOrtho(-size * aspect, size * aspect, -size, size, -100000.0,
                100000.0);
    }

    //Make the world and window coordinates coincide so that 1.0 in
    //model space equals one pixel in window space.
    glScaled(aspect, aspect, 1.0);

}


void drawBlock()
{
    //Draw Stationary Block
    glBegin(GL_POLYGON);
    glColor3f(1,0,0);
    glVertex3f(-50,-50,50);
    glVertex3f(-50, 50,50);
    glVertex3f( 50, 50,50);
    glVertex3f( 50,-50,50);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1,0,1);
    glVertex3f(-50,-50,-50);
    glVertex3f(-50, 50,-50);
    glVertex3f( 50, 50,-50);
    glVertex3f( 50,-50,-50);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1,1,0);
    glVertex3f(-50,50,-50);
    glVertex3f(-50,50, 50);
    glVertex3f( 50,50, 50);
    glVertex3f( 50,50,-50);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0,1,1);
    glVertex3f(-50,-50,-50);
    glVertex3f(-50,-50, 50);
    glVertex3f( 50,-50, 50);
    glVertex3f( 50,-50,-50);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1,1,1);
    glVertex3f( 50,-50,-50);
    glVertex3f( 50,-50, 50);
    glVertex3f( 50,50, 50);
    glVertex3f( 50,50,-50);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0,.7,.5);
    glVertex3f(-50,-50,-50);
    glVertex3f(-50,-50, 50);
    glVertex3f(-50, 50, 50);
    glVertex3f(-50, 50,-50);
    glEnd();

    glColor3f(1,1,1);
}


void CloudOpenGLPanel::paintCloud()
{
    float xyz[240][320][3];

    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 320; x++)
        {
            uint16_t k = y*320+x;
            xyz[y][x][0] = x;
            xyz[y][x][1] = y;
            xyz[y][x][2] = depthLookup[depth[k]];//>=100? depthLookup[depth[k]]: std::numeric_limits<float>::quiet_NaN();
        }
    }

    makeCurrent();                                      //Set this as current GL context

    glPushAttrib(GL_ALL_ATTRIB_BITS);                   //push Attributes
    glPushMatrix();                                     //Push the Old Matrix


    glMatrixMode(GL_MODELVIEW);                         //Model View Mode
    glLoadIdentity();                                   //Load the identity

    //Initialize Viewport
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );              //Clear Color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear Color / Depth
    glColor3f(1, 1, 1);                                 //Point Color White
    glEnable(GL_DEPTH_TEST);                            //Enable Depth Test


    //Rotate about center of pointcloud
    glScalef(zoom,zoom,1);                      //Zoom
    glRotatef(rotangles[0], 1, 0, 0);           //Rotate X
    glRotatef(rotangles[1], 0, 1, 0);           //Rotate Y
    glRotatef(180,1,0,0);
    //drawBlock();

    //Center
    glTranslatef(-160, -120,0);                 //Center the Image
    glPointSize(.1);                            //Set Point Size =.1

    //Enable the Vertex Array
    glEnableClientState(GL_VERTEX_ARRAY);       //Allows use of GL_VERTEX_ARRAY
    glVertexPointer(3, GL_FLOAT, 0, xyz);       //Load the Vertex Array
    glDrawElements(GL_POINTS,320*240,GL_UNSIGNED_INT,indices);  //Draw the points
    glDisableClientState(GL_VERTEX_ARRAY);      //Disable Vertex Array
    glPopMatrix();                              //Pop the Matrix
    glPopAttrib();                              //Pop the attributes
    swapBuffers();                              //Swap te buffers
}

void CloudOpenGLPanel::paintDisparity()
{
    if(painter.begin(this))
    {
        QRect r = painter.window();
        painter.fillRect(r,Qt::black);
        static uint8_t buf[320*240*3];
        for(unsigned i=0;i<320*240;i++)
        {
            uint8_t tmp = depth[i]*0xFF/(float)0x7FF;
            buf[i*3]  =tmp;
            buf[i*3+1]=tmp;
            buf[i*3+2]=tmp;
        }

        QImage img((uchar*) buf,320,240,QImage::Format_RGB888);
        img=img.scaledToHeight(480,Qt::FastTransformation);
        painter.translate(r.width()/2.0,r.height()/2.0);
        painter.drawImage(-320,-240,img);
        painter.end();
    }
    else
    {
        dprintf("Disparity Painter didn't begin")
    }
}

void CloudOpenGLPanel::paintIR()
{
    if(painter.begin(this))
    {
        QRect r = painter.window();
        painter.fillRect(r,Qt::black);
        static uint8_t buf[320*240*4];
        for(unsigned i=0;i<320*240;i++)
        {
            uint8_t tmp = image[i];
            buf[i*4]  =tmp;
            buf[i*4+1]=tmp;
            buf[i*4+2]=tmp;
            buf[i*4+3]=tmp;
        }
        QImage img((uchar*) buf,320,240,QImage::Format_ARGB32);
        painter.translate(r.width()/2.0,r.height()/2.0);
        painter.drawImage(-320,-240,img.scaledToHeight(480,Qt::FastTransformation));
        painter.end();
    }
    else
    {
        dprintf("IR Painter didn't begin")
    }
}

void CloudOpenGLPanel::paintInvalid()
{
    if(painter.begin(this))
    {
        QRect r = painter.window();
        painter.fillRect(r,Qt::black);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
        painter.translate(r.width()/2.0,r.height()/2.0);
        painter.rotate(-45);

        painter.setPen(QPen(Qt::red));
        painter.setFont(QFont("Helvetica",50));

        painter.drawText(-219.5,-46,439,92, Qt::AlignLeft, QStringLiteral("Invalid View"));
        painter.end();
    }
    else
    {
        dprintf("Invalid Painter didn't begin\n");
    }

}



//TODO Initializes to UpsideDown.
void CloudOpenGLPanel::paintEvent(QPaintEvent * event)
{
    switch(mode)
    {
    case PAINT_DEPTH:
        paintCloud();
        break;
    case PAINT_DISPARITY:
        paintDisparity();
        break;
    case PAINT_IR:
        paintIR();
        break;
    default:
        paintInvalid();
        break;
    }
}



void CloudOpenGLPanel::mouseMoveEvent(QMouseEvent *event)
{
    if(mode == PAINT_DEPTH || mode == PAINT_OVERLAY)
    {
        int y = event->y();
        int x=event->x();
        if(mx>0 &&my>0)
        {
            rotangles[0]+=y-my;
            rotangles[1]+=x-mx;
            update();
        }
        my=y;
        mx=x;
    }
}


void CloudOpenGLPanel::mousePressEvent(QMouseEvent * event)
{
    if (event->button()==Qt::LeftButton)
    {
        mx = event->x();
        my = event->y();
    }
}

void CloudOpenGLPanel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button()==Qt::LeftButton)
    {
        mx = -1;
        my = -1;
    }
}


void CloudOpenGLPanel::keyPressEvent(QKeyEvent *event)
{
    dprintf("Key Pressed: %d\n",event->key());
    if (event->key()==Qt::Key_Down) //Zoom in
    {
        zoom*=1.1;
        update();
    }

    else if (event->key() == Qt::Key_Up)
    {//Zoom out
        zoom/=1.1;
        update();

    }

    else if (event->key() == Qt::Key_Space) //Home in the Kinect
    {
        rotangles[0]=0;
        rotangles[1]=1;
        update();
    }
}


void CloudOpenGLPanel::setMode(PAINTMODE m)
{
    mode=m;
    update();
}
