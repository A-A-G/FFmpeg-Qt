#include "pbdecoderthread.h"

#include "pbvideodescriptor.h"

#include <QSize>
#include <QImage>
#include <QElapsedTimer>

#include <QDebug>

namespace PowerBike
{
  
PBDecoderThread::PBDecoderThread(PBVideoDescriptor* pbVideoDescriptor)
  :_videoDescriptor(pbVideoDescriptor)
{
}

PBDecoderThread::~PBDecoderThread()
{
  _videoDescriptor->queueMutex->unlock();
  while(this->isRunning())
  {
    this->exit();
  }
}

void PBDecoderThread::run()
{
  forever
  {
    _videoDescriptor->queueMutex->lock();
    if((_videoDescriptor->queue.size() >= PBVideoDescriptor::IMAGE_BUFFER_SIZE) && !_videoDescriptor->getShouldStop())
    {
      _videoDescriptor->queueNotFull->wait(_videoDescriptor->queueMutex);
    }
    _videoDescriptor->queueMutex->unlock();
    if(_videoDescriptor->getShouldStop())
    {
      return;
    }
//     QElapsedTimer timer;
//     timer.start();
    VideoHandler::PTS_QImage* frame = _videoDescriptor->video->getNextPBFrame(_videoDescriptor->getVideoSize().width(), _videoDescriptor->getVideoSize().height(), _videoDescriptor->decodingFactor);
//     qDebug() << "PBDecoderThread getNextPBFrame() : " << timer.elapsed();
//     delete frame;
    if (frame != nullptr)
    {
      _videoDescriptor->queue.enqueue(frame);
      _videoDescriptor->queueNotEmpty->wakeAll();
    }
    msleep(5);
  }
}

}
