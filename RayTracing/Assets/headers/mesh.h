#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <map>

#include <textureClass.h>
#include <filesUtil/myFile.h>

const int DIFFUSE = 0;
const int SPECULAR = 1;
const int LIGHT = 2;
const int CHECKER = 3;
const int GLASS = 4;
const int TEXTURE = 5;

const int MAX_TEXTURES = 5;

struct Material
{
    glm::vec4 color;
    glm::vec4 specularColor;
    glm::vec4 emissionColor;

    int textureIndex = -1;
    float emissionStrength;
    float smoothness; 
    float specularProbability;

    float checkerScale;
    float refractiveIndex;
    int materialType; // 76 bytes
    int index; // padded, 80 bytes

    Material() : color(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)), materialType(DIFFUSE) {}

    void makeDiffusive(const glm::vec3& col)
    {
        materialType = DIFFUSE;
        color = glm::vec4(col, 0.0f);
    }

    void makeSpecular(const glm::vec3& col, const glm::vec3& specularCol, float smooth, float specProb)
    {
        materialType = DIFFUSE;
        color = glm::vec4(col, 0.0f);
        specularColor = glm::vec4(specularCol, 0.0f);
        smoothness = smooth;
        specularProbability = specularProbability;
    }

    void makeLight(const glm::vec3& emissionCol, float emissionStr)
    {
        materialType = LIGHT;
        emissionColor = glm::vec4(emissionCol, 0.0f);
        emissionStrength = emissionStr;
    }

    void makeChecker(float scale)
    {
        materialType = CHECKER;
        checkerScale = scale;
    }

    void makeGlass(const glm::vec3& col, float refIndex)
    {
        materialType = GLASS;
        color = glm::vec4(col, 0.0f);
        refractiveIndex = refIndex;
    }

    void makeTextured(int texIndex, const glm::vec3& col)
    {
        materialType = TEXTURE;
        textureIndex = texIndex;;
    }
};

struct Sphere
{
    Material material;
    glm::vec4 center;
    float radius;
};

struct RTXTriangle
{
    glm::vec4 a;
    glm::vec4 b;
    glm::vec4 c;
    glm::vec2 aTex;
    glm::vec2 bTex;
    glm::vec2 cTex;
    int materialIndex;
    float pad; // 80 bytes

    RTXTriangle(int matIndex, const glm::vec4& a_, const glm::vec4& b_, const glm::vec4& c_,
            const glm::vec2& aTex_, const glm::vec2& bTex_, const glm::vec2& cTex_) 
            : materialIndex(matIndex), a(a_), b(b_), c(c_), aTex(aTex_), bTex(bTex_), cTex(cTex_) {}

    void print() const {
        std::cout << "RTXTriangle vertices:" << std::endl;
        std::cout << "Vertex A: (" << a.x << ", " << a.y << ", " << a.z << ", " << a.w << ")" << std::endl;
        std::cout << "Vertex B: (" << b.x << ", " << b.y << ", " << b.z << ", " << b.w << ")" << std::endl;
        std::cout << "Vertex C: (" << c.x << ", " << c.y << ", " << c.z << ", " << c.w << ")" << std::endl;

        std::cout << "Texture coordinates:" << std::endl;
        std::cout << "Tex A: (" << aTex.x << ", " << aTex.y << ")" << std::endl;
        std::cout << "Tex B: (" << bTex.x << ", " << bTex.y << ")" << std::endl;
        std::cout << "Tex C: (" << cTex.x << ", " << cTex.y << ")\n" << std::endl;
    }
};

struct BVHTriangle
{
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 center;
    int index;

    BVHTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
    {
        min = glm::min(glm::min(a, b), c);
        max = glm::max(glm::max(a, b), c);
        center = (a + b + c) / 3.0f;
    }
};

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::string token;
    for (char ch : str) {
        if (ch == delimiter) {
            if (!token.empty()) {
                result.push_back(token); // Add the token to the result
                token.clear(); // Reset the token
            }
        } else {
            token += ch; // Append character to the current token
        }
    }
    // Add the last token if not empty
    if (!token.empty()) {
        result.push_back(token);
    }
    return result;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        if (start != end) { // Avoid adding empty tokens
            result.push_back(str.substr(start, end - start)); // Add the token
        }
        start = end + delimiter.length(); // Move past the delimiter
        end = str.find(delimiter, start); // Find the next occurrence
    }

    // Add the last token if there's remaining content
    if (start < str.length()) {
        result.push_back(str.substr(start));
    }

    return result;
}

