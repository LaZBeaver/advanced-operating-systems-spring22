#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
//#include <fcntl.h>


#define PAGE_SIZE (4*1024)
#define THREAD_WRITER_NUMBER 4
#define THREAD_READER_NUMBER 4
#define NOT_A_MILLION 1000000

char *page = NULL;
//char *page_aligned = NULL;
char rd;
char wr ='a';
pthread_barrier_t barrier;

void *thread_allocator()
{
    page = (char*) malloc(PAGE_SIZE);
    if (page == NULL)
    {
        write(STDERR_FILENO, "failed to allocate memory", 
        strlen("failed to allocate memory"));
        exit(EXIT_FAILURE);
    }
    //page_aligned = (char*) (((uintptr_t)page) & ~ (PAGE_SIZE - 1));
}

void *thread_advisor()
{
    for (size_t i = 0; i < NOT_A_MILLION; i++)
    {
        madvise(page, MADV_SEQUENTIAL, 8 * PAGE_SIZE);
        //pthread_barrier_wait(&barrier);
    }
}

void *thread_reader()
{
    for (size_t i = 0; i < NOT_A_MILLION; i++)
    {
        madvise(page, MADV_SEQUENTIAL, 8 * PAGE_SIZE);
        pthread_barrier_wait(&barrier);
        /*
        for (size_t i = 0; i < PAGE_SIZE * 8; i++)
        {
            rd = page[i];
        }
        */
    }
}

void *thread_writer()
{
    for (size_t i = 0; i < NOT_A_MILLION; i++)
    {
        madvise(page, MADV_SEQUENTIAL, 8 * PAGE_SIZE);
        pthread_barrier_wait(&barrier);
        /*
        for (size_t i = 0; i < PAGE_SIZE * 8; i++)
        {
            page[i] = wr;
        }
        */
    }
}

void handler(int sig)
{
    write(2, "SIGSEGV Occured!\n", strlen("SIGSEGV Occured!\n"));
    exit(EXIT_FAILURE);
}

int main()
{
    /*
    cpu_set_t sets [8];
    for (size_t i = 0; i < 8; i++)
    {
        CPU_ZERO(&sets[i]);
        CPU_SET(i, &sets[i]);
    }
    */

    //printf("Number of cpu cores %lu\n", sysconf(_SC_NPROCESSORS_CONF));
    //printf("Number of online cpu cores %lu\n", sysconf(_SC_NPROCESSORS_ONLN));


    struct sigaction sa;
    memset (&sa, '\0', sizeof(sa));
    sa.sa_sigaction = (void*)handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    
    //if (pthread_barrier_init(&barrier, NULL, THREAD_READER_NUMBER+THREAD_WRITER_NUMBER+1) != 0)
    if (pthread_barrier_init(&barrier, NULL, THREAD_READER_NUMBER+THREAD_WRITER_NUMBER) != 0)
    {
        write(STDERR_FILENO, "failed to initiate the barrier", 
        strlen("failed to initiate the barrier"));
        exit(EXIT_FAILURE);
    }

    pthread_t tid_allocator;
    //pthread_t tid_advisor;

    pthread_t tid_readers [THREAD_READER_NUMBER];
    pthread_t tid_writers [THREAD_WRITER_NUMBER];

    //---------------------------------------------------------

    if (pthread_create(&tid_allocator, NULL, thread_allocator, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to create the thread", 
        strlen("failed to create the thread"));
        exit(EXIT_FAILURE);
    }

    if (pthread_join(tid_allocator, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to join the thread", 
        strlen("failed to join the thread"));
        exit(EXIT_FAILURE);
    }

    /*
    if (pthread_create(&tid_advisor, NULL, thread_advisor, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to create the thread", 
        strlen("failed to create the thread"));
        exit(EXIT_FAILURE);
    }
    */

    for (size_t i = 0; i < THREAD_READER_NUMBER; i++)
    {
        if (pthread_create(&tid_readers[i], NULL, thread_reader, NULL) != 0)
        {
            write(STDERR_FILENO, "failed to create the thread", 
            strlen("failed to create the thread"));
            exit(EXIT_FAILURE);
        }

        //sched_setaffinity(tid_readers[i], sizeof(sets[i]), &sets[i]);
    }

    for (size_t i = 0; i < THREAD_WRITER_NUMBER; i++)
    {
        if (pthread_create(&tid_writers[i], NULL, thread_writer, NULL) != 0)
        {
            write(STDERR_FILENO, "failed to create the thread", 
            strlen("failed to create the thread"));
            exit(EXIT_FAILURE);
        }

        //sched_setaffinity(tid_readers[i+THREAD_READER_NUMBER], sizeof(sets[i+THREAD_READER_NUMBER]), &sets[i+THREAD_READER_NUMBER]);
    }
    
    //---------------------------------------------------------

    /*
    if (pthread_join(tid_advisor, NULL) != 0)
    {
        write(STDERR_FILENO, "failed to join the thread", 
        strlen("failed to join the thread"));
        exit(EXIT_FAILURE);
    }
    */

    for (size_t i = 0; i < THREAD_READER_NUMBER; i++)
    {
        if (pthread_join(tid_readers[i], NULL) != 0)
        {
            write(STDERR_FILENO, "failed to join the thread", 
            strlen("failed to join the thread"));
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < THREAD_WRITER_NUMBER; i++)
    {
        if (pthread_join(tid_writers[i], NULL) != 0)
        {
            write(STDERR_FILENO, "failed to join the thread", 
            strlen("failed to join the thread"));
            exit(EXIT_FAILURE);
        }
    }
    
    //---------------------------------------------------------

    pthread_barrier_destroy(&barrier);
    exit(EXIT_SUCCESS);
}





