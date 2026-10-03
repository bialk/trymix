#include "QScintillaEditor.h"

#include <QColor>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QPalette>

#include <Qsci/qscilexeryaml.h>
#include <Qsci/qsciscintilla.h>

#include <algorithm>
#include <vector>

namespace {

class MultiCaretNavigationFilter final : public QObject
{
public:
  explicit MultiCaretNavigationFilter(QsciScintilla* editor)
    : QObject(editor), m_editor(editor)
  {}

protected:
  bool eventFilter(QObject* watched, QEvent* event) override
  {
    if (watched != m_editor || event->type() != QEvent::KeyPress)
      return QObject::eventFilter(watched, event);

    auto* keyEvent = static_cast<QKeyEvent*>(event);
    const bool moveLeft = keyEvent->key() == Qt::Key_Left;
    const bool moveRight = keyEvent->key() == Qt::Key_Right;
    const bool moveHome = keyEvent->key() == Qt::Key_Home;
    const bool moveEnd = keyEvent->key() == Qt::Key_End;
    if (!moveLeft && !moveRight && !moveHome && !moveEnd)
      return QObject::eventFilter(watched, event);

    const auto modifiers = keyEvent->modifiers() & ~Qt::KeypadModifier;
    if (modifiers != Qt::NoModifier && modifiers != Qt::ShiftModifier)
      return QObject::eventFilter(watched, event);

    const int selectionCount = static_cast<int>(m_editor->SendScintilla(
      QsciScintilla::SCI_GETSELECTIONS));
    if (selectionCount <= 1)
      return QObject::eventFilter(watched, event);

    const bool extendSelection = modifiers == Qt::ShiftModifier;
    std::vector<long> newCarets(selectionCount);
    for (int selection = 0; selection < selectionCount; ++selection) {
      const long caret = m_editor->SendScintilla(
        QsciScintilla::SCI_GETSELECTIONNCARET, selection);
      const long anchor = m_editor->SendScintilla(
        QsciScintilla::SCI_GETSELECTIONNANCHOR, selection);

      if (moveHome || moveEnd) {
        const long line = m_editor->SendScintilla(
          QsciScintilla::SCI_LINEFROMPOSITION, caret);
        newCarets[selection] = m_editor->SendScintilla(
          moveHome ? QsciScintilla::SCI_POSITIONFROMLINE
                   : QsciScintilla::SCI_GETLINEENDPOSITION,
          line);
      } else if (!extendSelection && caret != anchor) {
        newCarets[selection] = moveLeft
          ? std::min(caret, anchor)
          : std::max(caret, anchor);
      } else {
        newCarets[selection] = m_editor->SendScintilla(
          moveLeft ? QsciScintilla::SCI_POSITIONBEFORE
                   : QsciScintilla::SCI_POSITIONAFTER,
          caret);
      }
    }

    for (int selection = 0; selection < selectionCount; ++selection) {
      const auto index = static_cast<unsigned long>(selection);
      m_editor->SendScintilla(
        QsciScintilla::SCI_SETSELECTIONNCARET, index, newCarets[selection]);
      if (!extendSelection) {
        m_editor->SendScintilla(
          QsciScintilla::SCI_SETSELECTIONNANCHOR, index,
          newCarets[selection]);
      }
    }

    return true;
  }

private:
  QsciScintilla* m_editor;
};

}

void QScintillaEditor::configureYaml(QsciScintilla* editor)
{
  const QPalette palette = editor->palette();
  const QColor background = palette.color(QPalette::Base);
  const QColor foreground = palette.color(QPalette::Text);
  const bool darkTheme = background.lightness() < foreground.lightness();
  QFont editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  editorFont.setPointSizeF(editorFont.pointSizeF() + 1.0);

  auto* lexer = new QsciLexerYAML(editor);
  editor->setLexer(lexer);
  lexer->setDefaultFont(editorFont);
  lexer->setFont(editorFont, -1);
  lexer->setDefaultPaper(background);
  lexer->setPaper(background, -1);
  lexer->setDefaultColor(foreground);
  lexer->setColor(foreground, -1);

  lexer->setColor(darkTheme ? QColor("#6A9955") : QColor("#008000"),
                  QsciLexerYAML::Comment);
  lexer->setColor(darkTheme ? QColor("#9CDCFE") : QColor("#001080"),
                  QsciLexerYAML::Identifier);
  lexer->setColor(darkTheme ? QColor("#C586C0") : QColor("#AF00DB"),
                  QsciLexerYAML::Keyword);
  lexer->setColor(darkTheme ? QColor("#B5CEA8") : QColor("#098658"),
                  QsciLexerYAML::Number);
  lexer->setColor(darkTheme ? QColor("#4EC9B0") : QColor("#267F99"),
                  QsciLexerYAML::Reference);
  lexer->setColor(darkTheme ? QColor("#569CD6") : QColor("#0000FF"),
                  QsciLexerYAML::DocumentDelimiter);
  lexer->setColor(darkTheme ? QColor("#DCDCAA") : QColor("#795E26"),
                  QsciLexerYAML::TextBlockMarker);
  lexer->setColor(darkTheme ? QColor("#F44747") : QColor("#CD3131"),
                  QsciLexerYAML::SyntaxErrorMarker);
  lexer->setColor(darkTheme ? QColor("#D4D4D4") : QColor("#202020"),
                  QsciLexerYAML::Operator);

  editor->setFont(editorFont);
  editor->setExtraAscent(1);
  editor->setExtraDescent(1);
  editor->SendScintilla(QsciScintilla::SCI_SETMULTIPLESELECTION, 1);
  editor->SendScintilla(QsciScintilla::SCI_SETADDITIONALSELECTIONTYPING, 1);
  editor->SendScintilla(QsciScintilla::SCI_SETADDITIONALCARETSVISIBLE, 1);
  editor->SendScintilla(QsciScintilla::SCI_SETADDITIONALCARETSBLINK, 1);
  editor->SendScintilla(QsciScintilla::SCI_SETADDITIONALCARETFORE, foreground);
  editor->SendScintilla(
    QsciScintilla::SCI_SETMULTIPASTE, QsciScintilla::SC_MULTIPASTE_EACH);
  editor->installEventFilter(new MultiCaretNavigationFilter(editor));
  editor->setColor(foreground);
  editor->setPaper(background);
  editor->setCaretForegroundColor(foreground);
  editor->setSelectionBackgroundColor(palette.color(QPalette::Highlight));
  editor->setSelectionForegroundColor(palette.color(QPalette::HighlightedText));
  editor->setMarginsBackgroundColor(palette.color(QPalette::AlternateBase));
  editor->setMarginsForegroundColor(foreground);
  editor->setMarginsFont(editorFont);
  editor->setMarginType(0, QsciScintilla::NumberMargin);
  editor->setMarginLineNumbers(0, true);
  editor->setMarginWidth(0, QStringLiteral("0000"));
  editor->setWrapMode(QsciScintilla::WrapNone);
  editor->setIndentationsUseTabs(false);
  editor->setTabWidth(2);
  editor->setAutoIndent(true);
}
