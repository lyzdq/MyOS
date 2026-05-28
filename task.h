#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define MAX_TASKS 8
#define STACK_SIZE 1024

/* 任务状态 */
enum { TASK_READY, TASK_RUNNING };

/* 保存寄存器快照 */
struct regs_context {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip;
} __attribute__((packed));

struct task {
    int id;
    int state;
    uint8_t stack[STACK_SIZE];
    struct regs_context *ctx;  /* 指向栈上保存的寄存器 */
    void (*entry)(void);       /* 任务入口函数 */
};

void task_init(void);
void task_create(void (*entry)(void));
void task_switch(void);
struct task *task_current(void);

#endif
