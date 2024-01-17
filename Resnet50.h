#pragma once


#include "ISession.h"

#define CLASSNUM 1000 // 因为使用的是imagenet，存在1000类别

struct OutputNode
{
    std::size_t class_index;
    float confidence;

    OutputNode(std::size_t idx, float conf): class_index(idx), confidence(conf){};
};

class Resnet50: public ISession
{
public:
    Resnet50();
    ~Resnet50();
    
protected:
    void Postprocess(ov::InferRequest& request) override;

};