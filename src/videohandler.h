#ifndef VIDEO_H
#define VIDEO_H

#include <QString>

class QImage;

class VideoHandler
{
  public:
    
    struct PTS_QImage
    {
      QImage *qImage;
      double pts;
    };
    
    enum Decoder
    {
      NONE,
      FFmpeg,
      OpenCV
    };
    
    virtual ~VideoHandler() = 0;
    virtual bool videoFileIsOpen() const = 0;
    virtual void closeVideoFile() = 0;
    virtual bool eofReached() const = 0;
    virtual bool openVideoFile(const QString& videoPath) = 0;
    virtual PTS_QImage* getNextPBFrame(int width, int height, int decodingFactor = 1) = 0;

};

inline VideoHandler::~VideoHandler() { }

#endif // VIDEO_H
