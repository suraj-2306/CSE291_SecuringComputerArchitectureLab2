#include "utility.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <time.h>

#define BUFF_SIZE (1 << 21)
#define THRESHOLD 120
#define NOSETSPROBE 512

int main(int argc, char **argv)
{

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
    uint64_t volatile times[512] = {0};

    int *shared_mem = (int *)huge_page;

    // To flush the shared memory out of the cache
    for (int i = 0; i < NOSETSPROBE; i += 1)
    {
        clflush(shared_mem + i * 64);
    }

    // Victim process to read something from the shared_mem
    int volatile mem_access = 23; // Some number less than 512
    one_block_access(shared_mem + mem_access * 64);

    // Reading the time taken by the attacker to read all the cache lines
    for (int i = 0; i < NOSETSPROBE; i += 1)
        times[i] = measure_one_block_access_time((uint64_t)(shared_mem + i * 64));

    for (int i = 0; i < NOSETSPROBE; i += 1)
        if (times[i] < THRESHOLD)
            printf("Accessed number is %d and access time is %d\n", i, times[i]);
}