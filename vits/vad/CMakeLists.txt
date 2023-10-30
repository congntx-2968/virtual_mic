add_library(silerovad SHARED silero_vad.cpp ${OBJECT_FILE})

target_include_directories(silerovad
    PRIVATE ${TORCH_INCLUDE_DIRS}
)
target_link_libraries(silerovad PRIVATE ${TORCH_LIBRARIES})