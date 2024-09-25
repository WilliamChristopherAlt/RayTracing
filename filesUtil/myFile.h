#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
namespace fs = std::filesystem;

std::string getExecutableDirectory();

std::string getPath(const std::string& pathRelative, int dirUpTraversal);

std::string getFileContents(const std::string& filePath);

void printFileContent(std::ifstream& fileStream);

std::vector<std::string> getFilenamesInFolder(const std::string& folderPath);