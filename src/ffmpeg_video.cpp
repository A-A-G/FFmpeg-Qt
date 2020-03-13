#include "ffmpeg_video.h"

extern "C"
{
  #include <libavcodec/avcodec.h>
  #include <libswscale/swscale.h>
  #include <libavformat/avformat.h>
  #include <libavutil/pixfmt.h>
  #include <libavutil/mathematics.h>
  #include <libavutil/imgutils.h>
}

#include <cfloat>
#include <iostream>

#include <QImage>
#include <QElapsedTimer>

#include <QDebug>

const AVPixelFormat AV_PIXEL_FORMAT = AV_PIX_FMT_RGB24;

void SaveFrame(AVFrame* pFrame, int width, int height, int iFrame)
{
  FILE* pFile;
  char szFilename[32];
  int  y;
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile = fopen(szFilename, "wb");

  if (pFile == nullptr)
  {
    return;
  }

  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);

  // Write pixel data
  for (y = 0; y < height; y++)
  {
    fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
  }

  // Close file
  fclose(pFile);
}

FFmpeg_Video::FFmpeg_Video()
  : _avFormatContext(nullptr),
    _avCodecContext(nullptr),
    _swsContext(nullptr),
    _frameBuffer(),
    _streamIndex(-1),
    _frametimer(0),
    _ptsOffset(-DBL_MAX),
    _eofReached(false)
{

}

FFmpeg_Video::FFmpeg_Video(const QString& videoPath)
  : _avFormatContext(nullptr),
    _avCodecContext(nullptr),
    _swsContext(nullptr),
    _frameBuffer(),
    _streamIndex(-1),
    _frametimer(0),
    _ptsOffset(-DBL_MAX),
    _eofReached(false)
{
  openVideoFile(videoPath);
}

FFmpeg_Video::~FFmpeg_Video()
{
  closeVideoFile();
  delete[] _frameBuffer.buffer;
  av_free(_frameBuffer.frame);
  sws_freeContext(_swsContext);
}

