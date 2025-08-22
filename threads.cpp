#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
std::vector<char> buffer;
bool hasData = false; // Indicates if the buffer contains data
bool doneReading = false; // Indicates if reading is complete

void readFile(const std::string& sourceFileName) {
    std::ifstream sourceFile(sourceFileName, std::ios::binary);
    if (!sourceFile) {
        std::cerr << "Error: Could not open source file!" << std::endl;
        return;
    }

    // Read the file's content into chunks, one at a time.
    const std::size_t chunkSize = 1024; // 1 KB buffer
    char tempBuffer[chunkSize];

    while (!sourceFile.eof()) {
        // Read a chunk of data.
        sourceFile.read(tempBuffer, chunkSize);
        std::streamsize bytesRead = sourceFile.gcount();

        // Lock the mutex and update the shared buffer
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return !hasData; });
            buffer.assign(tempBuffer, tempBuffer + bytesRead); // [action] copiing must be optimized
            hasData = true;
        }
        cv.notify_one(); // Notify the writer thread that data is available
    }

    // Signal that reading is finished
    {
        std::lock_guard<std::mutex> lock(mtx);
        doneReading = true;
    }
    cv.notify_one(); // Notify the writer thread of completion
    sourceFile.close();
}

void writeFile(const std::string& destinationFileName) {
    std::ofstream destinationFile(destinationFileName, std::ios::binary);
    if (!destinationFile) {
        std::cerr << "Error: Could not open or create destination file!" << std::endl;
        return;
    }

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return hasData || doneReading; });

        if (hasData) {
            // Write data from the buffer to the file
            destinationFile.write(buffer.data(), buffer.size());
            hasData = false;
            cv.notify_one(); // Notify the reader thread that buffer is empty
        } else if (doneReading) {
            break; // Exit the loop if reading is complete
        }
    }
    destinationFile.close();
}

int main() {
    std::string sourceFileName, destinationFileName;

    // Get the source and destination file names
    //std::cout << "Enter the source file name: ";
    //std::cin >> sourceFileName;
    //std::cout << "Enter the destination file name: ";
    //std::cin >> destinationFileName;
    sourceFileName = "file_to_copy.txt";
    destinationFileName = "copy.txt";

    std::cout << "copying started" << std::endl;
    // Create the reader and writer threads
    std::thread readerThread(readFile, sourceFileName);
    std::thread writerThread(writeFile, destinationFileName);

    // Wait for both threads to complete
    readerThread.join();
    writerThread.join();

    std::cout << "File copied successfully with two threads!" << std::endl;
    return 0;
}