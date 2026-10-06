#include "ReleaseSelectingMenu.h"
#include "PaletteWidgets.h"

#include <QAbstractButton>
#include <QHideEvent>
#include <QMouseEvent>
#include <QWidgetAction>

void ReleaseSelectingMenu::mouseMoveEvent(QMouseEvent* event)
{
  updateHoveredRow(event->position().toPoint());
  QMenu::mouseMoveEvent(event);
}

void ReleaseSelectingMenu::mouseReleaseEvent(QMouseEvent* event)
{
  updateHoveredRow(event->position().toPoint());
  if (QAction* action = actionAt(event->position().toPoint())) {
    if (auto* widgetAction = qobject_cast<QWidgetAction*>(action)) {
      if (QWidget* row = widgetAction->defaultWidget()) {
        QWidget* child = row->childAt(row->mapFromGlobal(
          event->globalPosition().toPoint()));
        while (child && child != row) {
          if (auto* button = qobject_cast<QAbstractButton*>(child)) {
            button->click();
            event->accept();
            return;
          }
          child = child->parentWidget();
        }
        widgetAction->trigger();
        event->accept();
        return;
      }
    }
  }
  QMenu::mouseReleaseEvent(event);
}

void ReleaseSelectingMenu::leaveEvent(QEvent* event)
{
  setHoveredRow(nullptr);
  QMenu::leaveEvent(event);
}

void ReleaseSelectingMenu::hideEvent(QHideEvent* event)
{
  setHoveredRow(nullptr);
  QMenu::hideEvent(event);
}

void ReleaseSelectingMenu::updateHoveredRow(const QPoint& position)
{
  QWidget* row = nullptr;
  if (auto* widgetAction = qobject_cast<QWidgetAction*>(actionAt(position)))
    row = widgetAction->defaultWidget();
  setHoveredRow(row);
}

void ReleaseSelectingMenu::setHoveredRow(QWidget* row)
{
  if (m_hoveredRow == row)
    return;

  if (auto* oldRow = dynamic_cast<PaletteHoverRow*>(m_hoveredRow.data()))
    oldRow->setHovered(false);
  m_hoveredRow = row;
  if (auto* newRow = dynamic_cast<PaletteHoverRow*>(m_hoveredRow.data()))
    newRow->setHovered(true);
}
