void run_disjkstra();
void simple_test();
void test_bitcount();
void test_quick_sort();
void test_fft();
void test_lesolve();

void testDetermineWriteEnergy();
void testDetermineWritingTime();

#include <system/service/logger.h>

struct __attribute__ ((packed)) config_word_interface{
unsigned long long writeMode:4;
unsigned long long column:22;
unsigned long long row:22;
unsigned long long subarray:4;
unsigned long long bank:4;
unsigned long long rank:4;
unsigned long long channel:4;
};

void use_interface(){

    /* write magic word */
    struct config_word_interface interface = {3,0,0,3,3,3,3};

    /* update write mode with interface */
    interface = {3,255,3,0,0,0,0};
    log(dec << "Updating Area with location: " << (int) interface.channel << ":" <<  (int) interface.rank << ":" << (int) interface.bank << ":" << (int) interface.subarray << ":" << (int) interface.row << ":" << (int) interface.column << " with mode: " << (int) interface.writeMode);

    test_bitcount();

}


void app_init() {
    
    log("Calling application");
    
    use_interface();
    
    asm volatile("svc #10");

    asm volatile("svc #0");
}