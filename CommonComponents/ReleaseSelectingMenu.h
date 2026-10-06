#pragma once

#include <QMenu>
#include <QPointer>

class QHideEvent;
class QMouseEvent;
class QWidget;

class ReleaseSelectingMenu : public QMenu
{
public:
  using QMenu::QMenu;

protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void hideEvent(QHideEvent* event) override;

private:
  void updateHoveredRow(const QPoint& position);
  void setHoveredRow(QWidget* row);

  QPointer<QWidget> m_hoveredRow;
};
