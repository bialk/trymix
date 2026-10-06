#include "PaletteWidgets.h"

#include <QComboBox>
#include <QEnterEvent>
#include <QEvent>
#include <QPalette>

QColor paletteTint(const QPalette& palette, int accentWeight)
{
  accentWeight = qBound(0, accentWeight, 100);
  const QColor window = palette.color(QPalette::Window);
  const QColor accent = palette.color(QPalette::Highlight);
  const int windowWeight = 100 - accentWeight;
  return QColor(
    (window.red() * windowWeight + accent.red() * accentWeight) / 100,
    (window.green() * windowWeight + accent.green() * accentWeight) / 100,
    (window.blue() * windowWeight + accent.blue() * accentWeight) / 100);
}

void applyPalettePopupStyle(QComboBox* comboBox)
{
  if (!comboBox)
    return;

  const QPalette colors = comboBox->palette();
  comboBox->setStyleSheet(QStringLiteral(
    "QComboBox QAbstractItemView {"
    " border: 1px solid %1; outline: none;"
    " selection-background-color: %2; selection-color: %3; }"
    "QComboBox QAbstractItemView::item { min-height: 24px; padding: 2px 6px; }"
    "QComboBox QAbstractItemView::item:hover {"
    " background-color: %2; color: %3; }"
    "QComboBox QAbstractItemView::item:disabled { color: %4; }"
  ).arg(paletteTint(colors, 55).name(QColor::HexRgb),
        paletteTint(colors, 30).name(QColor::HexRgb),
        colors.color(QPalette::WindowText).name(QColor::HexRgb),
        colors.color(QPalette::Disabled, QPalette::Text)
          .name(QColor::HexRgb)));
}

PaletteHoverRow::PaletteHoverRow(QWidget* parent)
  : QWidget(parent)
{
  setProperty("paletteHoverRow", true);
}

void PaletteHoverRow::setHovered(bool hovered)
{
  if (m_hovered == hovered)
    return;
  m_hovered = hovered;
  updateHoverStyle();
}

bool PaletteHoverRow::isHovered() const
{
  return m_hovered;
}

void PaletteHoverRow::enterEvent(QEnterEvent* event)
{
  setHovered(true);
  QWidget::enterEvent(event);
}

void PaletteHoverRow::leaveEvent(QEvent* event)
{
  setHovered(false);
  QWidget::leaveEvent(event);
}

void PaletteHoverRow::changeEvent(QEvent* event)
{
  QWidget::changeEvent(event);
  if (!m_updatingStyle && m_hovered &&
      (event->type() == QEvent::PaletteChange ||
       event->type() == QEvent::StyleChange))
    updateHoverStyle();
}

void PaletteHoverRow::updateHoverStyle()
{
  if (m_updatingStyle)
    return;

  m_updatingStyle = true;
  if (!m_hovered) {
    setStyleSheet({});
    m_updatingStyle = false;
    return;
  }

  const QPalette colors = palette();
  setStyleSheet(QStringLiteral(
    "QWidget[paletteHoverRow=\"true\"] {"
    " background-color: %1; border: 1px solid %2; border-radius: 3px; }"
    "QWidget[paletteHoverRow=\"true\"] QPushButton, "
    "QWidget[paletteHoverRow=\"true\"] QToolButton {"
    " color: %3; background-color: transparent; border: none; }"
  ).arg(paletteTint(colors, 30).name(QColor::HexRgb),
        paletteTint(colors, 55).name(QColor::HexRgb),
        colors.color(QPalette::WindowText).name(QColor::HexRgb)));
  m_updatingStyle = false;
}
