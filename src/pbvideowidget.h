#ifndef PBVIDEOWIDGET_H
#define PBVIDEOWIDGET_H

#include <QWidget>
#include <QImage>

class VideoHandler;

namespace PowerBike
{

class PBVideoDescriptor;
class PBFrameHandler;

class PBVideoWidget : public QWidget
{
  Q_OBJECT
  
  public:
    PBVideoWidget(QWidget* parent = nullptr);
    ~PBVideoWidget();
    void setVideoHandler(VideoHandler* video);
    void closeVideoFile();
    QSize sizeHint() const;
    bool videoFinished() const;
   
  public slots:
    void play();
    void stop();
    void newTime(double timeTCX, double timeERGO);
    
  signals:
    void stopped();

  protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    
  private:
    PBVideoDescriptor* _videoDescriptor;
    PBFrameHandler* _frameHandler;
    QImage _lastImage;

};

}
#endif