#include "ISession.h"

#include <chrono>

ISession::ISession(/* args */)
{
}

ISession::~ISession()
{
}

bool ISession::Init(const std::string& model_path, void* display, int inputWidth, int inputHeight, int inputChannels)
{
    iWidth = inputWidth;
    iHeight = inputHeight;

    ov::Core ov_core;
    ov_model = ov_core.read_model(model_path);

    BuildModel(inputWidth, inputHeight, inputChannels);

    ov::intel_gpu::ocl::VAContext ov_context(ov_core, display);
    ov_compiled_model = ov_core.compile_model(ov_model, ov_context);
    request = ov_compiled_model.create_infer_request();

    return true;
}

void ISession::BuildModel(int inputWidth, int inputHeight, int inputChannels)
{
    ov::preprocess::PrePostProcessor ppp(ov_model);
    ppp.input().tensor()
        .set_element_type(ov::element::u8)
        .set_shape({1, inputHeight, inputWidth, inputChannels})
        .set_layout("NHWC")
        // 这里可以直接使用这个ffmpeg 转换为rgb都可以
        .set_color_format(ov::preprocess::ColorFormat::NV12_TWO_PLANES, {"y", "uv"}) 
        .set_memory_type(ov::intel_gpu::memory_type::surface);

    ppp.input().preprocess()
        .convert_color(ov::preprocess::ColorFormat::BGR)
        .resize(ov::preprocess::ResizeAlgorithm::RESIZE_LINEAR, 224, 224)
        .convert_element_type(ov::element::f32)
        .scale(255.f);

    ppp.input().model().set_layout("NCHW");

    ov_model = ppp.build();
}

auto ISession::CreateNVTensor(unsigned int id)
{
    auto va_context = ov_compiled_model.get_context().as<ov::intel_gpu::ocl::VAContext>();
    return va_context.create_tensor_nv12(iHeight, iWidth, id);
}

void ISession::Infer(unsigned int surfaceId)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto nvTensor = CreateNVTensor(surfaceId);

    request.set_input_tensor(0, nvTensor.first);    // Y plane
    request.set_input_tensor(1, nvTensor.second);   // UV plane

    request.infer();

    Postprocess(request);

    auto end = std::chrono::high_resolution_clock::now();
    printf("infer consume: %f ms\n", 
        static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()));
}
