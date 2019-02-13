//
// "$Id: threads.h,v 1.1.2.8 2004/11/20 03:44:18 easysw Exp $"
//
// Simple threading API for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2004 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Inline classes to provide portable support for threads and mutexes.
//
// FLTK does not use this (it has an internal mutex implementation
// that is used if Fl::lock() is called). This header file's only
// purpose is so we can write portable demo programs. It may be useful
// or an inspiration to people who want to try writing multithreaded
// programs themselves.
//
// FLTK has no multithreaded support unless the main thread calls Fl::lock().
// This main thread is the only thread allowed to call Fl::run() or Fl::wait().
// From then on FLTK will be locked except when the main thread is actually
// waiting for events from the user. Other threads must call Fl::lock() and
// Fl::unlock() to surround calls to FLTK (such as to change widgets or
// redraw them).

#ifndef Threads_H
#  define Threads_H

#if !defined(_WIN32)
#ifndef HAVE_PTHREAD_H
#define HAVE_PTHREAD_H 1
#endif
#endif

#  if HAVE_PTHREAD_H
// Use POSIX threading...

#    include <pthread.h>
#    include <limits.h> /* PTHREAD_STACK_MIN */

typedef pthread_t Fl_Thread;

int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p);

#  elif defined(_WIN32) && !defined(__WATCOMC__) // Use Windows threading...

#    include <windows.h>
#    include <process.h>

typedef unsigned long Fl_Thread;

int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p);

#  elif defined(__WATCOMC__)
#    include <process.h>

typedef unsigned long Fl_Thread;

int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p);
#  endif // !HAVE_PTHREAD_H
#endif // !Threads_h

//
// End of "$Id: threads.h,v 1.1.2.8 2004/11/20 03:44:18 easysw Exp $".
//
