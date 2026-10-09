#define QT_NO_DEBUG_OUTPUT
#include "EventHandling.h"
#include "CentralWidget.h"
#include "EventParser.h"
#include "drawContext.h"

#include <QMouseEvent>
#include <QDebug>

#include <unordered_set>
#include <iostream>

EventContext3D::EventContext3D(CentralWidget* cw)
  :m_glWidget(cw){}

EventContext3D::~EventContext3D(){
  // removing captured handlers from context
  while(!m_captureHandlers.empty()){
    auto eh = m_captureHandlers.top();
    eh->popedInContext(this);
    m_captureHandlers.pop();
  }
}

void
EventContext3D::setEvent(QEvent* e) {
  //m_glWidget->makeCurrent();
  m_mouseHistory.clear();
  m_x = EventParser(e).x();
  m_y = EventParser(e).y();

  m_gly = 2.*(m_glWidget->height()/2. - m_y)/float(m_glWidget->height());
  m_glx = 2.*(m_x-m_glWidget->width()/2.)/float(m_glWidget->width());

  //std::cout << m_x << "," << m_gly << "," << m_glWidget->height() << std::endl;

  if(e->type() == QEvent::MouseButtonPress){
    QMouseEvent* m_e = static_cast<QMouseEvent*>(e);
    switch(int(m_e->button())){
      case Qt::LeftButton:  m_mouseHistory = "M:L:DOWN"; break;
      case Qt::MiddleButton:  m_mouseHistory = "M:M:DOWN"; break;
      case Qt::RightButton:  m_mouseHistory = "M:R:DOWN"; break;
      default: m_mouseHistory.clear();
    }
    qDebug() << m_mouseHistory;
  }
  else if(e->type() == QEvent::MouseButtonRelease){
    QMouseEvent* m_e = static_cast<QMouseEvent*>(e);
    switch(int(m_e->button())){
      case Qt::LeftButton:  m_mouseHistory = "M:L:UP"; break;
      case Qt::MiddleButton:  m_mouseHistory = "M:M:UP"; break;
      case Qt::RightButton:  m_mouseHistory = "M:R:UP"; break;
      default: m_mouseHistory.clear();
    }
    qDebug() << m_mouseHistory;
  }
  else if(e->type() == QEvent::MouseMove){
    m_mouseHistory = "M:MOVE";
    qDebug() << m_mouseHistory;
  }
  else if(e->type() == QEvent::KeyPress){
    QKeyEvent* m_e = static_cast<QKeyEvent*>(e);
    if(QChar::isPrint(m_e->key()))
      m_keyHistory = QString("K:%1:DOWN").arg(QChar(m_e->key()));
    else
      m_keyHistory = QString("K:Unkn:DOWN");
    qDebug() << m_keyHistory;
  }
  else if(e->type() == QEvent::KeyRelease){
    QKeyEvent* m_e = static_cast<QKeyEvent*>(e);
    if(QChar::isPrint(m_e->key()))
      m_keyHistory = QString("K:%1:UP").arg(QChar(m_e->key()));
    else
      m_keyHistory = QString("K:Unkn:UP");
    qDebug() << m_keyHistory;
  }
  else if(e->type() == QEvent::Resize){
    QResizeEvent* m_e = static_cast<QResizeEvent*>(e);
    m_mouseHistory = "S:RESIZE";
    m_x= m_e->size().width();
    m_y= m_e->size().height();
    qDebug() << m_mouseHistory << m_x << m_y;
  }
}

bool
EventContext3D::tryProcessCapture(){
  qDebug() << "total capture list size: " << m_captureHandlers.size();
  if(!m_captureHandlers.empty()){
    m_captureHandlers.top()->handle(*this);
    return true;
  }
  return false;
}

bool
EventContext3D::isMatched(QString const& eventstring){
  if(eventstring.isEmpty())
    return false;
  auto items = eventstring.split("+");
  qDebug() << eventstring << m_keyHistory << m_mouseHistory;
  for(auto& i:items){
    if(m_keyHistory.endsWith(i))
      continue;
    if(m_mouseHistory.endsWith(i))
      continue;
    return false;
  }
  return true;
}

void
EventContext3D::pushHandler(EventHandler3D* eh){
  m_captureHandlers.push(eh);
  eh->pushedInContext(this);
}

void EventContext3D::popHandler(){
  assert(!m_captureHandlers.empty());
  auto eh = m_captureHandlers.top();
  eh->popedInContext(this);
  m_captureHandlers.pop();
}

int EventContext3D::x(){
  return m_x;
}

int EventContext3D::y(){
  return m_y;
}

int EventContext3D::w(){
  return m_glWidget->width();
}

int EventContext3D::h(){  
  return m_glWidget->height();
}

float EventContext3D::glx(){
  return m_glx;
}

float EventContext3D::gly(){
  return m_gly;
}


void EventContext3D::update(){
  m_glWidget->update();
}

EventHandler3D::~EventHandler3D(){
  for(auto& i: m_children){
    i->m_parents.erase(this);
  }
  for(auto& i: m_parents){
    i->m_children.remove(this);
  }
  while(!m_inContextStack.empty()){
    auto cx = m_inContextStack.top();
    cx->popHandler();
  }
}

void
EventHandler3D::handle(EventContext3D& ecntx){
  for(auto& i: m_reacts){
    if(ecntx.isMatched(i.first)){
      i.second(ecntx);
      return;
    }
  }

  for(auto& i:m_children){
    i->handle(ecntx);
  }
}

void
EventHandler3D::addChild(EventHandler3D* eh){
  // regestering delete notifiers
  eh->m_parents.insert(this);
  m_children.remove(eh);
  m_children.push_back(eh);
}

void
EventHandler3D::removeChild(EventHandler3D* ev){
  ev->m_parents.erase(this);
  m_children.remove(ev);
}

std::function<void(EventContext3D& ecntx)>&
EventHandler3D::addReact(QString const & event){
  m_reacts.push_back({event, {}});
  return m_reacts.back().second;
}

void
EventHandler3D::pushedInContext(EventContext3D* ecntx){
  assert(ecntx);
  m_inContextStack.push(ecntx);
}

void
EventHandler3D::popedInContext(EventContext3D* ecntx){
  assert(ecntx);
  assert(!m_inContextStack.empty());
  assert(ecntx == m_inContextStack.top());
  m_inContextStack.pop();
}
