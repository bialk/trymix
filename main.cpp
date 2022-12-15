#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyle>
#include <QDesktopWidget>
#include <QScreen>

int main(int argc, char *argv[])
{
  //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication a(argc, argv);

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
