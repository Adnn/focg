#pragma once


# include "Scene.h"

#include <fstream>
#include <istream>
#include <sstream>
#include <string>


namespace ad {
namespace focg {


void appendToScene(const std::string & aFilename, Scene & aScene);

//
// Implementations
//
#define ERR(x) throw std::logic_error{(x)}


template <class T_colorStore>
void appendToScene(std::istream & aInputObj, Scene & aScene, const T_colorStore & aColors)
{
    std::vector<Vertex> vertices;
    for (std::string line; std::getline(aInputObj, line);)
    {
        std::istringstream input{line};
        std::string type;
        input >> type;

        // Empty line
        if (type.empty())
        {}
        // comment
        else if (type[0] == '#')
        {}
        // object name
        else if (type == "o")
        {}
        // Vertex
        else if (type == "v")
        {
            double x, y, z;
            input >> x; input >> y; input >> z;
            if (input)
            {
                vertices.push_back({
                        {x, y, z, 1.0},
                        aColors[vertices.size() % aColors.size()]
                });
            }
            else
            {
                ERR("Invalid vertex format.");
            }
        }
        // Vertex normal
        else if (type == "vn")
        {}
        // Texture coordinate
        else if (type == "vt")
        {}
        else if (type == "f")
        {
            std::array<std::size_t, 3> triangleVertices;
            int count = 0;
            for (std::string indices; input >> indices;)
            {
                // ATTENTION Obj format is 1-indexed
                triangleVertices[count++] = std::stoul(indices.substr(0, indices.find("/"))) - 1;
            }

            if (count != 3)
            {
                ERR("Only handle faces with 3 vertices, not " + std::to_string(count) + ".");
            }
            else
            {
                aScene.triangles.push_back({
                        vertices.at(triangleVertices[0]),
                        vertices.at(triangleVertices[1]),
                        vertices.at(triangleVertices[2])
                });
            }
        }
        else
        {
            ERR("Unsupported line type: " + type);
        }
    }
}


template <class T_colorStore>
void appendToScene(const std::string & aFilename,
                   Scene & aScene,
                   const T_colorStore & aColors)
{
    appendToScene(std::ifstream{aFilename}, aScene, aColors);
}


inline void appendToScene(std::istream & aInputObj,
                          Scene & aScene,
                          math::hdr::Rgb_d aColor = math::hdr::gWhite<>)
{
    appendToScene(aInputObj, aScene, std::vector<math::hdr::Rgb_d>{aColor});
}


#undef ERR


} // namespace focg
} // namespace ad]
