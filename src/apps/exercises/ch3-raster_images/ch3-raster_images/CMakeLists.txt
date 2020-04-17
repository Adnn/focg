project(ch3-raster_images VERSION "${CMAKE_PROJECT_VERSION}")

set(${PROJECT_NAME}_HEADERS
    BayerPattern.h
)

set(${PROJECT_NAME}_SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME}
               ${${PROJECT_NAME}_HEADERS}
               ${${PROJECT_NAME}_SOURCES}
)

add_executable(ad::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

find_package(Graphics REQUIRED COMPONENTS arte platform)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ad::arte
        ad::platform)

set_target_properties(${PROJECT_NAME} PROPERTIES
                      VERSION "${${PROJECT_NAME}_VERSION}"
)

install(TARGETS ${PROJECT_NAME})
