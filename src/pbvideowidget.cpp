#include "pbvideowidget.h"

#include "pbvideodescriptor.h"
#include "pbframehandler.h"

#include "ffmpeg_video.h"
// #include "../../../util/video/opencv_video.h"

#include <QPainter>
#include <QResizeEvent>
#include <QTimer>
#include <QApplication>

#include <QDebug>

#include <cmath>

namespace PowerBike
{

PBVideoWidget::PBVideoWidget(QWidget* parent)
  :QWidget(parent),
  _videoDescriptor(new PBVideoDescriptor()),
  _frameHandler(new PBFrameHandler(_videoDescriptor))
{
  connect(_frameHandler, SIGNAL(stopped()), this, SIGNAL(stopped()));
  connect(_frameHandler, SIGNAL(newImage()), this, SLOT(update()));
  _videoDescriptor->setVideoSize(size());
}

PBVideoWidget::~PBVideoWidget()
{
  delete _frameHandler;
  delete _videoDescriptor;
}

void PBVideoWidget::setVideoHandler(VideoHandler* video)
{
  if (nullptr != _videoDescriptor->video)
  {
    delete _videoDescriptor->video;
  }
  _videoDescriptor->video = video;
}

void PBVideoWidget::closeVideoFile()
{
  _videoDescriptor->video->closeVideoFile();
  _videoDescriptor->setShouldStop(true);
  if (_frameHandler->isPlaying())
  {
    _videoDescriptor->queueNotFull->wakeAll();
  }
}

void PBVideoWidget::play()
{
  _frameHandler->play();
}

void PBVideoWidget::stop()
{
  _frameHandler->stop();
}

void PBVideoWidget::paintEvent(QPaintEvent *event)
{
  if (_frameHandler->getCurrentImage() != _lastImage)
  {
    _lastImage = _frameHandler->getCurrentImage();
  }
  if ((!_lastImage.isNull())&&(_lastImage.size().width() != size().width() || _lastImage.size().height() != size().height()))
  {
    _lastImage = _lastImage.scaled(size().width(), size().height());
  }
  QPainter painter(this);
  QPoint point(0,0);
  painter.drawImage(point, _lastImage);
}

void PBVideoWidget::newTime(double timeTCX, double timeERGO)
{
  _frameHandler->newTime(timeTCX, timeERGO);
}

void PBVideoWidget::resizeEvent(QResizeEvent *event)
{ 
  _videoDescriptor->setVideoSize(size());
}

QSize PBVideoWidget::sizeHint() const
{
  return size();
}

bool PBVideoWidget::videoFinished() const
{
  return _videoDescriptor->videoFinished;
}

}
#include "moc_pbvideowidget.cpp"
