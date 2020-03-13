#include "pbframehandler.h"

#include "pbdecoderthread.h"
#include "pbvideodescriptor.h"
#include "videohandler.h"

#include <QTimer>
#include <QImage>

#include <cmath>

#include <QDebug>

namespace PowerBike
{

PBFrameHandler::PBFrameHandler(PBVideoDescriptor* videoDescriptor)
  : _thread(new QThread(this)),
    _videoDescriptor(videoDescriptor),
    _decoderThread(nullptr),
    _curImage(),
    _lastTimeTCX(0),
    _lastTimeTCXDifference(0),
    _lastUpdateTime(0),
    _lastTimeERGODifference(0),
    _lastTimeERGO(0),
    _isPlaying(false),
    _stopped(false)
{
  this->moveToThread(_thread);
  _thread->start();
}

PBFrameHandler::~PBFrameHandler()
{
  while (!_thread->isFinished())
  {
    _thread->exit();
  }

  delete _decoderThread;
}

void PBFrameHandler::play()
{
  if ((nullptr != _videoDescriptor->video) && (_videoDescriptor->video->videoFileIsOpen()))
  {
    _decoderThread = new PBDecoderThread(_videoDescriptor);
    _decoderThread->start();
    _isPlaying = true;
    refreshVideo();
  }
  else
  {
    stop();
  }
}

void PBFrameHandler::stop()
{
  if (!_stopped)
  {
    emit stopped();
    _videoDescriptor->setShouldStop(true);
    _stopped = true;
  }
}

double PBFrameHandler::getNormalClock()
{
//   return (_lastTimeTCX + (((MainWindow::getApplicationTimer().nsecsElapsed() / 1000000000.0 - _lastUpdateTime) / _lastTimeERGODifference) * _lastTimeTCXDifference));
  return _lastTimeTCX;
}

void PBFrameHandler::refreshVideo()
{
  if (_videoDescriptor->video->eofReached() && _videoDescriptor->queue.empty())
  {
    _videoDescriptor->videoFinished = true;
    _videoDescriptor->setShouldStop(true);
  }

  if (_videoDescriptor->getShouldStop())
  {
    stop();
    return;
  }

//   if (0 == _lastTimeTCXDifference)
//   {
//     scheduleRefresh(5);
//     return;
//   }

  while (_videoDescriptor->queue.empty())
  {
    _videoDescriptor->queueMutex->lock();
    _videoDescriptor->queueNotEmpty->wait(_videoDescriptor->queueMutex);
    _videoDescriptor->queueMutex->unlock();
  }

  VideoHandler::PTS_QImage* frame = _videoDescriptor->queue.dequeue();
  _videoDescriptor->queueNotFull->wakeAll();
  double actualDelay = frame->pts - getNormalClock();

//   if (_videoDescriptor->queue.size() < PBVideoDescriptor::IMAGE_BUFFER_SIZE * 0.9)
//   {
//     _videoDescriptor->setDecodingFactor((int)(5 * (PBVideoDescriptor::IMAGE_BUFFER_SIZE * 0.9 - _videoDescriptor->queue.size()) / (double)PBVideoDescriptor::IMAGE_BUFFER_SIZE));
//   }
//   else
//   {
//     _videoDescriptor->setDecodingFactor(1);
//   }
//   qDebug() << actualDelay;
  if (actualDelay < 0)
  {
    actualDelay = 0;
  }

  _curImage = QImage(*(frame->qImage));
  delete frame->qImage; delete frame;
  emit newImage();

  if (!_stopped)
  {
//     scheduleRefresh(int (actualDelay * 1000));
    scheduleRefresh(20);
  }
}

void PBFrameHandler::newTime(double timeTCX, double timeERGO)
{
  if (_isPlaying)
  {
//     _lastUpdateTime = MainWindow::getApplicationTimer().nsecsElapsed() / 1000000000.0;
    _lastTimeERGODifference = timeERGO - _lastTimeERGO;
    _lastTimeERGO = timeERGO;
    _lastTimeTCXDifference = timeTCX - _lastTimeTCX;
    _lastTimeTCX = timeTCX;
  }
}

QImage PBFrameHandler::getCurrentImage() const
{
  return _curImage;
}

void PBFrameHandler::scheduleRefresh(int delay)
{
  QTimer::singleShot(delay, this, SLOT(refreshVideo()));
}

bool PBFrameHandler::isPlaying() const
{
  return _isPlaying;
}

}
#include "moc_pbframehandler.cpp"
