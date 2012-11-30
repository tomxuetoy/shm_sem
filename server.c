// The code is from link:
// http://blog.csdn.net/lanmanck/article/details/6100019
/*server.c:向共享内存中写入People*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main()
{
    struct People {
        char name[10];
        int age;
    };

    int semid;
    int shmid;
    key_t semkey;
    key_t shmkey;
    semkey=ftok("server.c",0);
    shmkey=ftok("client.c",0);

    /*创建共享内存和信号量的IPC*/
    semid=semget(semkey,1,0666|IPC_CREAT);
    if(semid==-1)
        printf("creat sem is fail/n");
    shmid=shmget(shmkey,1024,0666|IPC_CREAT);
    if(shmid==-1)
        printf("creat shm is fail/n");

    /*设置信号量的初始值，就是资源个数*/
    union semun {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } sem_u;

    sem_u.val=1;
    semctl(semid,0,SETVAL,sem_u);

    /*将共享内存映射到当前进程的地址中，之后直接对进程中的地址addr操作就是对共享内存操作*/

    struct People * addr;
    addr=(struct People*)shmat(shmid,0,0);
    if(addr==(struct People*)-1)
        printf("shm shmat is fail/n");

    /*信号量的P操作*/
    void p() {
        struct sembuf sem_p;
        sem_p.sem_num=0;
        sem_p.sem_op=-1;
        if(semop(semid,&sem_p,1)==-1)
            printf("p operation is fail/n");
    }

    /*信号量的V操作*/
    void v() {
        struct sembuf sem_v;
        sem_v.sem_num=0;
        sem_v.sem_op=1;
        if(semop(semid,&sem_v,1)==-1)
            printf("v operation is fail/n");
    }

    /*向共享内存写入数据*/
    p();
    strcpy((*addr).name,"xiaoming");
    /*注意：①此处只能给指针指向的地址直接赋值，不能在定义一个  struct People people_1;addr=&people_1;因为addr在addr=(struct People*)shmat(shmid,0,0);时,已经由系统自动分配了一个地址，这个地址与共享内存相关联，所以不能改变这个指针的指向，否则他将不指向共享内存，无法完成通信了。
    注意：②给字符数组赋值的方法。刚才太虎了。。*/
    (*addr).age=10;
    v();

    /*将共享内存与当前进程断开*/
    if(shmdt(addr)==-1)
        printf("shmdt is fail/n");
}
