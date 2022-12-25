#include "ViewPortController.h"

#include <QMouseEvent>
#include <QDebug>

#include <unordered_set>


ViewPortController::ViewPortController()
{

}



EventContext3D::EventContext3D(CentralWidget* cw)
  :m_glWidget(cw){}

void
EventContext3D::setEvent(QEvent* e) {m_event = e;}

QEvent*
EventContext3D::event() {return m_event;}

bool
EventContext3D::tryProcessCapture(){
  //qDebug() << "total capture list size: " << m_capture.size() << Qt::endl;
  if(isCaptured()){
    return m_capture.top()(*this);
  }
  return false;
}

void
EventContext3D::setCapture(std::function<bool(EventContext3D& ecntx)> handler){
  m_capture.push(handler);
}

void
EventContext3D::releaseCapture(){
  m_capture.pop();
};
bool EventContext3D::isCaptured(){
  return !m_capture.empty();
}

CentralWidget* EventContext3D::glWidget(){
  return m_glWidget;
}






EventHandler3D::~EventHandler3D(){
  for(auto& i: m_children){
    i->m_parents.erase(this);
  }
  for(auto& i: m_parents){
    i->m_children.remove(this);
  }
}

void
EventHandler3D::handle(EventContext3D& ecntx){

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


