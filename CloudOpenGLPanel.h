#ifndef CLOUDOPENGLPANEL_H
#define CLOUDOPENGLPANEL_H

#include <QGLWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>


class CloudOpenGLPanel : public QGLWidget
{
    Q_OBJECT
public:

    enum PAINTMODE
    {
        PAINT_DEPTH,
        PAINT_DISPARITY,
        PAINT_IR,
        PAINT_RGB,
        PAINT_OVERLAY,
        PAINT_INVALID
    };

    explicit CloudOpenGLPanel(QWidget *parent = 0);
    void giveDepthData(uint16_t* data){depth=data;}
    void giveImageData(uint16_t* data){image=data;}
    void calculateDepthLookup(double mult,double add);
    void setMode(PAINTMODE m);

protected:
    void initializeGL();
    void resizeGL(int x, int h);
    void paintEvent(QPaintEvent *);

    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    void calculateDepthLookup();
    void paintCloud();
    void paintDisparity();
    void paintIR();
    void paintInvalid();

    QPainter painter;
    unsigned int indices[240][320];
    uint16_t* depth;
    uint16_t* image;

    //uint16_t* image;
    float depthLookup[2048];

    PAINTMODE mode;
    float zoom;
    int rotangles[2];
    int mx,my;
};

#endif // CLOUDOPENGLPANEL_H
