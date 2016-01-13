/* Moments Example for EC440 HW4

This example generates an array of N data elements then calculates the first 10
moment averages of that array.  It also reports the elapsed time for the computation.

In this case, we take a random distribution of x in the range XMIN <= x < XLIMIT
In theory, as N --> infinity, the expected moments are

moment(k) = (XLIMIT**(k+1)-XMIN**(k+1))/(2*(k+1))

*/

#define N 12000000
#define N_MOMENTS 10
#define N_threads 4

// seed to use for random number generated in main
#define SEED 123
#define XMIN 0.0
#define XLIMIT 2.0

// For the clock_gettime function
#include <time.h>
// for error checking
#include <errno.h>
// for random numbers
#include <stdlib.h>

#include <stdio.h>
#include <pthread.h>
// for power function
#include <math.h>


double x[N]; // the main data array
double moments[N_MOMENTS]; // array for moment sums and for the final answers
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //intializes a mutex
void generate_data(double *destination, int n, double xmin, double xlimit, int seed){
    // fills the destination with random numbers x, xmin<=x<xlimit using random seed
    int i;
    double scale;
    
    srandom(seed);   // set the random number seed so that we can reproduce same data
    scale=(xlimit-xmin)/RAND_MAX; // scale for random number
 
    
    for (i=0;i<n;++i){
        destination[i]= xmin+scale*random();
         //printf("RANDOM NUMBER : %f\n", destination[i]);
    }
}
double time_in_seconds(struct timespec *t){
    // a timespec has integer values for seconds and nano seconds
    return (t->tv_sec + 1.0e-9 * (t->tv_nsec));
}

void *sum_moments(void* arg)// function is called when a new thread is made
{
    // calculate m moment sums for n element array data and place sums in result
    struct timespec threadstart, threadend;
    double term;
    int i,j,start,end;
    int offset = *(int*)arg; // convert arg to int. 
    double local_moments[N_MOMENTS];

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&threadstart);// gets start time of thread 1
    // printf("Thread %d starts at time: %-20.9g sec\n", offset, time_in_seconds(&threadstart));
    start = (N/N_threads)*offset; // where the current thread will start calculations from
    end = (N/N_threads)*(offset+1);// where current thread will end calculations.
     // end = (N/5)+offset; // thread 5

    for (j=0;j<N_MOMENTS;++j) 
    {
        
        local_moments[j]=0; // initialize result

    }

    for (i=start;i<end;++i){ // main loop over data points
        // lock while changing shared resource 
        for (j=0, term=x[i];    // loop over moments, start with term as x[i]
             j<N_MOMENTS;               // continue for m steps
             ++j,term*=x[i])
             {   // increment count and increase the power
            
                 local_moments[j]+= term;   // accumulate result for power j+1
             //printf("This is resultvinny: %f\n", moments[j]);
            
        }
        // unlocks when done using shared resource       
    }

     pthread_mutex_lock(&mutex);
     for (j = 0; j<N_MOMENTS;j++)
     {
        moments[j]+=local_moments[j];
     }
     pthread_mutex_unlock(&mutex);
double final_time;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&threadend);// gets start time of thread 1
    // printf("thread %d: ends at %-20.9g sec\n", offset, time_in_seconds(&threadend));
   final_time = time_in_seconds(&threadend)-time_in_seconds(&threadstart);
    printf("total time of thread %d, is %-20.9g sec\n",offset, final_time );
    pthread_exit(NULL); //exits thread
}

// some helper functions for dealing with times




