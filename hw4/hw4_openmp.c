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
////���һ�н�usת��Ϊs��ͳһ��λ
//�ýṹ�����ڼ�����̺߳ʹ��м���pi������ʱ��

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
   int n, thread_count;//����Ԫ�ظ������߳���
   int *a, *copy;//������a�����������ս�����Ƶ�copy��
   double start, stop;//���ڼ�������ʱ��
   
   /* please choose terms 'n', and the threads 'thread_count' here. */
   n = 10;
   thread_count = 4;

   /* You can also get number of threads from command line */
   //if (argc != 3) Usage(argv[0]);
   //Get_args(argv, &thread_count, &n);
   
   /* Allocate storage and generate data for a */
   a = (int *)malloc(n*sizeof(int));//����n��Ԫ�ص�int������a
   Gen_data(a, n);//Ϊa������������
   
   /* Allocate storage for copy */
   copy = (int *)malloc(n*sizeof(int));//����n��Ԫ�ص�int������copy
   
   /* Serial count sort */
   memcpy(copy, a, n*sizeof(int));//������õ�����a�Ľ�����Ƶ�copy��
#  ifdef DEBUG   
   Print_data(copy, n, "Original: Serial sort a");
#  endif
   GET_TIME(start);//�õ����м����������еĿ�ʼʱ��
   Count_sort_serial(copy, n);//���м�������
   GET_TIME(stop);//�õ����м����������еĽ���ʱ��
#  ifdef DEBUG   
   Print_data(copy, n, "Sorted: Serial sort a");
#  endif
   if (!Check_sort(copy, n))//��������Ƿ�ɹ�
      printf("Serial sort failed\n");
   printf("Serial run time: %e\n\n", stop-start);//��ӡ���м�����������ʱ��

   /* Parallel count sort */
   memcpy(copy, a, n*sizeof(int));//������õ�����a�Ľ�����Ƶ�copy��
#  ifdef DEBUG   
   Print_data(copy, n, "Original: Parallel qsort a");
#  endif
   GET_TIME(start);//�õ����м����������еĿ�ʼʱ��
   Count_sort_parallel(copy, n, thread_count);//���м�������
   GET_TIME(stop);//�õ����м����������еĽ���ʱ��
#  ifdef DEBUG   
   Print_data(copy, n, "Sorted: Parallel sort a");
#  endif
   if (!Check_sort(copy, n))//��������Ƿ�ɹ�
      printf("Parallel sort failed\n");
   printf("Parallel run time: %e\n\n", stop-start);//��ӡ���м�����������ʱ��   
   
   /* qsort library */
   memcpy(copy, a, n*sizeof(int));//������õ�����a�Ľ�����Ƶ�copy��
#  ifdef DEBUG   
   Print_data(copy, n, "Original: Library qsort a");
#  endif   
   GET_TIME(start);//�õ��������еĿ�ʼʱ��
   Library_qsort(copy, n);//����
   GET_TIME(stop);//�õ��������еĽ���ʱ��
#  ifdef DEBUG   
   Print_data(copy, n, "Sorted: Library qsort a");
#  endif
   if (!Check_sort(copy, n))//��������Ƿ�ɹ�
      printf("Library sort failed\n");
   printf("qsort run time: %e\n", stop-start);//��ӡ��������ʱ�� 

   free(a);//�ͷ�a����
   free(copy);//�ͷ�copy����
   
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

