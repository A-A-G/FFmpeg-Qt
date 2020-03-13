#include "ffmpeg.h"

#include "pbvideowidget.h"
#include "ffmpeg_video.h"

#include <QTimer>

#include <QDebug>

using namespace PowerBike;

PBVideoWidget* pbvideo;
FFmpeg_Video* videoHandler;

int counter = 0;
int timer = 20000;
int speed = 1;

FFmpeg::FFmpeg(QWidget *parent) :
    QMainWindow(parent)
{
  this->showMaximized();
  pbvideo = new PBVideoWidget(this);
  setCentralWidget(pbvideo);
  videoHandler = new FFmpeg_Video();
  videoHandler->openVideoFile("/home/alex/projects/powerbike/PowerbikeSoftware/test_data/videos/PowerbikeDemo1_maxQuality.mp4");
//   videoHandler->openVideoFile("/home/alex/projects/powerbike/PowerbikeSoftware/test_data/videos/allensbach-langenrain_video.mp4");
  pbvideo->setVideoHandler(videoHandler);
  pbvideo->play();
  //QTimer::singleShot(timer, this, &FFmpeg::sendTime);
  QTimer::singleShot(timer, this, SLOT(sendTime()));
}

void FFmpeg::sendTime()
{
  pbvideo->stop();
  pbvideo = new PBVideoWidget(this);
  setCentralWidget(pbvideo);
  if (0 == counter)
  {
    qDebug() << "maxQuality";
    videoHandler->printTimes();
    videoHandler = new FFmpeg_Video();
    videoHandler->openVideoFile("/home/alex/projects/powerbike/PowerbikeSoftware/test_data/videos/PowerbikeDemo1_highQuality.mp4");
    pbvideo->setVideoHandler(videoHandler);
    pbvideo->play();
    //QTimer::singleShot(timer, this, &FFmpeg::sendTime);
    QTimer::singleShot(timer, this, SLOT(sendTime()));
  }
  else if (1 == counter)
  {
    qDebug() << "highQuality";
    videoHandler->printTimes();
    videoHandler = new FFmpeg_Video();
    videoHandler->openVideoFile("/home/alex/projects/powerbike/PowerbikeSoftware/test_data/videos/PowerbikeDemo1_mediumQuality.mp4");
    pbvideo->setVideoHandler(videoHandler);
    pbvideo->play();
    //QTimer::singleShot(timer, this, &FFmpeg::sendTime);
    QTimer::singleShot(timer, this, SLOT(sendTime()));
  }
  else if (2 == counter)
  {
    qDebug() << "mediumQuality";
    videoHandler->printTimes();
    videoHandler = new FFmpeg_Video();
    videoHandler->openVideoFile("/home/alex/projects/powerbike/PowerbikeSoftware/test_data/videos/PowerbikeDemo1_lowQuality.mp4");
    pbvideo->setVideoHandler(videoHandler);
    pbvideo->play();
    //QTimer::singleShot(timer, this, &FFmpeg::sendTime);
    QTimer::singleShot(timer, this, SLOT(sendTime()));
  }
  else if (3 == counter)
  {
    qDebug() << "lowQuality";
    videoHandler->printTimes();
  }
  counter++;
//   pbvideo->newTime(speed*counter++*timer/1000.0, 0);
}

FFmpeg::~FFmpeg()
{
  delete pbvideo;
}
