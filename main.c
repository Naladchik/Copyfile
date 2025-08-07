#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::string sourceFileName, destinationFileName;

    // Receive input for the source file name
    std::cout << "Enter the source file name: ";
    std::cin >> sourceFileName;

    // Receive input for the destination file name
    std::cout << "Enter the destination file name: ";
    std::cin >> destinationFileName;

    // Open the source file in read mode
    std::ifstream sourceFile(sourceFileName, std::ios::binary);
    if (!sourceFile) {
        std::cerr << "Error: Could not open source file!" << std::endl;
        return 1; // Exit with error code
    }

    // Open the destination file in write mode
    std::ofstream destinationFile(destinationFileName, std::ios::binary);
    if (!destinationFile) {
        std::cerr << "Error: Could not open or create destination file!" << std::endl;
        return 1; // Exit with error code
    }

    // Copy content from source file to destination file
    destinationFile << sourceFile.rdbuf();

    // Close the files
    sourceFile.close();
    destinationFile.close();

    std::cout << "File content has been copied successfully!" << std::endl;

    return 0; // Exit with success code
}