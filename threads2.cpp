#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <chrono>

#define CHUNK_SIZE 65536

std::mutex mtx1, mtx2;
std::condition_variable cv1, cv2;
// std::array<char, CHUNK_SIZE> buf1;
// std::array<char, CHUNK_SIZE> buf2;
char buf1[CHUNK_SIZE];
char buf2[CHUNK_SIZE];
char cur_read_buf = 1;
char cur_write_buf = 1;
std::streamsize n_bytes_1;
std::streamsize n_bytes_2;
bool Stopped = false;

void readFile(const std::string& sourceFileName) {
    std::ifstream sourceFile(sourceFileName, std::ios::binary);
    std::cout << "readFile started" << std::endl;

    while (!sourceFile.eof()) {
        if(1 == cur_read_buf){
            mtx1.lock();
            std::cout << "readFile buf 1" << std::endl;
            sourceFile.read(buf1, CHUNK_SIZE);
            n_bytes_1 = sourceFile.gcount();
            mtx1.unlock();
            cur_read_buf = 2;
        }else{  //2 == cur_read_buf
            mtx2.lock();
            std::cout << "readFile buf 2" << std::endl;
            sourceFile.read(buf2, CHUNK_SIZE);
            n_bytes_2 = sourceFile.gcount();
            mtx2.unlock();
            cur_read_buf = 1;
        }
    }
    Stopped = true;
    sourceFile.close();
}

void writeFile(const std::string& destinationFileName) {
    std::ofstream destinationFile(destinationFileName, std::ios::binary);
    std::cout << "writeFile started" << std::endl;
    while (true) {
        if(1 == cur_write_buf){
            mtx1.lock();
            std::cout << "writeFile buf 1" << std::endl;
            destinationFile.write(buf1, n_bytes_1);
            mtx1.unlock();
            cur_write_buf = 2;
        }else{  //2 == cur_write_buf
            mtx2.lock();
            std::cout << "writeFile buf 2" << std::endl;
            destinationFile.write(buf2, n_bytes_2);
            mtx2.unlock();
            cur_write_buf = 1;
        }
        if(Stopped) break;
    }
    
    destinationFile.close();
}

int main() {
    std::string sourceFileName, destinationFileName;

    sourceFileName = "source.txt";
    destinationFileName = "destination.txt";

    auto start = std::chrono::high_resolution_clock::now();

    // Create the reader and writer threads
    std::thread readerThread(readFile, sourceFileName);
    std::thread writerThread(writeFile, destinationFileName);

    // Wait for both threads to complete
    readerThread.join();
    writerThread.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "execution duration: " << duration.count() << " us" << std::endl;

    return 0;
}