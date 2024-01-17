
#include "VideoDecoder.h"
#include "Resnet50.h"


int main(int argc, char* argv[])
{
    if(argc != 3) {
        printf("Usage:%s <model_path> <video_path>\n", argv[0]);
        return -1;
    }

    std::string model_path = argv[1];
    std::string video_path = argv[2];

    VideoDecoder decoder(video_path);
    auto vInfo = decoder.GetVideoInfo();
    void*display = decoder.GetDisplay();

    Resnet50 model;
    if(!model.Init(model_path, display, vInfo.width, vInfo.height, vInfo.channels))
    {
        printf("failed to init model!\n");
        return -1;
    }

    while(decoder.IsRunning() || decoder.GetQueueSize()) {
        
        auto frame = decoder.ReadFrame();

        auto surfaceId = decoder.GetSurfaceID(frame);
        
        model.Infer(surfaceId);

        decoder.DeleteFrame(frame);

    }

    return 0;
}