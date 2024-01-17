#include "Resnet50.h"

#include <vector>
#include <memory>

template<class T>
void softmax(std::vector<std::pair<std::size_t, T>>& result)
{
    T sum_exp = 0.0;
    for (const auto& entry : result) {
        sum_exp += std::exp(entry.second);
    }

    // 计算 softmax
    for (auto& entry : result) {
        entry.second = std::exp(entry.second) / sum_exp;
    }
}


Resnet50::Resnet50() {

}
Resnet50::~Resnet50()
{

}

void Resnet50::Postprocess(ov::InferRequest &request)
{
    ov::Tensor outTensor = request.get_output_tensor();
    float* rawPtr = reinterpret_cast<float*>(outTensor.data());
    std::vector<float> result = std::vector<float>(rawPtr, rawPtr + CLASSNUM);

    std::vector<std::pair<std::size_t, float>> indexValuePairs;
    for (std::size_t i = 0; i < result.size(); ++i) {
		indexValuePairs.emplace_back(i, result[i]);
    }

    std::sort(indexValuePairs.begin(), indexValuePairs.end(), [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });

    softmax(indexValuePairs);

    printf("Best class id:%ld, confidence:%f\n", indexValuePairs[0].first, indexValuePairs[0].second);
    //return std::make_shared<OutputNode>(indexValuePairs[0].first, indexValuePairs[0].second);
}