std::string strStreamToString(std::stringstream& strStream) {
    // Save the current position of the stream
    std::streampos originalPos = strStream.tellg();

    // Read the entire stream into a string
    std::string tempString((std::istreambuf_iterator<char>(strStream)), std::istreambuf_iterator<char>());

    // Restore the original position of the stream
    strStream.clear(); // Clear any error flags
    strStream.seekg(originalPos);

    return tempString;
}

int find(const std::string& str, int startPos, char chr)
{
    for (int i = startPos; i < str.length(); i++)
    {
        if (chr == str[i])
            return i;
    }
    return -1;
}

void getTrianglesData_(const std::string& folderRelativePath, int dirUpTraversal,
                        std::vector<RTXTriangle>& rtxTriangles, std::vector<BVHTriangle>& bvhTriangles,
                        std::vector<Material>& materials, std::vector<Texture2D>& textures)
{
    size_t namePosStart = folderRelativePath.find_last_of('\\') + 1;
    size_t namePosEnd = folderRelativePath.find_last_of('.');
    std::string fileName = folderRelativePath.substr(namePosStart, namePosEnd);

    std::string folderPath = getPath(folderRelativePath, dirUpTraversal);
    std::string objFilePath = folderPath + "\\" + fileName + ".obj";

    std::ifstream objFileStream(objFilePath);

    if (!objFileStream.is_open())
    {
        std::cout << "Cannot find the OBJ path specified." << std::endl;
        std::cout << "OBJ path: " << objFilePath << std::endl;
        throw(errno);
    }

    std::cout << "Loading model, please wait..." << std::endl;

    // Texture files
    std::map<std::string, int> texFileToIndex;
    std::vector<std::string> textureNames = getFilenamesInFolder(folderPath + "\\textures");
    for (int i = 0; i < textureNames.size(); i++)
    {
        texFileToIndex[textureNames[i]] = i;
        Texture2D texture(folderPath + "\\textures\\" + textureNames[i], GL_TEXTURE0 + i);
        textures.push_back(texture);
    }

    // MTL files
    std::map<std::string, std::map<std::string, Material>> libToMtlMaps;
    Material defaultMtl = Material();
    defaultMtl.index = 0;
    libToMtlMaps["_default_"]["_default_"] = defaultMtl ;
    std::vector<std::string> mtlFileNames = getFilenamesInFolder(folderPath);
    int matIndex = 0;

    for (const std::string& name : mtlFileNames)
    {
        std::map<std::string, Material> nameToMtl;
        size_t dotPos = name.find('.');
        if (dotPos == std::string::npos)
        {
            std::cout << "File extension not found" << std::endl;
            throw(errno);
        }

        if (name.substr(dotPos + 1) != "mtl")
            continue;

        std::ifstream mtlFileStream(folderPath + "\\" + name);

        std::string mtlName;

        while (!mtlFileStream.eof())
        {
            char line[256];
            std::stringstream strStream;

            mtlFileStream.getline(line, 256);
            strStream.str("");
            strStream.clear();
            strStream << line;

            std::string key;
            strStream >> key;
            
            if (key == "newmtl")
            {
                strStream >> mtlName;
                Material mat = Material();
                mat.index = ++matIndex;
                nameToMtl[mtlName] = mat;
                // std::cout << mtlName << std::endl;
            }
            else if (key == "Kd")
            {
                glm::vec3 v;
                strStream >> v.x >> v.y >> v.z;
                nameToMtl[mtlName].color = glm::vec4(v, 0.0f);
                // nameToMtl[mtlName].color = glm::vec4(1.0f, 0.0f, 1.0f, 0.0f);
            }
            else if (key == "map_Kd")
            {
                std::string texName;
                strStream >> texName;
                nameToMtl[mtlName].materialType = TEXTURE;
                int index;
                try {
                    index = texFileToIndex.at(texName);
                } catch (const std::out_of_range& e) {
                    std::cerr << "Key not found: " << e.what() << std::endl;
                    throw(errno);
                }
                nameToMtl[mtlName].textureIndex = index;
            }
            libToMtlMaps[name] = nameToMtl;
        }
    }

    for (const auto& libNameToMtlDict : libToMtlMaps)
    {
        for (const auto& mtlNameToMtl : libNameToMtlDict.second)
        {
            materials.push_back(mtlNameToMtl.second);
            // std::cout << mtlNameToMtl.first << std::endl;
            // std::cout << mtlNameToMtl.second.materialType << std::endl;
            // std::cout << mtlNameToMtl.second.index << std::endl;
        }
    }

    // OBJ file
    std::vector<glm::vec3> verts;
    std::vector<glm::vec2> texCoords;

    std::string currentLib = "_default_";
    std::string currentMtlName = "_default_";

    while (!objFileStream.eof())
    {
        std::string line;
        std::getline(objFileStream, line);
        std::stringstream strStream(line);
        std::string lineType;
        strStream >> lineType;
        
        if(lineType == "mtllib")
        {
            strStream >> currentLib;
        }
        else if (lineType == "usemtl")
        {
            strStream >> currentMtlName;
        }
        else if (lineType == "v")
        {
            glm::vec3 v;
            strStream >> v.x >> v.y >> v.z;
            verts.push_back(v);
        }
        else if (lineType == "vt")
        {
            glm::vec2 v;
            strStream >> v.x >> v.y;
            texCoords.push_back(v);
        }
        else if (lineType == "f")
        {
            int spacesOccurances = std::count(line.begin(), line.end(), ' ');
            if (spacesOccurances != 3)
            {
                std::cerr << "Invalid OBJ file, non-triangle face not supported.\nLine: " << line << std::endl;
                throw(errno);
            }

            std::vector<std::string> faceData = split(line, ' ');
            int slashOccurances = std::count(faceData[1].begin(), faceData[1].end(), '/');

            std::string vertexInfo[3];

            glm::vec3 trianglePoints[3];
            glm::vec2 triangleTexCoords[3];

            bool isTexture;

            // Diffuse
            if (slashOccurances == 0)
            {
                for (int i = 0; i < 3; ++i)
                {
                    strStream >> vertexInfo[i];
                    trianglePoints[i] = verts[std::stoi(vertexInfo[i]) - 1];
                }
                isTexture = false;
            }
            // Texture 
            // Format is a/a b/b c/c or a/a/a b/b/b c/c/c
            if (slashOccurances == 1 || (slashOccurances == 2 && faceData[1].find("//") == std::string::npos))
            {
                for (int i = 0; i < 3; ++i)
                {
                    strStream >> vertexInfo[i];
                    size_t slashPos = vertexInfo[i].find('/');
                    trianglePoints[i] = verts[std::stoi(vertexInfo[i].substr(0, slashPos)) - 1];

                    size_t secondSlashPos = vertexInfo[i].find('/', slashPos + 1);
                    if (secondSlashPos == std::string::npos) // a/a format
                    {
                        triangleTexCoords[i] = texCoords[std::stoi(vertexInfo[i].substr(slashPos + 1)) - 1];
                    }
                    else // a/a/a format
                    {
                        triangleTexCoords[i] = texCoords[std::stoi(vertexInfo[i].substr(slashPos + 1, secondSlashPos)) - 1];
                    }
                }
                isTexture = true;
            }
            // Diffuse
            else
            {
                for (int i = 0; i < 3; ++i)
                {
                    strStream >> vertexInfo[i];
                    size_t slashPos = vertexInfo[i].find('/');
                    trianglePoints[i] = verts[std::stoi(vertexInfo[i].substr(0, slashPos)) - 1];
                }
                isTexture = false;
            }

            Material material;
            try {
                material = libToMtlMaps.at(currentLib).at(currentMtlName); 
            } catch (const std::out_of_range& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Requested material or library not found: " << currentLib << ", " << currentMtlName << std::endl;
    
                throw(errno);
            }
            rtxTriangles.push_back(RTXTriangle(libToMtlMaps.at(currentLib).at(currentMtlName).index, glm::vec4(trianglePoints[0], 0.0f),
            glm::vec4(trianglePoints[1], 0.0f), glm::vec4(trianglePoints[2], 0.0f),
            triangleTexCoords[1], triangleTexCoords[2], triangleTexCoords[0]));

            bvhTriangles.push_back(BVHTriangle(trianglePoints[0], trianglePoints[1], trianglePoints[2]));
        }
    }
    std::cout << bvhTriangles.size() << " triangles loaded" << std::endl;
}