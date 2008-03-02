#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

class ViewerFLTK;

class vFLGLWidget : public Fl_Gl_Window
{

public:
    vFLGLWidget( int x,int y,int w,int h,const char *l=0 );
    void setViewerPtr( ViewerFLTK *);
    unsigned char LeftViewer;    
    bool draw_widget;

private:
    int handle(int);
    ViewerFLTK  *viewer;
};


