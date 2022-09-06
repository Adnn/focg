set(TARGET_NAME ch9-02-image_filtering)

set(${TARGET_NAME}_HEADERS
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

find_package(Graphics REQUIRED COMPONENTS arte)
find_package(Math REQUIRED COMPONENTS math)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::arte
        ad::math
)


##
## Install
##

install(TARGETS ${TARGET_NAME})
