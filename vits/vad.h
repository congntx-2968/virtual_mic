#ifndef VAD_H
#define VAD_H

#include <torch/torch.h>
#include <torch/script.h>

#include <vector>
#include <tuple>

typedef struct vad_params{
    int chunk_size = 512;
    int sampling_rate = 16000;
    int target_sampling_rate = 24000;
    float threshold = 0.5;
    float neg_threshold = 0.35;
    int min_speech_duration_frames = 5;
    int min_silence_duration_frames = 10;
    std::string device = "cpu";
} vad_options;

class VAD
{
public:
    explicit VAD();
    explicit VAD(const vad_options& options);

    void set_options(const vad_options& options);
    void init_model();
    std::vector<int> get_speech_states(const std::vector<float> &speech);
    std::vector<int> get_speech_states(const torch::Tensor &speech);
private:
    enum SPEECH_STATE{
        INACTIVATED = 0, ACTIVATED = 1
    };
    SPEECH_STATE prev_state = INACTIVATED;
    vad_options options;
    int step = 1;

};

#endif // VAD_H
