#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <chrono>

using namespace std;

#define COPY_WITH_STL false

#define CHUNK_SIZE 65535
//#define CHUNK_SIZE 2000

// sharable global variables
char buf1[CHUNK_SIZE];
char buf2[CHUNK_SIZE];
char * buf_from_file = buf1;
char * buf_to_file = buf1;
unsigned int p_c_actual_size;
bool data_ready = false;
bool write_finished = true;
bool finish = false;

mutex mtx;
condition_variable cv1, cv2;

void buff_toggle(char ** buf){
    if(*buf == buf1) *buf = buf2; else *buf = buf1;
}


void producer(const std::string& input_file_name)
{
    // ===== intro part =====
    std::ifstream input_file(input_file_name, std::ios::binary);

    // ===== loop ===========
    while (!input_file.eof()) {
        {
            unique_lock<mutex> lock(mtx);
            cv2.wait(lock, [] { return write_finished; });
            write_finished = false; 
        }
        //cout << "reading" << endl;
        input_file.read(buf_from_file, CHUNK_SIZE);
        p_c_actual_size = input_file.gcount();  // transmit size to consumer
        {
            lock_guard<mutex> lock(mtx);
            data_ready = true;        
            cv1.notify_one();
        }
        buff_toggle(&buf_from_file);
    }

    // ===== final part =====
    finish = true;
    input_file.close();
}

void consumer(const std::string& output_file_name)
{
    // ===== intro part =====
    std::ofstream output_file(output_file_name, std::ios::binary);

    // ===== loop ===========
    while(true){
        {
            unique_lock<mutex> lock(mtx);
            cv1.wait(lock, [] { return data_ready; });
            data_ready = false; 
        }
        // cout << "writing" << endl;
        output_file.write(buf_to_file, p_c_actual_size);
        buff_toggle(&buf_to_file);
        {
            lock_guard<mutex> lock(mtx);
            write_finished = true;        
            cv2.notify_one();
        }        
        if(finish) break;
    }

    // ===== final part =====    
    output_file.close();
}

int main()
{
    auto start = std::chrono::high_resolution_clock::now();  // start of timer
    // ----------------------------------------------------------------------------------------

    std::string v_input_name, v_output_name;

    v_input_name = "input.txt";
    v_output_name = "output.txt";

    if(COPY_WITH_STL){
        filesystem::copy_file(v_input_name, v_output_name);
    }else{
        // Create the reader and writer threads
        std::thread reader_thread(producer, v_input_name);
        std::thread writer_thread(consumer, v_output_name);

        // Wait for both threads to complete
        reader_thread.join();
        writer_thread.join();
    }

    // ----------------------------------------------------------------------------------------
    auto end = std::chrono::high_resolution_clock::now();  // stop of timer
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "execution duration: " << duration.count() << " us" << std::endl;
    return 0;
}
