// This code is under MIT licence
// you can find the complete file in project-root / LICENSE.txt

// sudo valgrind  --track-origins=yes --tool=memcheck --leak-check=full --show-reachable=yes ./nGen
//sudo valgrind  --track-origins=yes --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes --dsymutil=yes --ignore-range-below-sp=1024-1 --vgdb=yes --vgdb-error=0 ./nGen
//sudo gdbserver localhost:2001 ./nGen
//ddd --debugger "/usr/local/bin/arm-linux-gnueabihf-gdb" --eval-command="target remote SCOUT-06:2001"


// Dependencies
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>



// zero-mq based communication library
//#include <nEuron.h>

// Agent libraries
#include <agents/Platform.h>
#include <agents/Control.h>
#include <agents/AutoPilot.h>
#include <agents/Guidance.h>
#include <agents/Navigation.h>
#include <agents/Payload.h>


/** Interval constants - Develop Frequency.
 *  There are 1M microseconds per second - 1000 micros * 1000 millis
 *  To obtain frequency, divide 1M / frequency in hz
 *  i.e. 1M / 400hz = 2500 micros interval (distance between 2 points in time)
 *  need to multiply by 1000 since we will be comparing to nanoseconds
 *  Frequencies are harmonic
 *  this simply allows us to match interval to frequency easily
 */
const int INTERVAL_400_HZ = 2500 * 1000;
const int INTERVAL_200_HZ = 2500 * 1000;
const int INTERVAL_100_HZ = 10000 * 1000;
const int INTERVAL_50_HZ  = 20000 * 1000;
const int INTERVAL_10_HZ  = 100000 * 1000;
const int INTERVAL_5_HZ  = 200000 * 1000;
const int INTERVAL_1_HZ  = 1000000 * 1000;

// nano seconds in a second!
const long BILLION = 1000000000L;
const long DISPATCHER_SLEEP_TIME = 100000L;


/** Priority Group Constants - use RT PREMEEMPT priorities.
 *  RT priorities are 0-99 low to high
 *  Kernel sits at ~50
 *  we always leave highest priority available for intervention - 99
 *  dispatcher has to run at at a level above all scheduled agents - 98
 *  resulting in groups 70, 75,  80, 85, 90 for now
 *  TODO: automate with sched_get_priority_max and sched_get_priority_min
 *  TODO: look at using SCHED_DEADLINE to push dispatching to kernel
 */
const int PRIORITY_TOP  = 98;
const int PRIORITY_1    = 90;
const int PRIORITY_2    = 85;
const int PRIORITY_3    = 80;
const int PRIORITY_4    = 75;
const int PRIORITY_5    = 70;



// instantiation of agents
//Platform*   agnt_Platform = new Platform();
//Control*    agnt_Control = new Control();
//AutoPilot*  agnt_AutoPilot = new AutoPilot();
//Guidance*   agnt_Guidance = new Guidance();
Navigation* agnt_Navigation = new Navigation();
//Payload*    agnt_Payload_1 = new Payload();

// Prototypes for operational funtions
void dispatch_agents();
int build_and_depoly_thread(int, void* (*)(void*));
//void* dispatch_400hz(void*);
//void* dispatch_200hz(void*);
//void* dispatch_100hz(void*);
//void* dispatch_50hz(void*);
//void* dispatch_10hz(void*);
//void* dispatch_1hz(void*);
void* dispatch_5hz(void*);


int main()
{
//    // for nanosleep parameters
//    struct timespec tim = {0};
//    tim.tv_sec = 1;
//    tim.tv_nsec = 0;
//    
    dispatch_agents();

    while(1)
    {
        sleep(1);//nanosleep(&tim, (struct timespec *)NULL);
    }

    return 0;
}

// Parent dispatcher that runs
// dispatchers for each cycle group
void dispatch_agents()
{
    // (thread rt-priority, function to run in thread)
    //ret = build_and_depoly_thread(C_priority_1, dispatch_400hz);
    //ret = build_and_depoly_thread(C_priority_2, dispatch_200hz);
    //ret = build_and_depoly_thread(C_priority_3, dispatch_100hz);
    //build_and_depoly_thread(C_priority_4, dispatch_50hz);
    //ret = build_and_depoly_thread(C_priority_5, dispatch_10hz);
    build_and_depoly_thread(PRIORITY_3, dispatch_5hz);
    //ret = build_and_depoly_thread(C_priority_5, dispatch_1hz);
    

}

