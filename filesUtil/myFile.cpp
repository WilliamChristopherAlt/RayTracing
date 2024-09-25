#include <filesUtil/myFile.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <windows.h>

// Returns the path to the executable directory
std::string getExecutableDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

std::string getPath(const std::string& pathRelative, int dirUpTraversal)
{
    std::string exeDir = getExecutableDirectory();
    for (int i = 0; i < dirUpTraversal; i++) {
        std::size_t pos = exeDir.find_last_of("\\");
        if (pos != std::string::npos)
            exeDir = exeDir.substr(0, pos);
    }
    
    std::string fullPath = exeDir + "\\" + pathRelative;
    return fullPath;
}

std::string getFileContents(const std::string& filePath)
{    
    std::ifstream in(filePath.c_str(), std::ios::binary);

    if (in) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    } else {
        std::cout << "Cannot find the path specified" << std::endl;
        std::cout << "Path: " << filePath << std::endl;
        std::cout << "Execute directory: " << getExecutableDirectory() << std::endl;
        throw(errno);
    }
}

void printFileContent(std::ifstream& fileStream) {
    if (!fileStream) {
        std::cerr << "Error: File stream is not valid." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(fileStream, line)) { // Read line by line
        std::cout << line << std::endl; // Print each line
    }

    // Optional: Check for any read errors
    if (fileStream.bad()) {
        std::cerr << "Error occurred while reading the file." << std::endl;
    }
}

std::vector<std::string> getFilenamesInFolder(const std::string& folderPath) {
    std::vector<std::string> filenames;

    // Check if the folder exists and is a directory
    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        // Iterate over the directory entries
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            // Get the filename and add to the vector
            if (fs::is_regular_file(entry.status())) {
                filenames.push_back(entry.path().filename().string());
            }
        }
    } else {
        std::cerr << "Folder does not exist or is not a directory or the model doesn't have any textures." << std::endl;
    }

    return filenames;
}