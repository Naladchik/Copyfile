#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <chrono>

#define CHUNK_SIZE 65535
#define ONE 1
#define TWO 2

// sharable global variables
char buf1[CHUNK_SIZE];
char buf2[CHUNK_SIZE];
char p_c_end = 0;  // write in producer read in consumer
int  p_c_actual_size = 0;
bool p_c_b1_full = false;  // write in producer read in consumer
bool p_c_b2_full = false;  // write in producer read in consumer
bool c_p_b1_empty = true;  // write in consumer read in producer
bool c_p_b2_empty = true;  // write in consumer read in producer


void producer(const std::string& input_file_name)
{
    // ===== intro part =====
    std::ifstream input_file(input_file_name, std::ios::binary);
    char p_buf_in_use = ONE;  // can be ONLY 1 or 2

    // ===== loop ===========
    while (!input_file.eof()) {
        if(ONE == p_buf_in_use){
            while(! c_p_b1_empty){}  // wait for signal from consumer
            c_p_b1_empty = false;

            input_file.read(buf1, CHUNK_SIZE);
            p_c_actual_size = input_file.gcount();  // transmit size to consumer

            p_c_b1_full = true;  // send signal to cunsumer
            // p_buf_in_use = TWO;  // internal switch
        }else{  // TWO == p_buf_in_use
            while(! c_p_b2_empty){}  // wait for signal from cunsumer
            c_p_b2_empty = false;

            input_file.read(buf2, CHUNK_SIZE);
            p_c_actual_size = input_file.gcount();  // transmit size to consumer

            p_c_b2_full = true;  // send signal to cunsumer
            p_buf_in_use = ONE;  // internal switch
        }
    }

    // ===== final part =====
    p_c_end = 1;
    input_file.close();
}

void consumer(const std::string& output_file_name)
{
    // ===== intro part =====
    std::ofstream output_file(output_file_name, std::ios::binary);
    char c_buf_in_use = ONE;  // can be ONLY 1 or 2

    // ===== loop ===========
    while(true){
        if(ONE == c_buf_in_use){
            while(! p_c_b1_full){}  // wait for signal from producer
            p_c_b1_full = false;

            output_file.write(buf1, p_c_actual_size);

            c_p_b1_empty = true;  // send signal to producer
            // c_buf_in_use = TWO;  // internal switch
        }else{  // TWO == c_buf_in_use
            while(! p_c_b2_full){}  // wait for signal from producer
            p_c_b2_full = false;

            output_file.write(buf2, p_c_actual_size);

            c_p_b2_empty = true;  // send signal to producer
            c_buf_in_use = ONE;  // internal switch
        }

        if(p_c_end)break;
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

    // Create the reader and writer threads
    std::thread reader_thread(producer, v_input_name);
    std::thread writer_thread(consumer, v_output_name);

    // Wait for both threads to complete
    reader_thread.join();
    writer_thread.join();


    // ----------------------------------------------------------------------------------------
    auto end = std::chrono::high_resolution_clock::now();  // stop of timer
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "execution duration: " << duration.count() << " us" << std::endl;
    return 0;
}