int build_and_depoly_thread(int prio, void* (*thread_func)(void* arg))
{
    // prioritize and launch dispatcher in "loaded" state
    int retval = 0;
//    cpu_set_t cpuset;
    pthread_t dispatch_thread;
    pthread_attr_t attr;
    struct sched_param parm;
    pthread_attr_init(&attr);
    pthread_attr_getschedparam(&attr, &parm);
    parm.sched_priority = (prio)-1;
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &parm);
//    CPU_ZERO(&cpuset);
//    CPU_SET(3,&cpuset);
//    pthread_setaffinity_np(dispatch_thread, sizeof(cpu_set_t), &cpuset);
    retval = pthread_create(&dispatch_thread, &attr,
                            thread_func, NULL);
    if(retval ==0)
    {
        pthread_setschedparam(dispatch_thread, SCHED_FIFO, &parm);
    }
    // TODO: need to handle if agent not dispatched
    return retval;
}

//void* dispatch_400hz(void* arg)
//{
//    std::cout << "Dispatch 400 thread:\n" << std::endl;
//    // for nanosleep parameters
//    struct timespec tim = {0};
//    tim.tv_sec = 0;
//    tim.tv_nsec = 100000L;
//
//    struct timespec t_now, t_lasttick;
//
//    while(1)
//    {
//        // for interval (time-passed) calc
//        uint64_t diff = 0;
//        clock_gettime(CLOCK_MONOTONIC, &t_now);
//        diff = BILLION * (t_now.tv_sec - t_lasttick.tv_sec) + t_now.tv_nsec - t_lasttick.tv_nsec;
//        if(diff >= INTERVAL_400_HZ)
//        {
//            agnt_Control.cycle(400);
//            t_lasttick = t_now;
//        }
//        if(diff >= DISPATCHER_SLEEP_TIME)
//        {
//            nanosleep(&tim, (struct timespec *)NULL);
//        }
//    }
//
//}

//void* dispatch_200hz(void* arg)
//{
//    std::cout << "Dispatch 200 thread:\n" << std::endl;
//    // for nanosleep parameters
//    struct timespec tim = {0};
//    tim.tv_sec = 0;
//    tim.tv_nsec = 100000L;
//
//    struct timespec t_now, t_lasttick;
//    while(1)
//    {
//        // for interval (time-passed) calc
//        uint64_t diff = 0;
//        clock_gettime(CLOCK_MONOTONIC, &t_now);
//        diff = BILLION * (t_now.tv_sec - t_lasttick.tv_sec) + t_now.tv_nsec - t_lasttick.tv_nsec;
//        if(diff >= INTERVAL_200_HZ)
//        {
//            agnt_AutoPilot.cycle(200);
//            agnt_Payload_1.cycle(200);
//
//            t_lasttick = t_now;
//        }
//        if(diff >= DISPATCHER_SLEEP_TIME)
//        {
//            nanosleep(&tim, (struct timespec *)NULL);
//        }
//    }
//}

//void* dispatch_100hz(void* arg)
//{
//    std::cout << "Dispatch 100 thread:\n" << std::endl;
//    // for nanosleep parameters
//    struct timespec tim = {0};
//    tim.tv_sec = 0;
//    tim.tv_nsec = 100000L;
//
//    struct timespec t_now, t_lasttick;
//    while(1)
//    {
//        // for interval (time-passed) calc
//        uint64_t diff = 0;
//        clock_gettime(CLOCK_MONOTONIC, &t_now);
//        diff = BILLION * (t_now.tv_sec - t_lasttick.tv_sec) + t_now.tv_nsec - t_lasttick.tv_nsec;
//        if(diff >= INTERVAL_100_HZ)
//        {
//            agnt_Guidance.cycle(100);
//            t_lasttick = t_now;
//        }
//        if(diff >= DISPATCHER_SLEEP_TIME)
//        {
//            nanosleep(&tim, (struct timespec *)NULL);
//        }
//    }
//}

