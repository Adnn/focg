set(TARGET_NAME ch8-01-basic_pipeline)

set(${TARGET_NAME}_HEADERS
    Clipping.h
    GraphicsPipeline.h
    Line.h
    ObjLoader.h
    ObjModels.h
    NaivePipeline.h
    Rasterization.h
    Scene.h
    Shaders.h
    Triangle.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

add_executable(${TARGET_NAME}
               ${${TARGET_NAME}_HEADERS}
               ${${TARGET_NAME}_SOURCES}
)

add_executable(ad::${TARGET_NAME} ALIAS ${TARGET_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES
                      VERSION "${${PROJECT_NAME}_VERSION}"
)

##
## Dependencies
##

find_package(Math REQUIRED COMPONENTS math)
find_package(Graphics REQUIRED COMPONENTS arte graphics platform)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::arte
        ad::graphics
        ad::math
        ad::platform)


##
## Install
##

install(TARGETS ${TARGET_NAME})
