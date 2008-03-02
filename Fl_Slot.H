/* ***************************************************************************
 *  
 *  Copyright (C) by rio (heiko hoppe; heihop@web.de)
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *************************************************************************** */
/* Fl_Slot.H
 * slot: callback on non static memberfunction
 *
 * here 4 std slots imple.
 * 		void _slotclass::function(void)
 * 		void _slotclass::function(void* data)
 * 		void _slotclass::function(Fl_Widget* object)
 * 		void _slotclass::function(Fl_Widget* object,void* data)
 *
 * two ways to set a slot
 *		Fl_Widget->callback(fl_slotcall,fl_slot(pointer_to_slotclass,&_slotclass::function[,(void*)userdata]));
 *		or use a define 
 * 		Fl_Widget->slotcall(fl_slot(pointer_to_slotclass,&_slotclass::function[,(void*)userdata]));
 *
 * slotclass:
 		class slotclass:public Fl_Slot, ... {
 			...
 			public:
 			slotclass():Fl_Slot() ... {};
 			...
 			void void_slot();
 			void arg_slot(void*);
 			void flObject_slot(Fl_Widget*);
 			void flObject_arg_slot(Fl_Widget*,void*);
 		};
 
 */
#ifndef _FL_SLOT_H_
#define _FL_SLOT_H_ 1

#include <Fl/Fl_Widget.H>

//little debug defines
#ifndef DOUT
#ifdef _DEBUG_
#include <iostream>
#include <stdio.h>
using namespace std;
#define DOUT(x) cout<<x
#define DPRINT printf
#else
#define DOUT(x)
inline void DPRINT(...){
	;// nothing
}
#endif
#endif


// callerbase
// to call a slot
class Fl_Slot_Caller{
  public:
  	Fl_Slot_Caller(){};
  	virtual ~Fl_Slot_Caller(){}; /* if(_next){ delete _next; } */
	virtual void call(Fl_Widget* o)=0;
  	virtual void next(Fl_Slot_Caller* n)=0;
};

// base for all slotclasses
// a simple stack for memory controll
class Fl_Slot{
  public:
  	Fl_Slot(){ _slotroot=0; };
	~Fl_Slot(){ if(_slotroot){ delete _slotroot; } };
	void _fl_slot_add(Fl_Slot_Caller* t){ t->next(_slotroot); _slotroot=t; }; 
  private:
  	Fl_Slot_Caller* _slotroot;
};

// the Fl_Callback* function
inline void fl_slotcall(Fl_Widget* o,void* d){ 
	static_cast<Fl_Slot_Caller*>(d)->call(o); 
};

// a little define, so we don't have to patch FLTK ;)
#define slotcall(X) callback(fl_slotcall,X)

/* ************************************************************************************ 
 * std caller
 */
		
// void _slotclass::methode(void) Slots 
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass*,void(_slotclass::*)(void));
	
// void _slotclass::methode(void* data) Slots 
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass*,void(_slotclass::*)(void*),void*);
	
// void _slotclass::methode(Fl_Widget*) Slots
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass*,void(_slotclass::*)(Fl_Widget*));
	
// void _slotclass::methode(Fl_Widget* object,void* data) Slots
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass*,void(_slotclass::*)(Fl_Widget*,void*),void*);





////////////////////////////////////////////////////////////////////////////////////////
// imple. //////////////////////////////////////////////////////////////////////////////
// template export /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////





