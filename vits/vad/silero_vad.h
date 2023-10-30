#ifndef SILERO_VAD_H
#define SILERO_VAD_H

#include <torch/torch.h>
#include <torch/script.h>

#include <iostream>
#include <memory>

class __declspec(dllexport) SileroVAD
{
public:
    static void init_model();
    static void reset_states();
    static float predict(const std::vector<float> &chunk, int sr);
    static float predict(const torch::Tensor &chunk, int sr);
    static std::string dump_model_to_string(
        bool print_method_bodies,
        bool print_attr_values,
        bool print_param_values);
private:
    static std::shared_ptr<torch::jit::Module> m_pModel;
    static bool m_bInited;
};

#endif // SILERO_VAD_H