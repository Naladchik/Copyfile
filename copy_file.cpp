#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <chrono>
#include <cstring>

using namespace std;

constexpr unsigned int chunk_size = 65535;

// sharable global variables
char buf[2][chunk_size];
unsigned char buf_prod = 0;  // number of buffer for the next operation of producer
unsigned char buf_cons = 0;  // number of buffer for the next operation of consumer
unsigned char prod_num = 2;  // number of available for procurer buffers
unsigned char cons_num = 0;  // number of available for consumer buffers
unsigned int actual_size;
bool copy_with_stl = false;
bool finish = false;
std::string v_input_name, v_output_name;

mutex mtx;
condition_variable cv;

void producer(const std::string& input_file_name)
{
    // ===== intro part =====
    std::ifstream input_file(input_file_name, std::ios::binary);

    // ===== loop ===========
    while (!input_file.eof()) {
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return prod_num > 0; });
            prod_num--;
        }
        input_file.read(buf[buf_prod], chunk_size);
        actual_size = input_file.gcount();  // transmit size to consumer
        {
            lock_guard<mutex> lock(mtx);
            cons_num++;
            cv.notify_one();
        }
        buf_prod ^= 0x01;
    }

    // ===== final part =====
    finish = true;
}

void consumer(const std::string& output_file_name)
{
    // ===== intro part =====
    std::ofstream output_file(output_file_name, std::ios::binary);

    // ===== loop ===========
    while(true){
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return cons_num > 0; });
            cons_num--;
        }
        output_file.write(buf[buf_cons], actual_size);
        buf_cons ^= 0x01;
        {
            lock_guard<mutex> lock(mtx);
            prod_num++;
            if( finish && (cons_num == 0) ) break;
            cv.notify_one();
        }        
    }

    // ===== final part =====   
}

int main(int argc, char*argv[])
{
    if(2 > argc) {cout << "You enter no arguments. For help use --help command." << endl; return 1;}
	if(2 == argc){
		if(strcmp(argv[1], "--help")){
			cout << "Unknown key " << argv[1] << endl; return 1;
		}else{
			cout << "Enter copy_file <in file> -a <out file> to copy with DIY utiity." << endl;
			cout << "Enter copy_file <in file> -b <out file> to copy with standard copy_file() function." << endl; 
			cout << "Examples: copy_file input.txt -a output.txt." << endl; 
			return 0;
		}	
	}
	if(3 == argc) {printf("Impossible argument combination %s %s. For help use --help command.\n", argv[1], argv[2]); return 1;}
	if(4 == argc) {
		if(strcmp(argv[2], "-a") && strcmp(argv[2], "-b")){
			cout << "Uncnown command " << argv[2] <<". Enter copy_file --help for help." << endl; return 1;
		}else{
            v_input_name = argv[1];
            v_output_name = argv[3];
            if(strcmp(argv[2], "-a")) copy_with_stl = true;
        }
	}
	if(4 < argc) {printf("You enter too many arguments. For help use --help command.\n"); return 1;}

    auto start = std::chrono::high_resolution_clock::now();  // start of timer
    // ----------------------------------------------------------------------------------------

    if(copy_with_stl){
        cout << "copying with standard library" << endl;
        filesystem::copy_file(v_input_name, v_output_name);
    }else{
        cout << "copying with DIY" << endl;
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
