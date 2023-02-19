#include "CommonComponents/mainwindow.h"
#include "apputil/serializerV2test.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyle>
#include <QScreen>

int main(int argc, char *argv[])
{

#if !serializerTest
  sV2::TestAll testall;
  return 0;
#endif

  //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//  QGuiApplication::setAttribute(Qt::AA_Use96Dpi);
//  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
//        Qt::HighDpiScaleFactorRoundingPolicy::Round);

//  QFont font("Courier New");
//  font.setStyleHint(QFont::Monospace);
//  QApplication::setFont(font);

  QApplication a(argc, argv);

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
    QRect desktopGeometry = a.screenAt({0,0})->geometry();
    w.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            {int(desktopGeometry.width()*0.8),int(desktopGeometry.height()*0.8)},
            desktopGeometry
        )
    );

    w.show();
    return a.exec();
}
