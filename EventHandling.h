#ifndef EVENTHANDLING_H
#define EVENTHANDLING_H

#include <stack>
#include <functional>
#include <unordered_set>

#include <QString>

class QEvent;
class CentralWidget;
class EventHandler3D;
class ViewCtrl;

class EventContext3D{
public:
  EventContext3D(CentralWidget* cw);
  ~EventContext3D();

  void setEvent(QEvent* e);
  QEvent* event();
  CentralWidget* glWidget();

  bool tryProcessCapture();
  bool isMatched(const QString& eventstring);
  void pushHandler(EventHandler3D* eh);
  void popHandler();
  int x();
  int y();
  void update();

private:

  CentralWidget *m_glWidget;
  QEvent* m_event = nullptr;
  std::stack<EventHandler3D*> m_captureHandlers;
  QString m_keyHistory;
  QString m_mouseHistory;
  int m_x,m_y;
};

class EventHandler3D{
public:
  EventHandler3D() = default;
  virtual ~EventHandler3D();
  virtual void handle(EventContext3D& ecntx);

  void addChild(EventHandler3D* eh);
  void removeChild(EventHandler3D* ev);

  std::function<void(EventContext3D& ecntx)>& addReact(QString const & event);

  void pushedInContext(EventContext3D* ecntx);
  void popedInContext(EventContext3D* ecntx);
private:
  std::stack<EventContext3D*> m_inContextStack;
  std::list<EventHandler3D*> m_children;
  std::unordered_set<EventHandler3D*> m_parents;
  std::list<std::pair<QString, std::function<void(EventContext3D& ecntx)> > > m_reacts;
};

#endif // EVENTHANDLING_H
