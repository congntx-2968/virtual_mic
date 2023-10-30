#include <iostream>
#include "utils.h"
#include "model.h"

int main(int argc, char* argv[]) {
    if(argc < 5){
        std::cout << "Usage: " << argv[0] << " encoder decoder input output";
        return 0;
    }
    auto [s, fs] = read_wav(argv[3]);
    VITSWhisperV2 model(argv[1], argv[2]);
    std::vector<float> outputs;
    while (!s.empty()){
        clock_t t0 = clock();
        std::vector<float> data(SEGMENT_SIZE, 0);
        if (s.size() > SEGMENT_SIZE)
        {
            data.assign(s.cbegin(), s.cbegin() + SEGMENT_SIZE);
            std::vector<float> o = model.inference(data);
            outputs.insert(outputs.cend(), o.cbegin(), o.cend());
            s.erase(s.cbegin(), s.cbegin() + SEGMENT_SIZE);
        }
        else
        {
            std::copy(s.cbegin(), s.cend(), data.begin());
            std::vector<float> o = model.inference(data);
            outputs.insert(outputs.cend(), o.cbegin(), o.cbegin() + s.size());
            s.clear();
        }
        float ms = (clock() - t0) * 1000.0f / CLOCKS_PER_SEC;
        float rt = SEGMENT_SIZE * 1000.0f / fs;
        std::cout << "Speed: " << rt / ms << "x" << std::endl;
    }

    save_wav(argv[4], outputs, 24000);
    return 0;
}