//void* dispatch_50hz(void* arg)
//{
//    std::cout << "Dispatch 50 thread:\n" << std::endl;
//    // for nanosleep parameters
//    struct timespec tim = {0, 100000L};
////    tim.tv_sec = 0;
////    tim.tv_nsec = 100000L;
//
//    
//    struct timespec  t_now  = {0,0};
//    struct timespec  t_lasttick = {0,0};
//    while(1)
//    {   
//        // for interval (time-passed) calc
//        uint64_t diff = 0;
//        clock_gettime(CLOCK_MONOTONIC, &t_now);
//        diff = BILLION * (t_now.tv_sec - t_lasttick.tv_sec) + t_now.tv_nsec - t_lasttick.tv_nsec;
//        if(diff >= INTERVAL_50_HZ)
//        {
//            agnt_Navigation->cycle(50);
//            t_lasttick = t_now;
//        }
//        if(diff >= DISPATCHER_SLEEP_TIME)
//        {
//            nanosleep(&tim, (struct timespec *)NULL);
//        }
//    }
//}

//void* dispatch_10hz(void* arg)
//{
//    std::cout << "Dispatch 10 thread:\n" << std::endl;
//    // for nanosleep parameters
//    struct timespec tim = {0};
//    tim.tv_sec = 0;
//    tim.tv_nsec = 100000L;
//
//    struct timespec t_now, t_lasttick;
//    while(1)
//    {
//        // for interval (time-passed) calc
//        uint64_t diff = 0;   
//        clock_gettime(CLOCK_MONOTONIC, &t_now);
//        diff = BILLION * (t_now.tv_sec - t_lasttick.tv_sec) + t_now.tv_nsec - t_lasttick.tv_nsec;
//        if(diff >= INTERVAL_10_HZ)
//        {
//            agnt_Navigation.cycle(10);
//            agnt_Payload_1.cycle(10);
//            agnt_Control.cycle(10);
//
//            t_lasttick = t_now;
//        }
//        if(diff >= DISPATCHER_SLEEP_TIME)
//        {
//            nanosleep(&tim, (struct timespec *)NULL);
//        }
//
//    }
//}

//void* dispatch_1hz(void* arg)
//{
//    std::cout << "Dispatch 1 thread:\n" << std::endl;
//    // for nanosleep parameters
//    struct timespec tim = {0};
//    tim.tv_sec = 0;
//    tim.tv_nsec = 100000L;
//
//    struct timespec t_now, t_lasttick;
//    while(1)
//    {
//        // for interval (time-passed) calc
//        uint64_t diff = 0;
//        clock_gettime(CLOCK_MONOTONIC, &t_now);
//        diff = BILLION * (t_now.tv_sec - t_lasttick.tv_sec) + t_now.tv_nsec - t_lasttick.tv_nsec;
//        if(diff >= INTERVAL_1_HZ)
//        {
//            agnt_Platform.cycle(1); // send heartbeat out etc.
//
//            t_lasttick = t_now;
//        }
//        if(diff >= DISPATCHER_SLEEP_TIME)
//        {
//            nanosleep(&tim, (struct timespec *)NULL);
//        }
//
//    }
//
//}
void* dispatch_5hz(void* arg)
{
    std::cout << "Dispatch 5 thread:\n" << std::endl;
    // for nanosleep parameters
    struct timespec tim = {0, 100000L};
//    tim.tv_sec = 0;
//    tim.tv_nsec = 100000L;

    
    struct timespec  t_now  = {0,0};
    struct timespec  t_lasttick = {0,0};
    while(1)
    {   
        // for interval (time-passed) calc
        uint64_t diff = 0;
        clock_gettime(CLOCK_MONOTONIC, &t_now);
        diff = BILLION * (t_now.tv_sec - t_lasttick.tv_sec) + t_now.tv_nsec - t_lasttick.tv_nsec;
        if(diff >= INTERVAL_5_HZ)
        {
            agnt_Navigation->cycle(5); //, nSync pointer); // maybe send pointer to nSync here as arg
            t_lasttick = t_now;
        }
        if(diff >= DISPATCHER_SLEEP_TIME)
        {
            nanosleep(&tim, (struct timespec *)NULL);
        }
    }
}