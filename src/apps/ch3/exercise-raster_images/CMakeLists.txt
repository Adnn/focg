set(TARGET_NAME ch3-raster_images)

set(${TARGET_NAME}_HEADERS
    BayerPattern.h
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

find_package(Graphics CONFIG REQUIRED COMPONENTS arte)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::arte
)

install(TARGETS ${TARGET_NAME})
