//user/primes.c
#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

//筛选质数，接受父进程(左邻居)的管道参数
void sieve(int pleft[2]){
    //先读取第一个数，这个数一定为质数，若p != 1, 输出
    int p;
    read(pleft[0], &p, sizeof(p));
    //p = -1是手动设置的数组结尾，当p = -1时代表数据数组结束
    if(p == -1){
        exit(0);
    }
    printf("prime %d\n", p);

    //创建新管道
    int pright[2];
    pipe(pright);
    //这里的文件描述符相互独立，在子进程中关闭pip[1]写端，不影响父进程使用pip[1]写数字
    //这是因为管道，有一个引用计数，只有当所有指向该管道的读/写端的文件描述符都关闭时，管道才会真正关闭，相当于子进程只是减少了管道的引用计数
    if(fork() == 0){    //创建子进程(右邻居)
    //这里首先将pright[1]关闭，通过函数sieve将pright传递给子进程后
    //pright 就是 右邻居的 pleft，而对于每个进程来说，pleft只需要用到读端，所以提前将写端关闭
        close(pright[1]);   //子进程不需要这个管道(对子进程来说就是左边管道)的写端，关闭
        close(pleft[0]);   //子进程不需要这个管道（对子进程说是左边的左边管道）的读端，关闭
        sieve(pright);
    } else {
        close(pright[0]);    //当前进程不需要这个管道的读端，关闭
        // 从左邻居接受数组
        int buf;
        //当write端关闭时，read返回0
        while(read(pleft[0], &buf, sizeof(buf)) && buf != -1){
            if(buf % p != 0){
                write(pright[1], &buf, sizeof(buf));
            }
        }
        //手动将数组的末尾设置为-1，作为结束符号
        buf = -1;
        write(pright[1], &buf, sizeof(buf));
        wait(0);
        exit(0);
    }
}

int main(int argc, char **argv){
    //创建初始管道, input_pipe[0]:读，input_pipe[1]:写
    int input_pipe[2];
    pipe(input_pipe);

    if(fork() == 0){
        close(input_pipe[1]);   //右邻居用不到这个管道的写端，关闭
        sieve(input_pipe);
    } else {
        close(input_pipe[0]);
        int i;
        for(i = 2; i <= 35; i++){
            write(input_pipe[1], &i, sizeof(i));
        }
        //写入结束标志
        i = -1;
        write(input_pipe[1], &i, sizeof(i));
    }
    //等待子进程结束，但是无法等待子进程的子进程
    //所以需要再子进程中各自执行wait，形成等待链
    wait(0);
    exit(0);
}