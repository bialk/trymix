#include "EventParser.h"

EventParser::EventParser(QEvent* e)
  :m_e(e){}
bool EventParser::mouseButtonPress(Qt::MouseButton buttons){
  if(m_e->type() == QEvent::MouseButtonPress){
    QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
    return e->button() & buttons;
  }
  return false;
}
bool EventParser::mouseButtonRelease(Qt::MouseButton buttons){
  if(m_e->type() == QEvent::MouseButtonRelease){
    QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
    return e->button() & buttons;
  }
  return false;
}
bool EventParser::mouseButtonIsPressed(Qt::MouseButton buttons){
  QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
  return e? bool(e->buttons() & buttons): false;
}
bool EventParser::mouseMove(){
  return m_e->type() == QEvent::MouseMove;
}
int EventParser::x(){
  QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
  return e?e->x():INT_MIN;
}
int EventParser::y(){
  QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
  return e?e->y():INT_MIN;
}
bool EventParser::keyPress(Qt::Key button){
  if(m_e->type() == QEvent::KeyPress){
    QKeyEvent* e = static_cast<QKeyEvent*>(m_e);
    return e->key() == button;
  }
  return false;
}
bool EventParser::keyRelease(Qt::Key button){
  if(m_e->type() == QEvent::KeyRelease){
    QKeyEvent* e = static_cast<QKeyEvent*>(m_e);
    return e->key() == button;
  }
  return false;
}

