#ifndef PBDECODERTHREAD_H
#define PBDECODERTHREAD_H

#include <QThread>

struct AVFrame;
class QImage;
class QSize;

namespace PowerBike
{

class PBVideoDescriptor;

class PBDecoderThread : public QThread
{  
  public:
    PBDecoderThread(PBVideoDescriptor* pbVideoDescriptor);
    ~PBDecoderThread();
    void run();
    
  private:
    bool convertFrame(int widthDest, int heightDest, AVFrame* frame);
    QImage* createQImage(const QSize& videoSize);
    PBVideoDescriptor *_videoDescriptor;
};

}
#endif

