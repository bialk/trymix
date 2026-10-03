#pragma once

#include <QLabel>

class ElidedPathLabel : public QLabel
{
public:
  explicit ElidedPathLabel(QWidget* parent = nullptr);

  void setFullText(const QString& text);
  const QString& fullText() const;

  QSize minimumSizeHint() const override;

protected:
  void resizeEvent(QResizeEvent* event) override;
  void changeEvent(QEvent* event) override;

private:
  void updateElidedText();

  QString m_fullText;
};
