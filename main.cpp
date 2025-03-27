#include "WallPicture.h"
#include <QApplication>
#include <QSplashScreen>

#include "style.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QPixmap(":/WallPicture/picture.png"));

    QSplashScreen splash(QPixmap(":/WallPicture/picture.png"));
    splash.setStyleSheet(WP::splashStyle());
    splash.show();
    splash.showMessage("Загрузка...", Qt::AlignCenter);
    app.processEvents();

    std::function<void(int)> fn = [&splash, &app](int percent)
        {
            QString str = QString("Загрузка: %1%").arg(percent);
            splash.showMessage(str, Qt::AlignCenter);
            app.processEvents();
        };

    WP::WallPicture w(&fn);
    w.showFullScreen();

    splash.finish(&w);
    return app.exec();
}
