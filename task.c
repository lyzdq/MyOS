#include "task.h"

static struct task tasks[MAX_TASKS];
static int task_count = 0;
static int current_task = 0;

void task_init(void) {
    task_count = 0;
    current_task = 0;
}

void task_create(void (*entry)(void)) {
    if (task_count >= MAX_TASKS) return;
    struct task *t = &tasks[task_count];
    t->id = task_count;
    t->state = TASK_READY;
    t->entry = entry;

    /* 初始化栈：模拟中断压栈后的布局 */
    uint32_t *stack = (uint32_t *)(t->stack + STACK_SIZE);
    *--stack = (uint32_t)entry;   /* eip */
    *--stack = 0;                 /* eax */
    *--stack = 0;                 /* ecx */
    *--stack = 0;                 /* edx */
    *--stack = 0;                 /* ebx */
    *--stack = (uint32_t)(t->stack + STACK_SIZE); /* esp */
    *--stack = 0;                 /* ebp */
    *--stack = 0;                 /* esi */
    *--stack = 0;                 /* edi */
    t->ctx = (struct regs_context *)stack;

    task_count++;
}

struct task *task_current(void) {
    return &tasks[current_task];
}

/* 简易调度：轮转 */
__attribute__((noinline))
void task_switch(void) {
    if (task_count < 2) return;
    current_task = (current_task + 1) % task_count;
}
