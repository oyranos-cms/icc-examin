#include "vFLGLWidget.h"
#include "ViewerFLTK.h"


vFLGLWidget::vFLGLWidget( int x,int y,int w,int h,const char *l )
     : Fl_Gl_Window(x,y,w,h,l), 
       viewer(0)
{
LeftViewer = 1;
}

void vFLGLWidget::setViewerPtr( ViewerFLTK *v_ptr)
{
    viewer = v_ptr;
}

int vFLGLWidget::handle(int e)
{
	switch (e) {
	case FL_ENTER:
		viewer->Hdraw = 1;
                LeftViewer = 0;
                if (viewer) 
                    viewer->handleRedraw();
		if (viewer->Hok==0) viewer->Hdraw = 0;
		return (1);

	case FL_LEAVE:
		viewer->Hdraw = 0;
                LeftViewer = 1;
		return (1);

	case FL_PUSH:
		viewer->Hdraw = 0;
                viewer->vMousePressEvent( Fl::event_button(), Fl::event_x(), Fl::event_y());
		return (1);

	case FL_DRAG:
		viewer->Hdraw = 0;
                viewer->vMouseMoveEvent( 1, Fl::event_button(), Fl::event_x(), Fl::event_y());
		return (1);

	case FL_MOVE:
		viewer->Hdraw = 0;
                viewer->vMouseMoveEvent( 0, Fl::event_button(), Fl::event_x(), Fl::event_y());
		return (1);

	case FL_RELEASE:
		viewer->Hdraw = 1;
                if ( (viewer) && (viewer->Hdraw==1) && (viewer->Hok==0) )
                    viewer->handleRedraw();
                viewer->vMouseReleaseEvent( Fl::event_button(), Fl::event_x(), Fl::event_y());
		return (1);

	case FL_SHORTCUT:
		viewer->Hdraw = 1;
                viewer->vKeyH();
		return (1);
	}

	return (0);
}


