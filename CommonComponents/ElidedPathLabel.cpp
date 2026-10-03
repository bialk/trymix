#include "ElidedPathLabel.h"

#include <QEvent>
#include <QFontMetrics>
#include <QResizeEvent>

ElidedPathLabel::ElidedPathLabel(QWidget* parent)
  : QLabel(parent)
{}

void ElidedPathLabel::setFullText(const QString& text)
{
  m_fullText = text;
  updateElidedText();
}

const QString& ElidedPathLabel::fullText() const
{
  return m_fullText;
}

QSize ElidedPathLabel::minimumSizeHint() const
{
  return {0, QLabel::minimumSizeHint().height()};
}

void ElidedPathLabel::resizeEvent(QResizeEvent* event)
{
  QLabel::resizeEvent(event);
  updateElidedText();
}

void ElidedPathLabel::changeEvent(QEvent* event)
{
  QLabel::changeEvent(event);
  if (event->type() == QEvent::FontChange ||
      event->type() == QEvent::StyleChange)
    updateElidedText();
}

void ElidedPathLabel::updateElidedText()
{
  QLabel::setText(fontMetrics().elidedText(
    m_fullText, Qt::ElideLeft, contentsRect().width()));
}
