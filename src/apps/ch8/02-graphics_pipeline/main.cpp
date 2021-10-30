#include "GraphicsPipeline.h"
#include "ObjLoader.h"
#include "ObjModels.h"
#include "ShadingRenderer.h"

#include <platform/Filesystem.h>

#include <cstdlib>


using namespace ad;


void renderAll(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    filesystem::path animationFolder = aImagePath / "ch8-demo-anim";
    create_directory(animationFolder);
    focg::renderDemoScene(animationFolder);
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