int main(){
    struct timespec start, finish, resolution, calculation,thread1start,thread1end, thread2start,thread2end, thread3start, thread3end,thread4start,thread4end;  // timing info
    double calculation_time, start_time, finish_time;
    int err,offset1,wastetime=0; // error number for system calls
    pthread_t tid1;
    
    int i;
    
    offset1 = 0;
    err=clock_getres(CLOCK_THREAD_CPUTIME_ID,&resolution);
    if (err){
         perror("Failed to get clock resolution");
        exit(1);
    }    
    printf("Main: thread clock resolution = %-16.9g seconds\n",time_in_seconds(&resolution));
    
    
    generate_data(x,N,XMIN,XLIMIT,SEED); // generate data
    printf("Main: generated %d data values in the range %5.3f to %5.3f, seed=%d\n",
                N, XMIN, XLIMIT, SEED);

    err=clock_gettime(CLOCK_THREAD_CPUTIME_ID,&start);
    if (err){
        perror("Failed to read thread_clock with error = %d\n");
        exit(1);
    }    
    
    pthread_t tid2,tid3,tid4;//,tid5;
    // int offset2 = (N/4), offset3=(N/4)*2,offset4=(N/4)*3; // where each thread starts/ends parallel calculations
    // int offset2 = (N/5), offset3=(N/5)*2,offset4=(N/5)*3,offset5=(N/5)*4;
    int  offset2 = 1,offset3=2,offset4=3;
    printf("Main: calculation start time = %-20.9g seconds\n",time_in_seconds(&start));
    
    //creates a new thread to calculate moment in parallel
    pthread_create(&tid1, NULL, sum_moments, &offset1);
    pthread_create(&tid2,NULL,sum_moments,&offset2);
    pthread_create(&tid3, NULL, sum_moments, &offset3);
    pthread_create(&tid4,NULL,sum_moments,&offset4);
    // pthread_create(&tid5,NULL,sum_moments,&offset5);

    
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread1start);// gets start time of thread 1
    // printf("Thread1 start time = %-20.9g seconds\n",time_in_seconds(&thread1start));
    
    pthread_join(tid1, NULL); // waits for thread 1 to finish
    
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread1end);// gets end time of thread 1
    // printf("Thread1 Finish = %-20.9g seconds\n",time_in_seconds(&thread1end));
    // printf("Thread1 elapsed time = %-20.9g seconds\n", time_in_seconds(&thread1end)-time_in_seconds(&thread1start)); // prints time elapsed of thread 1
   
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread2start); // get start time of thread 2
    // printf("Thread2 start time = %-20.9g seconds\n",time_in_seconds(&thread2start));
    
    pthread_join(tid2,NULL); // waits for thread 2 to finish
    
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread2end); // get end time of thread2
    // printf("Thread2 Finish = %-20.9g seconds\n",time_in_seconds(&thread2end));
    // printf("Thread2 elapsed time = %-20.9g seconds\n", time_in_seconds(&thread2end)-time_in_seconds(&thread2start)); // prints time elapsed of thread 2

    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread3start); // get start time of thread 3
    // printf("Thread3 start time = %-20.9g seconds\n",time_in_seconds(&thread3start));
    
    pthread_join(tid3, NULL); // waits for thread 3 to finish
    
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread3end); // get end time of thread3
    // printf("Thread3 Finish = %-20.9g seconds\n",time_in_seconds(&thread3end));
    // printf("Thread3 elapsed time = %-20.9g seconds\n", time_in_seconds(&thread3end)-time_in_seconds(&thread3start)); // prints time elapsed of thread 3
    
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread4start); // get start time of thread 4
    // printf("Thread4 start time = %-20.9g seconds\n",time_in_seconds(&thread4start));
    
    pthread_join(tid4,NULL);// waits for thread 4 to finish

    // clock_gettime(CLOCK_THREAD_CPUTIME_ID,&thread4end); // get end time of thread4
    // printf("Thread4 Finish = %-20.9g seconds\n",time_in_seconds(&thread4end));
    // printf("Thread4 elapsed time = %-20.9g seconds\n", time_in_seconds(&thread4end)-time_in_seconds(&thread4start)); // prints time elapsed of thread 4

    // pthread_join(tid5,NULL);
  
    for (i=0;i<N_MOMENTS;++i) 
    {
        moments[i] /= N;  // convert sums to averages
        // printf("averages: %f\n", moments[i]);
    }

    // for(i = 0;i<N;i++)
    // {
    //     wastetime++;
    // }
    err=clock_gettime(CLOCK_THREAD_CPUTIME_ID,&finish);
    if (err){
        perror("Failed to read thread_clock with error = %d\n");
        exit(1);
    }    
    printf("Main: calculation finish time = %-20.9g seconds\n",time_in_seconds(&finish));
    
    calculation_time = time_in_seconds(&finish)-time_in_seconds(&start);
    printf("Calculation elapsed time = %-20.9g seconds\n",calculation_time);

    printf("\n==================== MOMENT RESULTS ======================\n");
    printf("%5s %30s %30s\n","m","moment","Expected");
    printf("%5s %30s %30s\n","-","------","--------");
    for(i=0;i<N_MOMENTS;++i){
        printf("%5d %30.16e %30.16e\n",i+1,moments[i],
                0.5*(pow(XLIMIT,i+2)-pow(XMIN,i+2))/(i+2));
    }
}
   
   
    
    
    
