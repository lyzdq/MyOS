#define VIDEO_MEMORY 0xb8000

volatile char *video = (volatile char *)VIDEO_MEMORY;
int cursor_pos = 0;
int task_ticks = 0;
int current_task = 0;
char cmd_buf[64];
int cmd_len = 0;

void putchar(char c) {
    if (c == '\n') {
        cursor_pos = (cursor_pos / 80 + 1) * 80;
    } else {
        video[cursor_pos * 2] = c;
        video[cursor_pos * 2 + 1] = 0x0f;
        cursor_pos++;
    }
    if (cursor_pos >= 80 * 25) {
        for (int i = 0; i < 24 * 80; i++) {
            video[i * 2] = video[(i + 80) * 2];
            video[i * 2 + 1] = video[(i + 80) * 2 + 1];
        }
        for (int i = 24 * 80; i < 25 * 80; i++) {
            video[i * 2] = ' ';
            video[i * 2 + 1] = 0x0f;
        }
        cursor_pos = 24 * 80;
    }
}

void print(const char *s) {
    while (*s) putchar(*s++);
}

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

struct idt_entry {
    unsigned short base_low, sel;
    unsigned char zero, flags;
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;
extern void irq0_handler();
extern void irq1_handler();

void idt_set_gate(int n, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[n].base_low = base & 0xffff;
    idt[n].base_high = (base >> 16) & 0xffff;
    idt[n].sel = sel;
    idt[n].zero = 0;
    idt[n].flags = flags;
}

void timer_handler(void) {
    outb(0x20, 0x20);
    task_ticks++;
}

void draw_row(int row, char c, char color) {
    for (int i = 0; i < 80; i++) {
        video[(row * 80 + i) * 2] = c;
        video[(row * 80 + i) * 2 + 1] = color;
    }
}

void clear_row(int row) {
    for (int i = 0; i < 80; i++)
        video[(row * 80 + i) * 2] = ' ';
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a - *b;
}

void run_command(void) {
    cmd_buf[cmd_len] = '\0';
    putchar('\n');

    if (strcmp(cmd_buf, "help") == 0) {
        print("Commands: help clear task ver tick\n");
    } else if (strcmp(cmd_buf, "clear") == 0) {
        /* 清空整个屏幕 */
        for (int i = 0; i < 80 * 25; i++) {
            video[i * 2] = ' ';
            video[i * 2 + 1] = 0x0f;
        }
        cursor_pos = 0;
    } else if (strcmp(cmd_buf, "task") == 0) {
        if (current_task == 0) print("Running: Task A\n");
        else print("Running: Task B\n");
    } else if (strcmp(cmd_buf, "ver") == 0) {
        print("MyOS v0.5 - Mini Kernel\n");
    } else if (strcmp(cmd_buf, "tick") == 0) {
        print("Ticks: ");
        if (task_ticks >= 100) putchar('0' + task_ticks / 100);
        if (task_ticks >= 10) putchar('0' + (task_ticks / 10) % 10);
        putchar('0' + task_ticks % 10);
        putchar('\n');
    } else if (cmd_len > 0) {
        print("Unknown: ");
        print(cmd_buf);
        putchar('\n');
    }

    print("> ");
    cmd_len = 0;
}

void keyboard_handler(void) {
    unsigned char code = inb(0x60);
    static char map[128] = {
        0, 0, '1','2','3','4','5','6','7','8','9','0','-','=',0,
        0, 'q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
        0, '\\','z','x','c','v','b','n','m',',','.','/',0,
        0, 0, 0, ' ', 0
    };

    if (code & 0x80) { outb(0x20, 0x20); return; }

    char c = (code < 128) ? map[code] : 0;

    if (c == '\n') {
        run_command();
    } else if (c == '\b' || code == 14) {
        if (cmd_len > 0) {
            cmd_len--;
            putchar('\b');
        }
    } else if (c && cmd_len < 63) {
        cmd_buf[cmd_len++] = c;
        putchar(c);
    }

    outb(0x20, 0x20);
}

__attribute__((section(".text.startup")))
void kernel_main(void) {
    int i;
    for (i = 0; i < 80 * 25; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x0f;
    }

    print("MyOS v0.5 - Shell\n================\n");
    print("Type 'help' for commands\n");
    print("TaskA=green  TaskB=red\n> ");

    idtp.limit = 256 * 8 - 1;
    idtp.base = (unsigned int)&idt;
    for (i = 0; i < 256; i++)
        idt_set_gate(i, (unsigned long)irq0_handler, 0x08, 0x8e);
    idt_set_gate(33, (unsigned long)irq1_handler, 0x08, 0x8e);
    __asm__ volatile ("lidt (%0)" : : "r"(&idtp));

    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0xFC);

    outb(0x43, 0x36);
    outb(0x40, 0xA9);
    outb(0x40, 0x04);

    __asm__ volatile ("sti");

    int last_switch = 0;

    while (1) {
        __asm__ volatile ("hlt");

        /* 调度条固定在第20-21行，不干扰命令行区域 */
        if (task_ticks - last_switch >= 5) {
            last_switch = task_ticks;
            current_task = 1 - current_task;
            if (current_task == 0) {
                draw_row(20, 'A', 0x0a);
                clear_row(21);
            } else {
                draw_row(21, 'B', 0x0c);
                clear_row(20);
            }
        }

        /* 右下角tick */
        video[(24 * 80 + 70) * 2] = '0' + ((task_ticks / 10) % 10);
        video[(24 * 80 + 71) * 2] = '0' + (task_ticks % 10);
        video[(24 * 80 + 70) * 2 + 1] = 0x07;
        video[(24 * 80 + 71) * 2 + 1] = 0x07;
    }
}
