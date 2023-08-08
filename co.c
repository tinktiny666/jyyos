#include "co.h"
#include <setjmp.h>
#include <stdint-gcc.h>
#include <stdlib.h>

#define STACK_SIZE 32768
/*
void entry(void *arg) {
  while (n--) {
    printf("%s", (const char *)arg);
    co_yield();
  }
}

int main() {
  struct co *co1 = co_start("co1", entry, "a");
  struct co *co2 = co_start("co2", entry, "b");
  co_wait(co1); // never returns
  co_wait(co2);
}
*/


static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
    asm volatile(
#if __x86_64__
        "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
        :
        : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
        : "memory"
#else
        "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
        :
        : "b"((uintptr_t)sp + 8), "d"(entry), "a"(arg)
        : "memory"
#endif
    );
}


enum co_status {
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
    char *name;
    void (*func)(void *); // co_start 指定的入口地址和参数
    void *arg;

    enum co_status status;     // 协程的状态
    struct co *waiter;         // 是否有其他协程在等待当前协程
    jmp_buf context;           // 寄存器现场 (setjmp.h)
    uint8_t stack[STACK_SIZE]; // 协程的堆栈 uint8_t为一个字节
};

struct co *current; //当前运行线程

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
    struct co* co_new=(struct co*)malloc(sizeof(struct co));
    memset(co_new->stack,0,STACK_SIZE);
    co_new->arg=arg;
    co_new->func=func;
    co_new->name=name;
    co_new->status=CO_NEW;
    co_new->waiter=NULL;
    return co_new;
}

void co_wait(struct co *co)
{
    if(co->status == CO_NEW)
        {
          co->status=CO_RUNNING;
          current=co;
          stack_switch_call(co->stack,co->func,co->arg);
        }
    while (1) {
        if (co->status == CO_DEAD) {
            break;
        }
    }
    return;
}

void co_yield ()
{
    int val = setjmp(current->context);
    if (val == 0) {
        // ?
        if(!current->waiter){
          longjmp(current->context,0);
        }
        current->status=CO_WAITING;
        current=current->waiter;
        longjmp(current->context,0);
    } else
    {
      current->status=CO_RUNNING;
    }
}
