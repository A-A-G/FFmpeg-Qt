#ifndef FFMPEG_VIDEO_H
#define FFMPEG_VIDEO_H

#include "videohandler.h"

#include <QString>

struct AVPacket;
class QImage;

class SwsContext;
struct AVRational;
class AVFrame;
class AVFormatContext;
class AVCodecContext;
class AVCodec;

class FFmpeg_Video : public VideoHandler
{
  public: 
    FFmpeg_Video();
    explicit FFmpeg_Video(const QString& videoPath);
    virtual ~FFmpeg_Video();
    virtual bool openVideoFile(const QString& videoPath);
    virtual void closeVideoFile();
    virtual bool videoFileIsOpen() const;
    bool eofReached() const;
//     bool seek(int64_t frameNumber);
//     AVPacket* readNextKeyPacket();
    AVPacket* readNextPacket();
//     AVFrame* readNextFrame();
    AVFrame* readNextFrame(AVPacket* packet);
//     QImage* getQImage(double timeS, uint width, uint height);
//     QImage* getNextQImage(uint width, uint height);
    QImage* getQImage(AVFrame* frame, uint width, uint height);
    static QImage* getQImage(uint8_t* frameBuffer, int len, uint width, uint height);
    virtual PTS_QImage* getNextPBFrame(int width, int height, int decodingFactor = 1);
    void printTimes();
    
  private:
    
    struct FrameBuffer
    {
      AVFrame* frame = nullptr;
      uint8_t* buffer = nullptr;
      uint width = 0;
      uint height = 0;
      int headerLength = 0;
    };
    
    AVFormatContext* _avFormatContext;
    AVCodecContext* _avCodecContext;
    SwsContext* _swsContext;
    FrameBuffer _frameBuffer;
    int _streamIndex;
    double _frametimer;
    double _ptsOffset;
    bool _eofReached;
    bool convertFrame(AVFrame* srcFrame, AVFrame* destFrame, uint width, uint height);
    void allocateFrame();
    
};

#endif // FFMPEG_VIDEO_H
