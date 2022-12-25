#ifndef VIEWPORTCONTROLLER_H
#define VIEWPORTCONTROLLER_H

#include <stack>
#include <functional>
#include <unordered_set>

class QEvent;
class CentralWidget;

class EventContext3D{
public:
  EventContext3D(CentralWidget* cw);

  void setEvent(QEvent* e);
  QEvent* event();

  bool tryProcessCapture();

  void setCapture(std::function<bool(EventContext3D& ecntx)> handler);
  void releaseCapture();
  bool isCaptured();

  CentralWidget* glWidget();

private:
  CentralWidget *m_glWidget;
  QEvent* m_event = nullptr;
  std::stack<std::function<bool(EventContext3D& ecntx)>> m_capture;
};

class EventHandler3D{
public:
  virtual ~EventHandler3D();
  virtual void handle(EventContext3D& ecntx);

  void addChild(EventHandler3D* eh);
  void removeChild(EventHandler3D* ev);

private:
  std::list<EventHandler3D*> m_children;
  std::unordered_set<EventHandler3D*> m_parents;
};


class ViewPortController
{
public:
  ViewPortController();
};



#endif // VIEWPORTCONTROLLER_H
