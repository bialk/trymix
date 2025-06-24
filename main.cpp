#include "CommonComponents/mainwindow.h"
#include "apputil/serializerV2test.h"
#include "qevent.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyle>
#include <QScreen>
#include <QCommandLineParser>
#include <QStyleFactory>

// test definitions
void test_ProcessAsServer();

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);  
  a.setApplicationName("Try Mix!");
  a.setApplicationVersion("1.0");
  a.setStyle(QStyleFactory::create("Fusion"));

  // dark palette
  QPalette newPalette;
  newPalette.setColor(QPalette::PlaceholderText, QColor(127, 127, 127));

  newPalette.setColor(QPalette::Light,           QColor( 60,  60,  60));
  newPalette.setColor(QPalette::Midlight,        QColor( 52,  52,  52));
  newPalette.setColor(QPalette::Mid,             QColor( 37,  37,  37));

  newPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  newPalette.setColor(QPalette::WindowText, Qt::white);
  newPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                  QColor(127, 127, 127));
  newPalette.setColor(QPalette::Base, QColor(42, 42, 42));
  newPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
  newPalette.setColor(QPalette::ToolTipBase, Qt::white);
  newPalette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
  newPalette.setColor(QPalette::Text, Qt::white);
  newPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
  newPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
  newPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  newPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  newPalette.setColor(QPalette::ButtonText, Qt::white);
  newPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                  QColor(127, 127, 127));
  newPalette.setColor(QPalette::BrightText, Qt::red);
  newPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  newPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  newPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
  newPalette.setColor(QPalette::HighlightedText, Qt::white);
  newPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                   QColor(127, 127, 127));

  a.setPalette(newPalette);


  a.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

  { // dealing with command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    const QCommandLineOption helpOption = parser.addHelpOption();
    parser.addVersionOption();
    // An option with a value
    QCommandLineOption testOption({{ "t", "test"},
                                   "run test, where <testname> in {(s)erializer, (p)rocessAsServer}",
                                   "testname", "serializer"});
    parser.addOption(testOption);
    parser.process(a);

    if (!parser.positionalArguments().isEmpty() || !parser.optionNames().isEmpty()){
      QString testOptionName = parser.value(testOption);
      //running test for serializer
      if(testOptionName == "s" || testOptionName == "serializer"){
        // serialzer test
        sV2::TestAll testall;
        return 0;
      }
      if(testOptionName == "p" || testOptionName == "processAsServer"){
        test_ProcessAsServer();
        return 0;
      }

      qWarning() << parser.helpText();
      return 0;
    }
  }


  class EventFilter: public QObject{
  public: EventFilter():QObject(){}
    bool eventFilter(QObject *, QEvent* e){
      if((e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease)){
        auto keyEvent = dynamic_cast<QKeyEvent*>(e);
        if(keyEvent->key() == Qt::Key_Z){
          //qWarning() << "eventFilter Test" << e->type() << keyEvent->key();
        }
      }
      return false;
    }
  };

  a.installEventFilter(new EventFilter);

  // workaround: on Qt 6.4.1, windows, HDPI display with scaling discovered
  // that default font 9pt "Segoe UI" is not scaled properly (it is too large)
  // thus force it to 10pt which works fine:
#if defined WIN32 && QT_VERSION == QT_VERSION_CHECK(6, 4, 1)
  //qApp->setStyleSheet("QMenuBar,QMenu,QDockWidget,QTreeWidget{ font: 10pt \"Segoe UI\";}");
  qApp->setStyleSheet("*{ font: 10pt \"Segoe UI\";}");
#endif

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "t1_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    //QRect desktopGeometry = a.screenAt({0,0})->geometry();
    // w.setGeometry(
    //     QStyle::alignedRect(
    //         Qt::LeftToRight,
    //         Qt::AlignCenter,
    //         {int(desktopGeometry.width()*0.8),int(desktopGeometry.height()*0.8)},
    //         desktopGeometry
    //     )
    // );

    w.show();
    return a.exec();
}
