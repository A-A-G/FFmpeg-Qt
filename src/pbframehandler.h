#ifndef PBFRAMEHANDLER_H
#define PBFRAMEHANDLER_H

#include <QObject>

#include <QImage>

class QThread;

namespace PowerBike
{
  
class PBVideoDescriptor;
class PBDecoderThread;

class PBFrameHandler : public QObject
{
  Q_OBJECT
  
  public:
    explicit PBFrameHandler(PBVideoDescriptor* videoDescriptor);
    virtual ~PBFrameHandler();
    QImage getCurrentImage() const;
    bool isPlaying() const;
   
  public slots:
    void play();
    void stop();
    void newTime(double timeTCX, double timeERGO);
    
  signals:
    void stopped();
    void newImage();

  private slots:
    void refreshVideo();
    
  private:
   void scheduleRefresh(int delay);
    QThread* _thread;
    PBVideoDescriptor *_videoDescriptor;
    PBDecoderThread *_decoderThread;
    QImage _curImage;
    double getNormalClock();
    double _lastTimeTCX;
    double _lastTimeTCXDifference;
    double _lastUpdateTime;
    double _lastTimeERGODifference;
    double _lastTimeERGO;
    bool _isPlaying;
    bool _stopped;
  
};

}
#endif // PBFRAMEHANDLER_H
