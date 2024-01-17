#pragma once

#include <string>

#include <openvino/openvino.hpp>

#include <openvino/runtime/intel_gpu/ocl/va.hpp>
#include <libavformat/avformat.h>


class ISession
{
public:
    ISession(/* args */);
    ~ISession();

    bool Init(const std::string& model_path, void* display, int inputWidth, int inputHeight, int inputChannels);

    void Infer(unsigned int surfaceId);

protected:
    
    virtual void Postprocess(ov::InferRequest& request) = 0;

private:

    auto CreateNVTensor(unsigned int id);

    void BuildModel(int inputWidth, int inputHeight, int inputChannels);
private:

    std::shared_ptr<ov::Model> ov_model;
    ov::CompiledModel ov_compiled_model;

    ov::InferRequest request;

    int iWidth;
    int iHeight;
};

