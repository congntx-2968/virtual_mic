add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vad)

add_library(vits SHARED ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp ${CMAKE_CURRENT_SOURCE_DIR}/vad.cpp)

target_include_directories(vits
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vad
    PRIVATE ${TORCH_INCLUDE_DIRS}
)

target_link_libraries(vits
    PRIVATE silerovad
    PRIVATE ${TORCH_LIBRARIES}
)
