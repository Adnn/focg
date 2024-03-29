set(TARGET_NAME ch4-ray_tracer)

set(${TARGET_NAME}_HEADERS
    Hit.h
    Intersect.h
    Light.h
    Material.h
    Ray.h
    RayTracer.h
    Scene.h
    Shading.h
    Surfaces.h
    View.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp

    Surfaces.cpp
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
find_package(Graphics REQUIRED COMPONENTS arte)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::arte
        ad::math
)


##
## Install
##

install(TARGETS ${TARGET_NAME})
