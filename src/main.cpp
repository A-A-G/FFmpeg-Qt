#include "ffmpeg.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FFmpeg w;
    w.show();

    return app.exec();
}

