#ifndef PBVIDEODESCRIPTOR_H
#define PBVIDEODESCRIPTOR_H

#include "videohandler.h"

#include <QSize>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QImage>

#include <QDebug>

namespace PowerBike
{

class PBVideoDescriptor
{
  public:
    VideoHandler* video;
    QQueue<VideoHandler::PTS_QImage*> queue;
    QMutex* queueMutex;
    QWaitCondition* queueNotEmpty;
    QWaitCondition* queueNotFull;
    QMutex mutex;
    int decodingFactor;
    bool videoFinished;
    
    const static int IMAGE_BUFFER_SIZE = 100;
    
  private:
    QSize _videoSize;
    bool _shouldStop;

  public:
    PBVideoDescriptor()
    :video(nullptr),
    queue(),
    queueMutex(new QMutex()),
    queueNotEmpty(new QWaitCondition()),
    queueNotFull(new QWaitCondition()),
    mutex(),
    decodingFactor(1),
    videoFinished(false),
    _videoSize(),
    _shouldStop(false){}
    
    ~PBVideoDescriptor()
    {
      queueNotEmpty->wakeAll();
      delete queueNotEmpty;
      queueNotFull->wakeAll();
      delete queueNotFull;
      queueMutex->unlock();
      delete queueMutex;
      while(!queue.empty())
      {
				VideoHandler::PTS_QImage* frame = queue.dequeue();
				delete frame->qImage; frame->qImage = nullptr;
				delete frame;
      }
      delete video;
    }
	
    QSize getVideoSize()
    {
      return _videoSize;
    }
    
    void setVideoSize(const QSize& size)
    {
      QMutexLocker locker(&mutex);
      _videoSize = size;
    }

    void setShouldStop(bool shouldStop)
    {
      QMutexLocker locker(&mutex);
      _shouldStop = shouldStop;
    }
    
    bool getShouldStop()
    {
      return _shouldStop;
    }
    
    void setDecodingFactor(int factor)
    {
      decodingFactor = factor;	  
    }
};

}
#endif
