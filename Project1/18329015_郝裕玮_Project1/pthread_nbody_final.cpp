#include<iostream>
#include<cmath>   //公式里需要用到sqrt开根函数
#include<fstream> //该头文件用于读写文件
#include<iomanip> //该头文件用于控制数据精度
#include<sys/time.h> //标准日期时间头文件，用于计算运行时间
#include<pthread.h>
using namespace std;

#define timestep 20     //迭代次数 
#define dT 0.005        //时间间隔
#define G 1             //引力常数    
#define MAX 1024        //各信息的数组大小均设置为1024，与粒子个数相同
#define particles 1024   //数据集中共有1024个粒子

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}
//最后一行将us转换为s，统一单位
//该结构体用于计算程序串行部分运行时间

struct information
{
   double x;
   double y;
   double z;
}pos[MAX],v[MAX];//初始化两个数组，分别为每个粒子的位置和速度
                 //数组内部每个元素存储了对应粒子在x轴，y轴，z轴上的对应分量

long thread_count;         // 并行线程数
int b_thread_count;       // 线程互斥锁计数器
pthread_mutex_t mutex;        // 线程互斥量，也即互斥锁
pthread_cond_t cond_var;      // 线程条件变量

double m[MAX];//每个粒子的质量

void Barrier_init(void){//通过条件变量等待法来同步所有线程
    b_thread_count=0;//计数器初始化为0
    pthread_mutex_init(&mutex,NULL);//初始化互斥锁mutex
    pthread_cond_init(&cond_var,NULL);//初始化条件变量conda_var
 }
 
void Barrier(void){//设置路障
    pthread_mutex_lock(&mutex);//线程调用该函数来获得临界区的访问权
    //互斥量可以用来限制每次只有1个线程能进入临界区。互斥量保证了一个线程独享临界区
    //其他线程在有线程已经进入该临界区的情况下，不能同时进入。
    b_thread_count++;//计数器+1
    if(b_thread_count==thread_count){//说明所有线程均已获得互斥锁
        b_thread_count=0;//计数器清零
        pthread_cond_broadcast(&cond_var);//唤醒所有等待条件变量condavar的线程
        //也即被阻塞在当前锁mutex上的线程会被唤醒。
    }
    else{
        while(pthread_cond_wait(&cond_var, &mutex)!=0);//详见下方补充
    }
    pthread_mutex_unlock(&mutex);//线程退出临界区
}

void* loop_schedule(void* rank){
    long long my_rank=(long long)rank;//如果指针类型的大小和表示进程编号的整数类型不同，在编译时就会受到警告
    //所以我们需要进行强制类型转换
    long long my_n=MAX/thread_count;//计算每个线程中需要分配的循环次数
    long long my_first_i=my_n*my_rank;//计算每个线程中开始计算的第一项
    long long my_last_i=my_first_i+my_n;//计算每个线程中需要计算的最后一项、

    int i,j,k,s;//循环变量
    double x_diff,y_diff,z_diff;//两粒子之间在三条坐标轴上的相对距离
    double dist,dist_cubed;//两粒子之间的绝对距离和绝对距离的立方
    double x_f,y_f,z_f;//两粒子之间作用力在三条坐标轴上的分量

    for(k=1;k<=timestep;k++){//迭代次数为timestep次（20）
        for(i=my_first_i;i<=my_last_i-1;i++){//遍历每个粒子
            x_f=0;
            y_f=0;
            z_f=0;
            //每次遍历该粒子以外的粒子之前需要先将作用力清空，防止与上一个粒子在三个方向上所受的作用力相叠加
            for(j=0;j<=particles-1;j++){//遍历该粒子以外的所有粒子
            //因为该粒子以外的所有粒子都对它有作用力
                if(i==j){//该粒子本身可以跳过
                continue;
                }
                x_diff=pos[i].x-pos[j].x;
                y_diff=pos[i].y-pos[j].y;
                z_diff=pos[i].z-pos[j].z;
                //计算两粒子在三条坐标轴上的相对距离

                dist=sqrt(x_diff*x_diff+y_diff*y_diff+z_diff*z_diff);//计算两粒子之间的绝对距离
                dist_cubed=dist*dist*dist;//计算距离的立方

                x_f-=G*m[i]*m[j]/dist_cubed*x_diff;
                y_f-=G*m[i]*m[j]/dist_cubed*y_diff;
                z_f-=G*m[i]*m[j]/dist_cubed*z_diff;
                //计算两粒子之间作用力在三条坐标轴上的分量
            }
            v[i].x+=dT*x_f/m[i];
            v[i].y+=dT*y_f/m[i];
            v[i].z+=dT*z_f/m[i];
            //利用dv=f*dT/m(动量定理)来更新粒子在三条坐标轴上的速度分量
        }
        Barrier();//设置路障
        for(s=my_first_i;s<=my_last_i-1;s++){//遍历每个粒子
            pos[s].x+=dT*v[s].x;
            pos[s].y+=dT*v[s].y;
            pos[s].z+=dT*v[s].z;
            //利用d=d0+v*dT来更新粒子在三条坐标轴上的坐标
        }
        Barrier();//设置路障
    }
    return NULL;
}

