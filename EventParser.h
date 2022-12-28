#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <QMouseEvent>

class EventParser{
public:
  EventParser(QEvent* e);
  bool mouseButtonPress(Qt::MouseButton buttons);
  bool mouseButtonRelease(Qt::MouseButton buttons);
  bool mouseButtonIsPressed(Qt::MouseButton buttons);
  bool mouseMove();
  int x();
  int y();
  bool keyPress(Qt::Key button);
  bool keyRelease(Qt::Key button);

private:
  QEvent* m_e = nullptr;
};

#endif // EVENTPARSER_H
