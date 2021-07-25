/* File:      hw4_openmp.c
 * Purpose:   Implementation of parallel count sort
 *
 * Compile:   gcc -g -Wall -fopenmp -o hw4_openmp hw4_openmp.c
 *
 * Run:      ./hw4_openmp <thread_count> <n>
 *
 * Input:   none
 * Output:  runtime of serial count sort
 *          runtime of parallel count sort
 *          runtime of qsort library function
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>


#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}
////最后一行将us转换为s，统一单位
//该结构体用于计算多线程和串行计算pi的运行时间

void Usage(char prog_name[]);
void Get_args(char* argv[], int* thread_count_p, int* n_p);
void Gen_data(int a[], int n);
void Count_sort_serial(int a[], int n);
void Count_sort_parallel(int a[], int n, int thread_count);
void Library_qsort(int a[], int n);
int  My_compare(const void* a, const void* b);
void Print_data(int a[], int n, char msg[]);
int  Check_sort(int a[], int n);

int main(int argc, char* argv[]) {
   int n, thread_count;//数组元素个数和线程数
   int *a, *copy;//对数组a进行排序，最终结果复制到copy中
   double start, stop;//用于计算运行时间
   
   /* please choose terms 'n', and the threads 'thread_count' here. */
   n = 10;
   thread_count = 4;

   /* You can also get number of threads from command line */
   //if (argc != 3) Usage(argv[0]);
   //Get_args(argv, &thread_count, &n);
   
   /* Allocate storage and generate data for a */
   a = (int *)malloc(n*sizeof(int));//生成n个元素的int型数组a
   Gen_data(a, n);//为a数组生成数据
   
   /* Allocate storage for copy */
   copy = (int *)malloc(n*sizeof(int));//生成n个元素的int型数组copy
   
   /* Serial count sort */
   memcpy(copy, a, n*sizeof(int));//将排序好的数组a的结果复制到copy中
#  ifdef DEBUG   
   Print_data(copy, n, "Original: Serial sort a");
#  endif
   GET_TIME(start);//得到串行计数排序运行的开始时间
   Count_sort_serial(copy, n);//串行计数排序
   GET_TIME(stop);//得到串行计数排序运行的结束时间
#  ifdef DEBUG   
   Print_data(copy, n, "Sorted: Serial sort a");
#  endif
   if (!Check_sort(copy, n))//检查排序是否成功
      printf("Serial sort failed\n");
   printf("Serial run time: %e\n\n", stop-start);//打印串行计数排序运行时间

   /* Parallel count sort */
   memcpy(copy, a, n*sizeof(int));//将排序好的数组a的结果复制到copy中
#  ifdef DEBUG   
   Print_data(copy, n, "Original: Parallel qsort a");
#  endif
   GET_TIME(start);//得到并行计数排序运行的开始时间
   Count_sort_parallel(copy, n, thread_count);//并行计数排序
   GET_TIME(stop);//得到并行计数排序运行的结束时间
#  ifdef DEBUG   
   Print_data(copy, n, "Sorted: Parallel sort a");
#  endif
   if (!Check_sort(copy, n))//检查排序是否成功
      printf("Parallel sort failed\n");
   printf("Parallel run time: %e\n\n", stop-start);//打印并行计数排序运行时间   
   
   /* qsort library */
   memcpy(copy, a, n*sizeof(int));//将排序好的数组a的结果复制到copy中
#  ifdef DEBUG   
   Print_data(copy, n, "Original: Library qsort a");
#  endif   
   GET_TIME(start);//得到快排运行的开始时间
   Library_qsort(copy, n);//快排
   GET_TIME(stop);//得到快排运行的结束时间
#  ifdef DEBUG   
   Print_data(copy, n, "Sorted: Library qsort a");
#  endif
   if (!Check_sort(copy, n))//检查排序是否成功
      printf("Library sort failed\n");
   printf("qsort run time: %e\n", stop-start);//打印快排运行时间 

   free(a);//释放a数组
   free(copy);//释放copy数组
   
   return 0;
}  /* main */

/*---------------------------------------------------------------------
 * Function:  Usage 
 * Purpose:   Print a message showing how to run the program and quit
 * In arg:    prog_name:  the name of the program from the command line
 */
void Usage(char prog_name[]) {
   fprintf(stderr, "usage: %s <thread_count> <n>\n", prog_name);
   exit(0);
}  /* Usage */


/*---------------------------------------------------------------------
 * Function:  Get_args
 * Purpose:   Get the command line arguments
 * In arg:    argv:  strings from command line
 * Out args:  thread_count_p: number of threads
 *            n_p: number of elements
 */
void Get_args(char* argv[], int* thread_count_p, int* n_p) {
   *thread_count_p = strtol(argv[1], NULL, 10);
   *n_p = strtol(argv[2], NULL, 10);
}  /* Get_args */


/*---------------------------------------------------------------------
 * Function:  Gen_data
 * Purpose:   Generate random ints in the range 1 to n
 * In args:   n: number of elements
 * Out arg:   a: array of elements
 */

