#pragma once


# include "Scene.h"

#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>


namespace ad {
namespace focg {


void appendToScene(const std::string & aFilename, Scene & aScene);

//
// Implementations
//
#define ERR(x) throw std::logic_error{(x)}



std::vector<std::string> splitString(const std::string & aString, char aDelimiter)
{
    std::stringstream in{aString};
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(in, segment, aDelimiter))
    {
       seglist.push_back(segment);
    }
    return seglist;
}


template <class T_colorStore, class T_vertex>
void appendToScene(std::istream & aInputObj, Scene_base<T_vertex> & aScene, const T_colorStore & aColors)
{
    std::vector<T_vertex> vertices;
    std::vector<HVec> normals;
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
                vertices.push_back(T_vertex{
                        HPos{x, y, z, 1.0},
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
        {
            double x, y, z;
            input >> x; input >> y; input >> z;
            normals.push_back({x, y, z, 0.0});
        }
        // Texture coordinate
        else if (type == "vt")
        {}
        else if (type == "f")
        {
            std::array<std::size_t, 3> triangleVertices;
            int count = 0;
            for (std::string indicesStr; input >> indicesStr;)
            {
                std::vector<std::string> indices = splitString(indicesStr, '/');
                assert(indices.size() == 3);

                // ATTENTION Obj format is 1-indexed
                std::size_t vertexIndex = std::stoul(indices[0]) - 1;
                triangleVertices[count++] = vertexIndex;

                if constexpr(std::is_same_v<T_vertex, VertexAdvanced>)
                {
                    // If it is VertexAugmented
                    std::size_t normalIndex = std::stoul(indices[2]) - 1;
                    // Dirty: patch the vertex in the initial list each time with the normal.
                    vertices.at(vertexIndex).normal = normals.at(normalIndex);
                }
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


template <class T_colorStore, class T_vertex>
void appendToScene(const std::string & aFilename,
                   Scene_base<T_vertex> & aScene,
                   const T_colorStore & aColors)
{
    appendToScene(std::ifstream{aFilename}, aScene, aColors);
}


template <class T_vertex>
inline void appendToScene(std::istream & aInputObj,
                          Scene_base<T_vertex> & aScene,
                          math::hdr::Rgb aColor = math::hdr::gWhite)
{
    appendToScene(aInputObj, aScene, std::vector<math::hdr::Rgb>{aColor});
}


#undef ERR


} // namespace focg
} // namespace ad]