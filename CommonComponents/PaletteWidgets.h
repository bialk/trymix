#pragma once

#include <QColor>
#include <QWidget>

class QComboBox;
class QEnterEvent;
class QEvent;
class QPalette;

QColor paletteTint(const QPalette& palette, int accentWeight);
void applyPalettePopupStyle(QComboBox* comboBox);

class PaletteHoverRow : public QWidget
{
public:
  explicit PaletteHoverRow(QWidget* parent = nullptr);

  void setHovered(bool hovered);
  bool isHovered() const;

protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void changeEvent(QEvent* event) override;

private:
  void updateHoverStyle();

  bool m_hovered = false;
  bool m_updatingStyle = false;
};
