#define QT_NO_DEBUG_OUTPUT
#include "EventHandling.h"
#include "CentralWidget.h"
#include <EventParser.h>

#include <QMouseEvent>
#include <QDebug>

#include <unordered_set>


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
  m_selectionId = -1;
  if(e->type() == QEvent::MouseButtonPress){
    QMouseEvent* m_e = static_cast<QMouseEvent*>(e);
    switch(int(m_e->button())){
      case Qt::LeftButton:  m_mouseHistory = "M:L:DOWN"; break;
      case Qt::MiddleButton:  m_mouseHistory = "M:M:DOWN"; break;
      case Qt::RightButton:  m_mouseHistory = "M:R:DOWN"; break;
      default: m_mouseHistory.clear();
    }
    qDebug() << m_mouseHistory << Qt::endl;
  }
  else if(e->type() == QEvent::MouseButtonRelease){
    QMouseEvent* m_e = static_cast<QMouseEvent*>(e);
    switch(int(m_e->button())){
      case Qt::LeftButton:  m_mouseHistory = "M:L:UP"; break;
      case Qt::MiddleButton:  m_mouseHistory = "M:M:UP"; break;
      case Qt::RightButton:  m_mouseHistory = "M:R:UP"; break;
      default: m_mouseHistory.clear();
    }
    qDebug() << m_mouseHistory << Qt::endl;
  }
  else if(e->type() == QEvent::MouseMove){
    m_mouseHistory = "M:MOVE";
    qDebug() << m_mouseHistory << Qt::endl;
  }
  else if(e->type() == QEvent::KeyPress){
    QKeyEvent* m_e = static_cast<QKeyEvent*>(e);
    m_keyHistory = QString("K:%1:DOWN").arg(QChar(m_e->key()));
    qDebug() << m_keyHistory << Qt::endl;
  }
  else if(e->type() == QEvent::KeyRelease){
    QKeyEvent* m_e = static_cast<QKeyEvent*>(e);
    m_keyHistory = QString("K:%1:UP").arg(QChar(m_e->key()));
    qDebug() << m_keyHistory << Qt::endl;
  }
  else if(e->type() == QEvent::Resize){
    QResizeEvent* m_e = static_cast<QResizeEvent*>(e);
    m_mouseHistory = "S:RESIZE";
    m_x= m_e->size().width();
    m_y= m_e->size().height();
    qDebug() << m_keyHistory << x() << " " << y() << Qt::endl;
  }
}

//QEvent*
//EventContext3D::event() {return m_event;}

bool
EventContext3D::tryProcessCapture(){
  qDebug() << "total capture list size: " << m_captureHandlers.size() << Qt::endl;
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
  qDebug() << eventstring << m_keyHistory << m_mouseHistory << Qt::endl;
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

void EventContext3D::update(){
  m_glWidget->update();
}

int EventContext3D::select(){
  if(m_selectionId != -1)
    return m_selectionId;
  m_glWidget->makeCurrent();
  GLuint selectBuf[1024];
  glSelectBuffer (1024, selectBuf);
  glRenderMode (GL_SELECT);
  glInitNames();
  m_glWidget->paintGL();
  auto hits = glRenderMode (GL_RENDER);
  assert(hits != -1);
  if(hits==-1){
    qDebug() << "Selection Buffer overflow";
    hits=0;
    m_selectionId = 0;
    return m_selectionId;
  }

  auto processHits2 = [&](unsigned int stackdepth, unsigned int* stacknames)->int{
    int i;
    unsigned int j;
    float maxz1=1e10;
    GLuint maxname = 0;
    GLuint names, name, *ptr;
    //printf ("hits = %d\n", hits);
    ptr = (GLuint *) selectBuf;
    for (i = 0; i < hits; i++) {
      /* for each hit */
      names = *ptr; //printf (" number of names for this hit = %d\n", names);
      ptr++;
      float z1=static_cast<float>(*ptr/0x7fffffff);
      //printf(" z1 is %g;", (float) *ptr/0x7fffffff);
      ptr++;
      //printf(" z2 is %g\n", (float) *ptr/0x7fffffff);
      ptr++;
      //printf (" names are ");
      int count = stackdepth+1;
      int depthname;
      for (j = 0; j < names; j++) {
        /* for each name */
        name = *ptr;
        if( j<stackdepth) {if(stacknames[j]==name) count--;}
        else if(j==stackdepth) { depthname=name; count--;}
        //printf ("%d ", name);
        ptr++;
      }
      if(count==0 && z1<maxz1 ){
        maxname = depthname; maxz1=z1;
      }
      //printf ("\n");
    }
    //printf("maxname=%i\n",maxname);
    return maxname;
  };

  unsigned int stack[]={0};
  return m_selectionId = processHits2(0, stack);
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