void Gen_data(int a[], int n) {//������Ҫ���������
   int i;
   
   for (i = 0; i < n; i++)
      a[i] = random() % n + 1; // (double) RAND_MAX;
      //�������n������

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

void Count_sort_serial(int a[], int n) {//��Count_sort_parallel���ݼ���һ�£����ٽ��͡�
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
   int i,j,count;//i,j����ѭ��
   //count���ڼ�¼ÿ��������Ԫ�ص�λ�ã�ͨ����¼�ж��ٸ��ȴ�����Ԫ��С�������λ��С�ڴ�����Ԫ�ص�Ԫ�ظ�����
   int* temp=(int *)malloc(n*sizeof(int));//����n��Ԫ�ص�int������temp
   
#  pragma omp parallel num_threads(thread_count) default(none) \
      private(i, j, count) shared(n, a, temp) 
      //pragma�Ľ��ͷ��ڴ�����������ֽ���  
   {
#     pragma omp for
      //����omp forͬ�����ڴ�����������ֽ���
      for(i=0;i<=n-1;i++){//��ÿ��Ԫ�ض�����һ�μ�������
         count=0;//ÿ������Ҫ��count����
         //�Ա�ͳ���ж��ٸ��ȴ�����Ԫ��С�������λ��С�ڴ�����Ԫ�ص�Ԫ�ظ���
         for(j=0;j<=n-1;j++){//��ʼͳ���ж��ٸ��ȴ�����Ԫ��С�������λ��С�ڴ�����Ԫ�ص�Ԫ�ظ���
            if(a[j]<a[i]||(a[j]==a[i]&&j<i)){//��Ҫ�������������
            //�ȴ�����Ԫ��С�������λ��С�ڴ�����Ԫ��
               count++;//����+1
            }
         }
         temp[count]=a[i];//��������Ԫ�ص���ȷλ��ͨ��count���õ�temp������
      }
#     pragma omp for
      for(i=0;i<=n-1;i++){
         a[i]=temp[i];//���ź��������temp���ݸ��Ƶ�����a��
         //�ò��У�omp for�����Դ��м��������е�memcpy(a, temp, n*sizeof(int))����ʱ���Ч���ϵ��Ż�
      }   
   }
   free(temp);//�ͷ�temp��������Ŀռ�
}  /* Count_sort_parallel */

/*---------------------------------------------------------------------
 * Function:     Library_qsort
 * Purpose:      sort elements in an array using qsort library function
 * In args:      n: number of elements
 * In/out arg:   a: array of elements
 */

void Library_qsort(int a[], int n) {
   qsort(a, n, sizeof(int), My_compare);//���ź���
   //����ԭ��Ϊ��void qsort(void* base,size_t num,size_t size,int (*compar)(const void*,const void*));
   //void* base��ָ��Ҫ���������ĵ�һ�������ָ�룬����ת��Ϊvoid*��
   //size_t num��baseָ���������Ԫ�ص�������size_t ���޷����������͡�
   //size_t size��������ÿ��Ԫ�صĴ�С�����ֽ�Ϊ��λ����size_t ���޷����������͡�
   //int (*compar)(const void*,const void*)��ָ��Ƚ�����Ԫ�صĺ�����ָ�롣
   //�ú������ظ�����qsort�Ƚ�����Ԫ�ء���Ӧ��ѭ����ԭ�ͣ�
   //int compar (const void* p1, const void* p2);
   //<0	p1�ŵ�p2֮ǰ
   //=0	p1,p2λ�ò���
   //>0	p1�ŵ�p2֮��


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
   
   return (*int_a - *int_b);//���庬��ɼ�Library_qsort�����еĽ���
}  /* My_compare */


/*---------------------------------------------------------------------
 * Function:  Print_data
 * Purpose:   print an array
 * In args:   a: array of elements
 *            n: number of elements
 *            msg: name of array
 */

void Print_data(int a[], int n, char msg[]) {//debugʱ��ӡ���������Ϣ
   int i;

   printf("%s = ", msg);//��ӡ������
   for (i = 0; i < n; i++)
      printf("%d ", a[i]);//��˳���ӡ����Ԫ��
   printf("\n");
}  /* Print_data */


/*---------------------------------------------------------------------
 * Function:  Check_sort
 * Purpose:   Determine whether an array is sorted
 * In args:   a: array of elements
 *            n: number of elements
 * Ret val:   true if sorted, false if not sorted
 */

int  Check_sort(int a[], int n) {//����Ƿ�����ɹ�
   int i;

   for (i = 1; i < n; i++)
      if (a[i-1] > a[i]) return 0;//��a[i-1]>a[i]��֤����������ʧ��
   return 1;
}  /* Check_sort */
