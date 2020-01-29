#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

typedef long long int ll;


void swap(int *b , int *c)
{
	int temp = *b;
	*b = *c;
	*c = temp;
}

int partition(int *a , int start , int end)
{
    int piv = start + (rand() % (start - end + 1));
    swap(&a[piv] , &a[end]);
	int i,pivtindx = start,pivot = a[end];

	for(i=start;i<end;i++)
	{
		if(a[i] <= pivot)
		{
			swap(&a[i] , &a[pivtindx]);
			pivtindx++;
		}
	}

	swap(&a[pivtindx],&a[end]);
	return(pivtindx);
}

int * sharedmemory(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int*)shmat(shm_id, NULL, 0);
}

typedef struct argument
{
    int l;
    int r;
    int* arr;

}argument;

void normal_quicksort(int *crr, int l, int r)
{
    if(l>r)
    return;

    if(r-l+1 < 5)
    {   
        for(int i=l+1;i<=r;i++)
        {

            int x = crr[i];
            int j;

            for(j = i-1;j>=l && crr[j]>x;j--)
            {
                crr[j+1] = crr[j];
            }

            crr[j+1] = x;
        }
        return;
    }

    ll pivotindx = partition(crr, l, r);
    normal_quicksort(crr,l,pivotindx-1);
    normal_quicksort(crr,pivotindx+1,r);
    return;
}

void multiprocess_quicksort(int *arr, int l, int r)
{
    if(l>r)
    return;

    if(r-l+1 < 5)
    {
        for(int i=l+1;i<=r;i++)
        {

            int x = arr[i];
            int j;

            for(j = i-1;j>=l && arr[j]>x;j--)
            {
                arr[j+1] = arr[j];
            }

            arr[j+1] = x;
        }

        return;
    }


    ll pivotindx = partition(arr, l, r);
    int pid1 = fork();
    int pid2;

    if(pid1 == 0)
    {
        multiprocess_quicksort(arr,l,pivotindx-1);
        _exit(0);
    }
    else
    {
        pid2 = fork();
        if(pid2 == 0)
        {
    		multiprocess_quicksort(arr,pivotindx+1,r);
            _exit(0);
        
        }
        else
        {
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
        }
        
    }
    
    return;
}

void *threaded_quicksort(void* a)
{
    argument *args = (argument*) a;

    int l = args->l;
    int r = args->r;
    int *brr = args->arr;

    if(l>r)
    return NULL;

    if(r-l+1 < 5)
    {
        for(int i=l+1;i<=r;i++)
        {

            int x = brr[i];
            int j;

            for(j = i-1;j>=l && brr[j]>x;j--)
            {
                brr[j+1] = brr[j];
            }

            brr[j+1] = x;
        }

        return NULL;
    }

    ll pivotindx = partition(brr, l, r);

    argument a1;
    a1.l = l;
    a1.r = pivotindx - 1;
    a1.arr = brr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threaded_quicksort, &a1);

    argument a2;
    a2.l = pivotindx + 1;
    a2.r = r;
    a2.arr = brr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, threaded_quicksort, &a2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return NULL;
}

void sort(ll n)
{
    struct timespec ts;

    int *arr = sharedmemory(sizeof(int)*(n+1));
    int brr[n+1] , crr[n+1];
    for(int i=0;i<n;i++)
    {
        scanf("%d",&arr[i]);
        brr[i] = arr[i];
        crr[i] = arr[i];
    }
    long double start , end;

    // multiprocess quicksort
    printf("Running concurrent_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start = ts.tv_nsec/(1e9)+ts.tv_sec;

    multiprocess_quicksort(arr, 0 , n-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("Time: %Lf\n",end-start);
    long double time1 = end - start;

    for(int i=0;i<n;i++)
    {
        printf("%d ",arr[i]);
    }
    printf("\n");

    //multithreaded quicksort
    pthread_t tid;
    argument a;
    a.l = 0;
    a.r = n-1;
    a.arr = brr;
    printf("Running threaded_concurrent_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start = ts.tv_nsec/(1e9)+ts.tv_sec;

    pthread_create(&tid, NULL, threaded_quicksort, &a);
    pthread_join(tid, NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("Time: %Lf\n",end-start);
    long double time2 = end - start;

    for(int i=0;i<n;i++)
    {
        printf("%d ",brr[i]);
    }
    printf("\n");

    //normal quicksort
    printf("Running normal_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start = ts.tv_nsec/(1e9)+ts.tv_sec;

    normal_quicksort(crr, 0, n-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("TIme: %Lf\n",end-start);
    long double time3 = end - start;

    for(int i=0;i<n;i++)
    {
        printf("%d ",crr[i]);
    }
    printf("\n");

    printf("normal_quicksort ran:\n\t[ %Lf ] times faster than concurrent_quicksort\n\t[ %Lf ] times faster than threaded_concurrent_quicksort\n\n\n", time1/time3, time2/time3);
    shmdt(arr);
    return;
}

int main()
{
    ll n;
    scanf("%lld",&n);
    sort(n);
    return 0;
}