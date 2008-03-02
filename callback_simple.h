//======================================================================
// callback_simple.h
//
//
// Copyright (c) 1996 Paul Jakubik 
// 
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  Paul Jakubik makes no representations 
// about the suitability of this software for any purpose.  It is 
// provided "as is" without express or implied warranty.
//----------------------------------------------------------------------

#ifndef CALLBACK_SIMPLE_H
#define CALLBACK_SIMPLE_H

//----------------------------------------
// l i b r a r y c o d e 
//----------------------------------------
class CountedBody
{
public:
  CountedBody(): count_(0) {}
  virtual ~CountedBody() {}
  void incCount() {count_++;}
  void decCount() {count_--;}
  int count() {return count_;}

private:
  int count_;
};
//----------------------------------------
template <class P1>
class Base_Callback1Body:
  public CountedBody
{
public:
  virtual void operator()(P1) const = 0;
};

//----------------------------------------
template <class P1>
class Nil_Callback1Body:
  public Base_Callback1Body<P1>
{
public:
  /*virtual*/ void operator()(P1 parm_) const
  {throw "nil callback invoked";}
};
//----------------------------------------

template <class P1>
class Callback1
{
public:
  Callback1():
    body_(new Nil_Callback1Body<P1>)
  {}

  Callback1(Base_Callback1Body<P1>* body):
    body_(body)
  {this->incBodyCount();}

  Callback1(const Callback1<P1>& callback):
    body_(callback.body_)
  {this->incBodyCount();}

  ~Callback1() {this->decBodyCount(); body_ = 0;}

  Callback1<P1>& operator=(const Callback1<P1>& callback)
  {
    if (body_ != callback.body_)
    {
      this->decBodyCount();
      body_ = callback.body_;
      this->incBodyCount();
    }
    return *this;
  }

void operator()(P1 p1) {(*body_)(p1);}

private:
  Base_Callback1Body<P1>* body_;
  void incBodyCount() {body_->incCount();}
  void decBodyCount() {body_->decCount(); if(body_->count() == 0) delete body_;}
};
//----------------------------------------
template <class P1, class Function>
class Function_Callback1Body:
  public Base_Callback1Body<P1>
{
public:
  Function_Callback1Body(Function& function_):
    _function(function_) 
  {}

  /*virtual*/ void operator()(P1 parm_) const
  {_function(parm_);}

private:
  Function _function;
};
//----------------------------------------
template <class P1, class Client, class Member>
class Member_Callback1Body:
  public Base_Callback1Body<P1>
{
public:
  Member_Callback1Body(Client& client_, Member member_):
    _client(client_), 
    _member(member_)
  {}

  /*virtual*/ void operator()(P1 parm_) const
  {((&_client)->*_member)(parm_);}

private:
  Client& _client;
  Member _member;
};
//----------------------------------------
template <class P1, class Client, class Member>
Callback1<P1>
make_callback(Callback1<P1>*, Client& client_, Member member_)
{
  return Callback1<P1>
  (
    new Member_Callback1Body<P1,Client,Member>(client_,member_)
  );
}
//----------------------------------------
template <class P1, class Function>
Callback1<P1>
make_callback(Callback1<P1>*, Function function_)
{return Callback1<P1> (new Function_Callback1Body<P1,Function>(function_));}
//----------------------------------------
class Button
{
public:
  Button(const Callback1<Button*>& callback_):_callback(callback_){}
  ~Button() {}
  void push() {_callback(this);}

private:
  Callback1<Button*> _callback;
};
//----------------------------------------
// c l i e n t c o d e
//----------------------------------------
#if 0
#include <iostream>
class CdPlayer
{
public:
  void playButtonPushed(Button*) {std::cout << "PLAY" << std::endl;}
  void stopButtonPushed(Button*) {std::cout << "STOP" << std::endl;}
};
//----------------------------------------
void buttonTest(Button*) {std::cout << "BUTTON TEST" << std::endl;}
//----------------------------------------
main()
{
  CdPlayer aCdPlayer;

  Button playButton
  (
    make_callback
    ((Callback1<Button*>*)0, aCdPlayer, &CdPlayer::playButtonPushed)
  );

  Button stopButton
  (
    make_callback
    ((Callback1<Button*>*)0, aCdPlayer, &CdPlayer::stopButtonPushed)
  );

  playButton.push();
  stopButton.push();

  Button testFunc(make_callback((Callback1<Button*>*)0,buttonTest));
  testFunc.push();

  Callback1<Button*> nullCb;
  Button testNull(nullCb);

  try
  {
    testNull.push();
  }
  catch(const char * p)
  {
    std::cout << "caught exception: " << p << std::endl;
  }

  return 0;
};

#endif

#endif //CALLBACK_SIMPLE_H
