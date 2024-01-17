#include "VideoDecoder.h"
#include <stdexcept>



VideoDecoder::VideoDecoder(const std::string& video_path)
{
    if(!Init(video_path))
        throw std::runtime_error("Failed to init ffmpeg context!\n");
    
    decodeThread = std::thread(&VideoDecoder::Run, this);
    decodeThread.detach();
}


VideoDecoder::~VideoDecoder()
{
    if(isRunning)
        decodeThread.join();
}

AVFrame* VideoDecoder::ReadFrame()
{
    return decoded_frames.pop();  // 这里可能会阻塞的
}

void VideoDecoder::DeleteFrame(AVFrame *frame)
{
    av_frame_free(&frame);
}


const VideoInfo& VideoDecoder::GetVideoInfo()
{
    if(!vInfo.height || !vInfo.width || !vInfo.channels) {
        vInfo.width = avformat_ctx->streams[video_stream_num]->codecpar->width;
        vInfo.height = avformat_ctx->streams[video_stream_num]->codecpar->height;
        vInfo.channels = 3;
    }

    return vInfo;
}

VASurfaceID VideoDecoder::GetSurfaceID(AVFrame *frame)
{
    return (VASurfaceID)(std::size_t)frame->data[3];
}

void* VideoDecoder::GetDisplay()
{
    if(!avdecoder_ctx->hw_device_ctx) {
        throw std::runtime_error("hw_device_ctx is Not Available\n");
    }

    AVHWDeviceContext *device_ctx = (AVHWDeviceContext *)avdecoder_ctx->hw_device_ctx->data;
    if(device_ctx->type != AV_HWDEVICE_TYPE_VAAPI){
        throw std::runtime_error("device_ctx->type != AV_HWDEVICE_TYPE_VAAPI \n");
    }

    AVVAAPIDeviceContext *vaapi_ctx = (AVVAAPIDeviceContext *)device_ctx->hwctx;
    if(!vaapi_ctx) {
        throw std::runtime_error("vaapi_ctx == nullptr \n");
    }

    return vaapi_ctx->display;
}

bool VideoDecoder::Init(const std::string &video_path)
{
    const char *device = nullptr;
    avformat_ctx = avformat_alloc_context();
    if(!avformat_ctx)
        return false;
        
    if(avformat_open_input(&avformat_ctx, video_path.c_str(), nullptr, nullptr) < 0 ) {
        printf("failed open input file!\n");
        //av_log(nullptr, AV_LOG_ERROR, "Failed to open input file: %s\n", video_path.c_str());
        avformat_free_context(avformat_ctx);
        return false;
    }

    video_stream_num = av_find_best_stream(avformat_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &avcodec, 0);
    if(video_stream_num < 0) {
        printf("failed to find video stream!\n");
        goto ERREND;
    }
    
    avcodecpar = avformat_ctx->streams[video_stream_num]->codecpar;
    if(!avcodecpar) {
        printf("failed to find avcodecpar:%p!\n", avcodecpar);
        goto ERREND;
    }
    
    avdecoder_ctx = avcodec_alloc_context3(avcodec);
    if(!avdecoder_ctx){
        printf("failed to alloc avdecoder_ctx!\n");
        goto ERREND;
    }
        

    if(avcodec_parameters_to_context(avdecoder_ctx, avcodecpar) < 0) {
        printf("failed to parameters_to_context!\n");
        goto ERREND;
    }   
        
    
    if(av_hwdevice_ctx_create(&avdecoder_ctx->hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, device, nullptr, 0) < 0) {
        printf("failed to create hwdevice ctx!\n");
        goto ERREND;
    }
        

    
    avdecoder_ctx->get_format = [](AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
        return AV_PIX_FMT_VAAPI; // request VAAPI frame format
    };

    if(avcodec_open2(avdecoder_ctx, avcodec, nullptr) < 0) {
        printf("failed to avcodec open2!\n");
        goto ERREND;
    }
        
    printf("[*] totoal frames:%ld\n", avformat_ctx->streams[video_stream_num]->nb_frames) ;

    goto SUCCESSEND;

ERREND:
    avcodec_free_context(&avdecoder_ctx);
    avformat_close_input(&avformat_ctx);
    avformat_free_context(avformat_ctx);
    return false;

SUCCESSEND:
    return true;
}

void VideoDecoder::Run()
{
    while (isRunning)
    {
        AVPacket *avpacket = av_packet_alloc();
        if (av_read_frame(avformat_ctx, avpacket) < 0) {
            av_packet_free(&avpacket); // End of stream or error. Send NULL to avcodec_send_packet once to flush decoder
        } else if (avpacket->stream_index != video_stream_num) {
            av_packet_free(&avpacket); // Non-video (ex, audio) packet
            continue;
        }

        if(avcodec_send_packet(avdecoder_ctx, avpacket) < 0)
            throw std::runtime_error("failed to decode the packet!");

         while (true)
        {
            auto av_frame = av_frame_alloc();
            int decode_err = avcodec_receive_frame(avdecoder_ctx, av_frame);
            if (decode_err == AVERROR(EAGAIN) || decode_err == AVERROR_EOF) {
                break;
            }

            if(decode_err < 0) 
                throw std::runtime_error("failed to receive frame!");
            
            if (av_frame->format != AV_PIX_FMT_VAAPI)
                throw std::runtime_error("Unsupported av_frame format");


            decoded_frames.push(av_frame, max_frame_store);
        }

        if (avpacket)
            av_packet_free(&avpacket);
        else
            break; // End of stream
    }
    
    Stop();
}

void VideoDecoder::Stop()
{
    isRunning = false;
    printf("stop decoder!\n");
}