namespace _Fl_Slot_inner{
// *************************************************************************************
template<class _slotclass>
class CallerVoid:public Fl_Slot_Caller{
  public:
	CallerVoid(_slotclass* t,void(_slotclass::*met)(void)){ $this=t; _met=met; _next=0; };
	~CallerVoid(){ if(_next){ delete _next; } DOUT("CallerVoid kaputt"<<endl); };
	void call(Fl_Widget*){ ($this->*_met)(); };
  	void next(Fl_Slot_Caller* n){ _next= n; };
  	Fl_Slot_Caller* next(){ return _next; };
  protected:
  	_slotclass* $this;
  	void(_slotclass::*_met)();
  	Fl_Slot_Caller* _next;
};

// *************************************************************************************
template<class _slotclass>
class CallerArg:public Fl_Slot_Caller{
  public:
	CallerArg(_slotclass* t,void(_slotclass::*met)(void*),void* ud)
		{ $this=t; _met=met; _next=0; _userdata=ud; };
	~CallerArg(){ if(_next){ delete _next; } DOUT("CallerArg kaputt"<<endl);};
	void call(Fl_Widget*){ ($this->*_met)(_userdata); };
  	void next(Fl_Slot_Caller* n){ _next= n; };
  	Fl_Slot_Caller* next(){ return _next; };
  protected:
  	_slotclass* $this;
  	void(_slotclass::*_met)(void*);
  	Fl_Slot_Caller* _next;
	void* _userdata;
};

// *************************************************************************************
template<class _slotclass>
class CallerFlObject:public Fl_Slot_Caller{
  public:
	CallerFlObject(_slotclass* t,void(_slotclass::*met)(Fl_Widget*)){ $this=t; _met=met; _next=0; };
	~CallerFlObject(){ if(_next){ delete _next; } DOUT("CallerFlObject kaputt"<<endl);};
	void call(Fl_Widget* o){ ($this->*_met)(o); };
  	void next(Fl_Slot_Caller* n){ _next= n; };
  	Fl_Slot_Caller* next(){ return _next; };
  protected:
  	_slotclass* $this;
  	void(_slotclass::*_met)(Fl_Widget*);
  	Fl_Slot_Caller* _next;
};

// *************************************************************************************
template<class _slotclass>
class CallerFlObjectArg:public Fl_Slot_Caller{
  public:
	CallerFlObjectArg(_slotclass* t,void(_slotclass::*met)(Fl_Widget*,void*),void* ud)
		{ $this=t; _met=met; _next=0; _userdata=ud; };
	~CallerFlObjectArg(){ if(_next){ delete _next; } DOUT("CallerFlObjectArg kaputt"<<endl); };
	void call(Fl_Widget* o){ ($this->*_met)(o,_userdata); };
  	void next(Fl_Slot_Caller* n){ _next= n; };
  	Fl_Slot_Caller* next(){ return _next; };
  protected:
  	_slotclass* $this;
  	void(_slotclass::*_met)(Fl_Widget*,void*);
  	Fl_Slot_Caller* _next;
	void* _userdata;
};
}// namespace _Fl_Slot_inner

// void _slotclass::methode(void) Slots 
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass* t,void(_slotclass::*met)(void)){
	_Fl_Slot_inner::CallerVoid<_slotclass>* _t= new _Fl_Slot_inner::CallerVoid<_slotclass>(t,met);
	t->_fl_slot_add((Fl_Slot_Caller*)_t);
	return (Fl_Slot_Caller*)_t;
};
	
// void _slotclass::methode(void* data) Slots 
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass* t,void(_slotclass::*met)(void*),void* d){
	_Fl_Slot_inner::CallerArg<_slotclass>* _t= new _Fl_Slot_inner::CallerArg<_slotclass>(t,met,d);
	t->_fl_slot_add((Fl_Slot_Caller*)_t);
	return (Fl_Slot_Caller*)_t;
};
	
// void _slotclass::methode(Fl_Widget*) Slots
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass* t,void(_slotclass::*met)(Fl_Widget*)){
	_Fl_Slot_inner::CallerFlObject<_slotclass>* _t= new _Fl_Slot_inner::CallerFlObject<_slotclass>(t,met);
	t->_fl_slot_add((Fl_Slot_Caller*)_t);
	return (Fl_Slot_Caller*)_t;
};
	
// void _slotclass::methode(Fl_Widget* object,void* data) Slots
template<class _slotclass>
Fl_Slot_Caller* fl_slot(_slotclass* t,void(_slotclass::*met)(Fl_Widget*,void*),void* d){
	_Fl_Slot_inner::CallerFlObjectArg<_slotclass>* _t= new _Fl_Slot_inner::CallerFlObjectArg<_slotclass>(t,met,d);
	t->_fl_slot_add((Fl_Slot_Caller*)_t);
	return (Fl_Slot_Caller*)_t;
};
#endif // _FL_SLOT_H_


 
