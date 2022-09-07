set(TARGET_NAME ch9-01-convolution_tests)

set(${TARGET_NAME}_HEADERS
    Convolution.h
    Filters.h
    Reconstruction.h
)

set(${TARGET_NAME}_SOURCES
    Convolution_tests.cpp
    Reconstruction_tests.cpp
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

find_package(Graphics REQUIRED COMPONENTS arte)

find_package(Math REQUIRED COMPONENTS math)

find_package(Catch2)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::arte
        ad::math

        Catch2::Catch2WithMain
)


##
## Install
##

install(TARGETS ${TARGET_NAME})
