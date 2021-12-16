#include "GraphicsPipeline.h"
#include "ObjLoader.h"
#include "ObjModels.h"
#include "ShadingRenderer.h"

#include <platform/Filesystem.h>

#include <resource/PathProvider.h>

#include <cstdlib>


using namespace ad;


void renderAll(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    auto readFile = [](const filesystem::path & aPath)
    {
        std::ifstream ifs{resource::pathFor(aPath).string()};
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    };


    // Cube
    filesystem::path animationFolder = aImagePath / "ch8-demo-anim-cube";
    create_directory(animationFolder);
    focg::renderDemoScene(animationFolder);

    // Bunny
    animationFolder = aImagePath / "ch8-demo-anim-bunny";
    create_directory(animationFolder);
    focg::renderDemoScene(animationFolder,
                          readFile("meshes/bunny-normals.obj"), 100, {0., -0.7, 0.},
                          {800, 800});
}


int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " output_image_folder\n";
        return EXIT_FAILURE;
    }

    try 
    {   
        renderAll(argv[1], {800, 800});
        return EXIT_SUCCESS;
    }
    catch (std::exception & e)
    {
        std::cerr << "Uncaught exception: " << e.what();
        return EXIT_FAILURE;
    }
}
