## Description

This project aim to convert your voice to another voice using deep learning model. The output will be play in your playback devices so you need 3rd-party virtual microphone softwares if you want to use it as a microphone in other applications.


## Build

1. Download and extract libtorch in this [link](https://pytorch.org/)

2. Change path to libtorch in `CMakeLists.txt` to the directory you have extracted at this line:
```cmake
set(TORCH_INSTALL_PREFIX D:\\Libraries\\libtorch\\2.0.1+cu118\\${CMAKE_BUILD_TYPE})
```

3. Install Qt5

4. Run this command for build:
```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
```

5. The application in `build/bin` folder.
