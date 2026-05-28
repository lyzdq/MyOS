/* pic.h - 8259A PIC 重映射 */

#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* 内联汇编：端口IO —— 必须放在最前面 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#define PIC1        0x20    /* 主PIC基地址 */
#define PIC2        0xA0    /* 从PIC基地址 */
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2+1)

/* 初始化PIC，将IRQ 0-15映射到中断号 32-47 */
static inline void pic_init(void) {
    uint8_t a1, a2;

    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, 0x11);   /* ICW1: 初始化 + 边沿触发 */
    outb(PIC1_DATA, 0x20);      /* ICW2: 主PIC映射到中断号32-39 */
    outb(PIC1_DATA, 0x04);      /* ICW3: 主PIC的IRQ2连接从PIC */
    outb(PIC1_DATA, 0x01);      /* ICW4: 8086模式 */

    outb(PIC2_COMMAND, 0x11);   /* ICW1 */
    outb(PIC2_DATA, 0x28);      /* ICW2: 从PIC映射到中断号40-47 */
    outb(PIC2_DATA, 0x02);      /* ICW3 */
    outb(PIC2_DATA, 0x01);      /* ICW4 */

    outb(PIC1_DATA, a1);        /* 恢复掩码 */
    outb(PIC2_DATA, a2);
}

/* 发送EOI */
static inline void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, 0x20);
    outb(PIC1_COMMAND, 0x20);
}

/* 设置中断屏蔽 */
static inline void pic_set_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t value = inb(port) | (1 << irq);
    outb(port, value);
}

static inline void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t value = inb(port) & ~(1 << irq);
    outb(port, value);
}

#endif