void Gen_data(int a[], int n) {//生成需要排序的数据
   int i;
   
   for (i = 0; i < n; i++)
      a[i] = random() % n + 1; // (double) RAND_MAX;
      //随机生成n个数据

#  ifdef DEBUG
   Print_data(a, n, "a");
#  endif
}  /* Gen_data */


/*---------------------------------------------------------------------
 * Function:     Count_sort_serial
 * Purpose:      sort elements in an array using count sort
 * In args:      n: number of elements
 * In/out arg:   a: array of elements
 */

void Count_sort_serial(int a[], int n) {//与Count_sort_parallel内容几乎一致，不再解释。
   int i, j, count;
   int* temp = (int *)malloc(n*sizeof(int));
   
   for (i = 0; i < n; i++) {
      count = 0;
      for (j = 0; j < n; j++) 
         if (a[j] < a[i])
            count++;
         else if (a[j] == a[i] && j < i)
            count++;
      temp[count] = a[i];
   }
   
   memcpy(a, temp, n*sizeof(int));
   free(temp);
}  /* Count_sort_serial */


void Count_sort_parallel(int a[], int n, int thread_count) {
   int i,j,count;//i,j用于循环
   //count用于记录每次需排序元素的位置（通过记录有多少个比待排序元素小或相等且位置小于待排序元素的元素个数）
   int* temp=(int *)malloc(n*sizeof(int));//生成n个元素的int型数组temp
   
#  pragma omp parallel num_threads(thread_count) default(none) \
      private(i, j, count) shared(n, a, temp) 
      //pragma的解释放在代码外进行文字解释  
   {
#     pragma omp for
      //对于omp for同样放在代码外进行文字解释
      for(i=0;i<=n-1;i++){//把每个元素都进行一次计数排序
         count=0;//每次排序都要将count清零
         //以便统计有多少个比待排序元素小或相等且位置小于待排序元素的元素个数
         for(j=0;j<=n-1;j++){//开始统计有多少个比待排序元素小或相等且位置小于待排序元素的元素个数
            if(a[j]<a[i]||(a[j]==a[i]&&j<i)){//需要计数的两种情况
            //比待排序元素小或相等且位置小于待排序元素
               count++;//计数+1
            }
         }
         temp[count]=a[i];//将待排序元素的正确位置通过count放置到temp数组中
      }
#     pragma omp for
      for(i=0;i<=n-1;i++){
         a[i]=temp[i];//将排好序的数组temp内容复制到数组a中
         //用并行（omp for）来对串行计数排序中的memcpy(a, temp, n*sizeof(int))进行时间和效率上的优化
      }   
   }
   free(temp);//释放temp数组申请的空间
}  /* Count_sort_parallel */

/*---------------------------------------------------------------------
 * Function:     Library_qsort
 * Purpose:      sort elements in an array using qsort library function
 * In args:      n: number of elements
 * In/out arg:   a: array of elements
 */

void Library_qsort(int a[], int n) {
   qsort(a, n, sizeof(int), My_compare);//快排函数
   //函数原型为：void qsort(void* base,size_t num,size_t size,int (*compar)(const void*,const void*));
   //void* base：指向要排序的数组的第一个对象的指针，将其转换为void*。
   //size_t num：base指向的数组中元素的数量，size_t 是无符号整数类型。
   //size_t size：数组中每个元素的大小（以字节为单位），size_t 是无符号整数类型。
   //int (*compar)(const void*,const void*)：指向比较两个元素的函数的指针。
   //该函数被重复调用qsort比较两个元素。它应遵循以下原型：
   //int compar (const void* p1, const void* p2);
   //<0	p1放到p2之前
   //=0	p1,p2位置不变
   //>0	p1放到p2之后


}  /* Library_qsort */

/*---------------------------------------------------------------------
 * Function:     My_compare
 * Purpose:      compare integer elements for use with qsort function
 * In args:      element a, element b
 * Return val:   positive if a > b, negative if b > a, 0 if equal
 */
int My_compare(const void* a, const void* b) {
   const int* int_a = (const int*) a;
   const int* int_b = (const int*) b;
   
   return (*int_a - *int_b);//具体含义可见Library_qsort函数中的解释
}  /* My_compare */


/*---------------------------------------------------------------------
 * Function:  Print_data
 * Purpose:   print an array
 * In args:   a: array of elements
 *            n: number of elements
 *            msg: name of array
 */

void Print_data(int a[], int n, char msg[]) {//debug时打印数组相关信息
   int i;

   printf("%s = ", msg);//打印数组名
   for (i = 0; i < n; i++)
      printf("%d ", a[i]);//按顺序打印数组元素
   printf("\n");
}  /* Print_data */


/*---------------------------------------------------------------------
 * Function:  Check_sort
 * Purpose:   Determine whether an array is sorted
 * In args:   a: array of elements
 *            n: number of elements
 * Ret val:   true if sorted, false if not sorted
 */

int  Check_sort(int a[], int n) {//检查是否排序成功
   int i;

   for (i = 1; i < n; i++)
      if (a[i-1] > a[i]) return 0;//若a[i-1]>a[i]则证明升序排序失败
   return 1;
}  /* Check_sort */