bool FFmpeg_Video::openVideoFile(const QString& videoPath)
{
  if (_avFormatContext != nullptr)
  {
    closeVideoFile();
  }

  if (avformat_open_input(&_avFormatContext, videoPath.toLatin1().data(), nullptr, nullptr) != 0)
  {
    qWarning("Couldn't open video file for this course");
    _avFormatContext = nullptr;
    return false;
  }

  if (avformat_find_stream_info(_avFormatContext, nullptr) < 0)
  {
    qWarning("Couldn't find stream information. Playback might fail!");
  }

  _streamIndex = av_find_best_stream(_avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

  if (_streamIndex == -1)
  {
    qWarning("Couldn't find a video stream.");
    closeVideoFile();
    return false;
  }
  
  av_dump_format(_avFormatContext, _streamIndex, videoPath.toLatin1().data(), 0);
  
  AVCodec* avCodec = avcodec_find_decoder(_avFormatContext->streams[_streamIndex]->codecpar->codec_id);
  
  if (avCodec == nullptr)
  {
    qWarning("Couldn't find a matching decoder.");
    closeVideoFile();
    return false;
  }
  
  _avCodecContext = avcodec_alloc_context3(avCodec);
  avcodec_parameters_to_context(_avCodecContext, _avFormatContext->streams[_streamIndex]->codecpar);

  if (avcodec_open2(_avCodecContext, avCodec, nullptr) < 0)
  {
    qWarning("Couldn't open codec.");
    closeVideoFile();
    return false;
  }

  _eofReached = false;
  _frametimer = 0;
  _ptsOffset = -DBL_MAX;
  return true;
}

void FFmpeg_Video::closeVideoFile()
{
  if (nullptr != _avCodecContext)
  {
    avcodec_close(_avCodecContext);
    avcodec_free_context(&_avCodecContext);
    _avCodecContext = nullptr;
  }

  if (nullptr != _avFormatContext)
  {
    avformat_close_input(&_avFormatContext);
    avformat_free_context(_avFormatContext);
    _avFormatContext = nullptr;
  }

  _streamIndex = -1;
}

// bool FFmpeg_Video::seek(int64_t frameNumber)
// {
//   if (nullptr == _avFormatContext)
//   {
//     qWarning() << "No AVFormatContext available.";
//     return false;
//   }
// 
//   if (avformat_seek_file(_avFormatContext, _streamIndex, frameNumber, frameNumber, frameNumber, AVSEEK_FLAG_FRAME) < 0)
//   {
//     qWarning("avformat_seek_file failed!");
//     return false;
//   }
// 
//   avcodec_flush_buffers(_avCodecContext);
//   return true;
// }

// AVPacket* FFmpeg_Video::readNextKeyPacket()
// {
//   AVPacket* packet = nullptr;
// 
//   while (!_eofReached)
//   {
//     packet = readNextPacket();
// 
//     if ((packet != nullptr) && (packet->flags & AV_PKT_FLAG_KEY))
//     {
//       break;
//     }
// 
//     av_packet_unref(packet);
//   }
// 
//   avcodec_flush_buffers(_avCodecContext);
//   return packet;
// }

AVPacket* FFmpeg_Video::readNextPacket()
{
  if (nullptr == _avFormatContext)
  {
    qWarning() << "No AVFormatContext available.";
    return nullptr;
  }

  if (_eofReached)
  {
    return nullptr;
  }

  AVPacket* packet = av_packet_alloc();
  int readFrameResult = av_read_frame(_avFormatContext, packet);

  if (readFrameResult < 0)
  {
    //error or end of file
    if (readFrameResult == (int)AVERROR_EOF)
    {
//       qWarning() << "End of video file";
      _eofReached = true;
    }
    else
    {
      qWarning("AVError %i while reading next packet.", readFrameResult);
    }

    return nullptr;
  }

  if (packet->stream_index != _streamIndex)
  {
    av_packet_unref(packet);
    av_packet_free(&packet);
    return readNextPacket();
  }

  return packet;
}

// AVFrame* FFmpeg_Video::readNextFrame()
// {
//   AVPacket* packet = readNextPacket();
//   AVFrame* frame = readNextFrame(packet);
//   av_packet_unref(packet);
//   av_packet_free(&packet);
//   return frame;
// }

int frameCounter = 0;
double frameTime = 0;

AVFrame* FFmpeg_Video::readNextFrame(AVPacket* packet)
{
  if (nullptr == packet)
  {
    qWarning() << "No AVPacket available.";
    return nullptr;
  }

  AVFrame* frame = av_frame_alloc();
  int frameDecoded = 0;
  
  QElapsedTimer timer;

  timer.start();
  frameDecoded = avcodec_send_packet(_avCodecContext, packet);
  double ft = timer.elapsed();
  int response = avcodec_receive_frame	(_avCodecContext, frame);
  frameDecoded = frameDecoded + response;
//   std::cout << response << std::endl;
//   while(response>=0) // just testing
//   {
//     AVFrame* f = av_frame_alloc();
//     response = avcodec_receive_frame(_avCodecContext, f);
//     if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) 
//     {
//       std::cout << "Nothing more :)" << std::endl;
//       break;
//     }
//     else if (response < 0) 
//     {
//       std::cout << "Error while receiving a frame from the decoder: " << std::endl;// << av_err2str(response);
//       break;
//     }
//     if (response >= 0) 
//     {
//       std::cout << "Got another frame! Sad!" << std::endl;
//     }
//   }
  frameTime = frameTime + ft;
  ++frameCounter;
//   qDebug() << "FFmpeg_Video avcodec_decode_video2() : " << ft << " Average: " << frameTime/(++frameCounter);
//   qDebug() << _avCodecContext->hwaccel << _avCodecContext->hwaccel_context;

  if (0 == frameDecoded)
  {
    //check for special formats (no workaround yet implemented)
    AVFrame* dump = av_frame_alloc();
    if (AVERROR(EAGAIN) != avcodec_receive_frame	(_avCodecContext, dump))
    {
      qDebug() << "Something wrong with video format!";
    }
    av_free(dump);
    return frame;
  }
  else
  {
    qWarning() << "Frame not decoded.";
    return nullptr;
  }
}

double fullTime = 0;
int fullCounter = 0;

FFmpeg_Video::PTS_QImage* FFmpeg_Video::getNextPBFrame(int width, int height, int decodingFactor)
{
  QElapsedTimer t;
  t.start();
  AVPacket* packet = readNextPacket();

  if (nullptr == packet)
  {
    if (_eofReached)
    {
      return nullptr;
    }
    else
    {
      return getNextPBFrame(width, height, decodingFactor);
    }
  }

  AVFrame* frame = readNextFrame(packet);
  av_packet_unref(packet);
  av_packet_free(&packet);

  if (nullptr == frame)
  {
    if (_eofReached)
    {
      return nullptr;
    }
    else
    {
      return getNextPBFrame(width, height, decodingFactor);
    }
  }

  if ((decodingFactor > 1) && (!_eofReached))
  {
    _frametimer = _frametimer + 1 / av_q2d(_avFormatContext->streams[_streamIndex]->avg_frame_rate);
    av_frame_free(&frame);
    return getNextPBFrame(width, height, decodingFactor - 1);
  }

  QImage* image = getQImage(frame, width, height);
  int64_t dts = frame->pkt_dts;
  av_frame_free(&frame);

  if (nullptr == image)
  {
    if (_eofReached)
    {
      return nullptr;
    }
    else
    {
      return getNextPBFrame(width, height, 1);
    }
  }

  PTS_QImage* pbFrame = new PTS_QImage();
  pbFrame->qImage = image;

  if ((int64_t)AV_NOPTS_VALUE == dts)
  {
    pbFrame->pts = _frametimer;
  }
  else
  {
    pbFrame->pts = dts * av_q2d(_avFormatContext->streams[_streamIndex]->time_base);
  }

  if (-DBL_MAX == _ptsOffset)
  {
    _ptsOffset = pbFrame->pts;
  }

  pbFrame->pts = pbFrame->pts - _ptsOffset;
  _frametimer = _frametimer + 1 / av_q2d(_avFormatContext->streams[_streamIndex]->avg_frame_rate);
  double ft = t.elapsed();
  fullTime = fullTime + ft;
  fullCounter++;
  return pbFrame;
}

// QImage* FFmpeg_Video::getNextQImage(uint width, uint height)
// {
//   AVFrame* nextFrame = readNextFrame();
//   QImage* image = getQImage(nextFrame, width, height);
//   av_free(nextFrame);
//   return image;
// }

// QImage* FFmpeg_Video::getQImage(double timeS, uint width, uint height)
// {
//   int frameNumber = av_rescale(timeS * 1000 , _avFormatContext->streams[_streamIndex]->time_base.den, _avFormatContext->streams[_streamIndex]->time_base.num) / 1000;
//   seek(frameNumber);
//   return getNextQImage(width, height);
// }

QImage* FFmpeg_Video::getQImage(AVFrame* frame, uint width, uint height)
{
  if (nullptr == frame)
  {
    qWarning("getQImage: Got nullptr frame...");
    return nullptr;
  }

  if ((_frameBuffer.height != height) || (_frameBuffer.width != width))
  {
    _frameBuffer.width = width;
    _frameBuffer.height = height;
    allocateFrame();
  }

  convertFrame(frame, _frameBuffer.frame, width, height);
  return getQImage(_frameBuffer.buffer, _frameBuffer.headerLength, width, height);
}

QImage* FFmpeg_Video::getQImage(uint8_t* frameBuffer, int len, uint width, uint height)
{
  if (nullptr == frameBuffer)
  {
    qWarning("getQImage: Didn't get data.");
    return nullptr;
  }

  QImage* qImage = new QImage();
  qImage->loadFromData(frameBuffer, len + width * height * 3, "PPM");
  return qImage;
}

double swsTime = 0;
int swsCounter = 0;

bool FFmpeg_Video::convertFrame(AVFrame* srcFrame, AVFrame* destFrame, uint width, uint height)
{
  if ((nullptr == srcFrame) || (nullptr == destFrame))
  {
    qWarning("convertFrame: Got nullptr frame...");
    return false;
  }

  _swsContext = sws_getCachedContext(_swsContext, _avCodecContext->width, _avCodecContext->height, _avCodecContext->pix_fmt, width, height, AV_PIXEL_FORMAT, SWS_BICUBIC, nullptr, nullptr, nullptr);

  if (nullptr == _swsContext)
  {
    qWarning("Cannot initialize the conversion context!");
    return false;
  }
  QElapsedTimer timer;
  timer.start();
  sws_scale(_swsContext, srcFrame->data, srcFrame->linesize, 0, _avCodecContext->height, destFrame->data, destFrame->linesize);
  double st = timer.elapsed();
  swsTime = swsTime + st;
  ++swsCounter;
//   qDebug() << "FFmpeg_Video sws_scale() : " << st << " Average: " << swsTime/(++swsCounter);
  return true;
}

void FFmpeg_Video::printTimes()
{
  qDebug() << "getNextPBFrame() : " << fullTime/fullCounter << " avcodec_decode_video2() : " << frameTime/frameCounter << " sws_scale() Average: " << swsTime/swsCounter;
}

void FFmpeg_Video::allocateFrame()
{
  if (nullptr != _frameBuffer.buffer)
  {
    delete[] _frameBuffer.buffer; _frameBuffer.buffer = nullptr;
  }

  int numBytes = av_image_get_buffer_size(AV_PIXEL_FORMAT, _frameBuffer.width, _frameBuffer.height, 1);
  _frameBuffer.buffer = new uint8_t[numBytes + 64];
  _frameBuffer.headerLength = sprintf((char*)_frameBuffer.buffer, "P6\n%d %d\n255\n", _frameBuffer.width, _frameBuffer.height);
_frameBuffer.frame = av_frame_alloc();
  av_image_fill_arrays(_frameBuffer.frame->data, _frameBuffer.frame->linesize, _frameBuffer.buffer + _frameBuffer.headerLength, AV_PIXEL_FORMAT, _frameBuffer.width, _frameBuffer.height, 1);
}

bool FFmpeg_Video::videoFileIsOpen() const
{
  return ((nullptr != _avFormatContext) && (nullptr != _avCodecContext));
}

bool FFmpeg_Video::eofReached() const
{
  return _eofReached;
}
