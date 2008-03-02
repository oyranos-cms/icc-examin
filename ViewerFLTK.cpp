#include "ViewerFLTK.h"
#include "vFLGLWidget.h"

#define DEBUG
#ifdef DEBUG
#define DBG cout << __FILE__<<":"<<__LINE__ << __func__ << "()" << endl;
#else
#define DBG
#endif

ViewerFLTK::ViewerFLTK(openvrml::browser & browser,
		     vFLGLWidget *da_widg) :
  openvrml::gl::viewer(browser),
  d_width(0),
  d_height(0),
  da_widget(da_widg)
{DBG
}


ViewerFLTK::~ViewerFLTK()
{DBG
}


void ViewerFLTK::handleInput( viewer::event_info *e )
{DBG
    da_widget->make_current();
    input( e );
}

void ViewerFLTK::handleRedraw()
{DBG
    if ( (( (da_widget->valid()) && (Hok==1) )
       || ( (da_widget->valid()) && (Hdraw==1) && (Hok==0) ))
      && da_widget )
    {DBG
        int width, height;
        width = da_widget->w();
        height = da_widget->h(); DBG
        #ifdef DEBUG
        cout << da_widget << " " << width << "x" << height << " "; DBG
        #endif

        da_widget->make_current(); DBG
        if (width != d_width || height != d_height)
        {DBG
            resize( width, height );
            d_height = height;
            d_width = width;
        } DBG
        redraw();
    }
}

void ViewerFLTK::vMousePressEvent( int MouseBttn, int MouseX, int MouseY )
{DBG
    viewer::event_info e;

    e.event = viewer::event_mouse_click;
    switch (MouseBttn)
    {
            case 1  : e.what = 0; break; 
            case 2  : e.what = 1; break; 
            case 3  : e.what = 2; break;
            case 0  : return;
            default: break;  

    }
    e.x = MouseX;
    e.y = MouseY;
    handleInput( &e );
}
 
void ViewerFLTK::vMouseReleaseEvent( int MouseBttn, int MouseX, int MouseY )
{DBG
    viewer::event_info e;

    e.event = viewer::event_mouse_release;
    switch (MouseBttn)
    {
            case 1  : e.what = 0; break; 
            case 2  : e.what = 1; break; 
            case 3  : e.what = 2; break;
            case 0  : return;
            default: break; 

    }
    e.x = MouseX;
    e.y = MouseY;
    handleInput( &e );
}

void ViewerFLTK::vMouseMoveEvent( int MouseState, int MouseBttn, int MouseX, int MouseY )
{DBG
    // may be a move or a drag
    viewer::event_info e;
    e.what = 0;
    if (MouseState == 1) {
    e.event = viewer::event_mouse_drag;
    switch (MouseBttn)
    {
            case 1  : e.what = 0; break; 
            case 2  : e.what = 1; break; 
            case 3  : e.what = 2; break;
            case 0  : return;
            default: break;  
    } // switch
    } // if
    else
        e.event = viewer::event_mouse_move;
    e.x = MouseX;
    e.y = MouseY;
    handleInput( &e);
}

void ViewerFLTK::vKeyH()
{DBG
    viewer::event_info e;
    e.event = viewer::event_key_down;
    switch (Fl::event_key()) {
            case FL_Home:       e.what = viewer::key_home; 
                                break;
            case FL_Left:       e.what = viewer::key_left; 
                                break;
            case FL_Up:         e.what = viewer::key_up; 
                                break;
            case FL_Right:      e.what = viewer::key_right; 
                                break;
            case FL_Down:       e.what = viewer::key_down; 
                                break;
            case FL_Page_Up:    e.what = viewer::key_page_up; 
                                break;
            case FL_Page_Down:  e.what = viewer::key_page_down; 
                                break;

            default:
                                
                                return; 
    }

    handleInput( &e );
}

void ViewerFLTK::post_redraw()
{DBG
     da_widget->damage();
}

void ViewerFLTK::set_cursor( cursor_style c )
{DBG
    if (da_widget)
    {
        switch(c) {
                case cursor_info:
                        da_widget->cursor(FL_CURSOR_HAND);
                        break;
                case cursor_cycle: //kai-uwe: another cursor would be nice here
                        da_widget->cursor(FL_CURSOR_MOVE);
                        break;
                case cursor_up_down:
                        da_widget->cursor(FL_CURSOR_NS);
                        break;
                case cursor_crosshair:
                        da_widget->cursor(FL_CURSOR_CROSS);
                        break;
                case cursor_inherit:
                        // what does VRML want here? 
                default:
                        da_widget->cursor(FL_CURSOR_DEFAULT);
                        break;
        }
    }
}

void ViewerFLTK::swap_buffers()
{DBG
    da_widget->swap_buffers();
}

void ViewerFLTK::timerUpdate()
{DBG
  //update( 0.0 );	// No gl calls should be made from update()
}


void ViewerFLTK::set_timer( double t ) 
{DBG
}



