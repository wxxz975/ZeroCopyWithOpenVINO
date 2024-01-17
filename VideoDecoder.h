#pragma once

#include <string>
#include <thread>

extern "C" {
    #include <libswscale/swscale.h>
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/hwcontext.h>
    #include <libavutil/hwcontext_vaapi.h>
}

#include "BlockingQueue.h"


struct VideoInfo
{
    std::size_t width = 0;
    std::size_t height = 0;
    std::size_t channels = 0;
};

class VideoDecoder
{
public:

    explicit VideoDecoder(const std::string& video_path);
    ~VideoDecoder();

    AVFrame* ReadFrame();

    void DeleteFrame(AVFrame* frame);

    const VideoInfo& GetVideoInfo();

    VASurfaceID GetSurfaceID(AVFrame* frame);

    void* GetDisplay();

    inline bool IsRunning() const { return isRunning; };

    inline auto GetQueueSize() {
        return decoded_frames.size();
    }
private:
    bool Init(const std::string& video_path);

    void Run();

    void Stop();

private:
    AVFormatContext *avformat_ctx = nullptr;
    AVCodec *avcodec = nullptr;
    AVCodecParameters *avcodecpar = nullptr;
    AVCodecContext *avdecoder_ctx = nullptr;
    int video_stream_num = -1;


    BlockingQueue<AVFrame* > decoded_frames;
    int max_frame_store = 10; // 设置队列最多保存多少帧数据

    volatile bool isRunning = true;
    std::thread decodeThread;

    VideoInfo vInfo; // 视频的宽高信息
};  