int main()
{ 
    int i,j,k,s;//循环变量

    ifstream myfile("C:\\Users\\93508\\Desktop\\.vscode\\.vscode\\nbody_init.txt");//打开数据集文件
    for(i=0;i<=particles-1;i++){
      myfile>>m[i]>>pos[i].x>>pos[i].y>>pos[i].z>>v[i].x>>v[i].y>>v[i].z;//根据题目要求将每一行的数据写入到各数组对应的结构体成员中
    }

    pthread_t* thread_handles;//pthread_t 用于声明线程ID
    double start,end;//用于计算pthread并行部分的开始时间和结束时间
    long long thread;
        
    thread_count=8;//可在这里选择或修改并行线程数

    Barrier_init();//路障初始化，注意要放在创建线程数组之前！
    thread_handles = (pthread_t*) malloc (thread_count*sizeof(pthread_t));//创建线程数组
   
    GET_TIME(start);
    for(thread=0;thread<thread_count;thread++){
        pthread_create(&thread_handles[thread],NULL,loop_schedule,(void*)thread);//创建线程
        //第一个参数为指向线程标识符的指针。
        //第二个参数用来设置线程属性。
        //第三个参数是线程运行函数的起始地址。
        //最后一个参数是运行函数的参数。
    }
    for(thread=0;thread<thread_count;thread++){
        pthread_join(thread_handles[thread], NULL);
        //函数pthread_join用来等待一个线程的结束。
        //第一个参数为被等待的线程标识符，第二个参数为一个用户定义的指针，它可以用来存储被等待线程的返回值。
        //这个函数是一个线程阻塞的函数，调用它的线程将一直等待到被等待的线程结束为止
        //当函数返回时，被等待线程的资源被收回。
        //也就是说主线程中要是加了这段代码，就会在该代码所处的位置卡住，直到这个线程执行完毕才会继续往下运行。
    }
    GET_TIME(end);//获取串行部分结束时间
    cout<<end-start<<endl;//输出运行时间

    ofstream outfile;
    outfile.open("C:\\Users\\93508\\Desktop\\.vscode\\.vscode\\pthread_result.txt",ios::out);
    //准备将结果写入pthread_result文件中
    for(i=0;i<=particles-1;i++){
      outfile<<setprecision(15)<<m[i]<<' '<<pos[i].x<<' '<<pos[i].y<<' '<<pos[i].z<<' '<<v[i].x<<' '<<v[i].y<<' '<<v[i].z<<endl;
      //将数据保留15位小数并按照要求把每个粒子的数据按顺序写入每行，一行数据代表一个粒子
    }

    myfile.close();
    outfile.close();
    //关闭文件
    system("pause");
}