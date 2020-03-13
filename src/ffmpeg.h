#ifndef FFMPEG_H
#define FFMPEG_H

#include <QMainWindow>

class FFmpeg : public QMainWindow
{
    Q_OBJECT

public:
    explicit FFmpeg(QWidget *parent = nullptr);
    ~FFmpeg();

private slots:
    void sendTime();
};

#endif // FFMPEG_H
