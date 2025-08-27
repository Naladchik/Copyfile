#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <chrono>

#define COPY_STL false

std::mutex mtx;
std::condition_variable cv;
std::vector<char> v_buffer;
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
    char tempBuffer[chunkSize];  // local buffer for (redundant) copying of one chunk

    while (!sourceFile.eof()) {
        // Read a chunk of data.
        sourceFile.read(tempBuffer, chunkSize);
        std::streamsize bytesRead = sourceFile.gcount();  // this value is chunkSize until the last chunk (it can be less than hcunkSize)

        // Lock the mutex and update the shared buffer
        {
            std::unique_lock<std::mutex> lock(mtx);  //
            cv.wait(lock, [] { return !hasData; });
            v_buffer.assign(tempBuffer, tempBuffer + bytesRead); // [action] copying must be optimized
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
            destinationFile.write(v_buffer.data(), v_buffer.size());  // writes the whole buffer
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
    //sourceFileName = "cat_compilation.mp4";
    destinationFileName = "copy.txt";

    std::cout << "copying started" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    if(COPY_STL){
        std::filesystem::copy(sourceFileName, destinationFileName); // copy file
        std::cout << "File copied successfully with std::filesystem::copy!" << std::endl;
    }else{
        // Create the reader and writer threads
        std::thread readerThread(readFile, sourceFileName);
        std::thread writerThread(writeFile, destinationFileName);

        // Wait for both threads to complete
        readerThread.join();
        writerThread.join();

        std::cout << "File copied successfully with two threads!" << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "execution duration: " << duration.count() << " ms" << std::endl;

    std::filesystem::remove(destinationFileName);
    std::cout << "copy " << destinationFileName << " was deleted" << std::endl;

    return 0;
}