; boot32.asm - 阶段2：进入保护模式，加载C内核
; 编译: nasm -f bin boot32.asm -o boot32.bin

[BITS 16]
[ORG 0x7c00]

start:
    ; 设置段寄存器
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; 打印实模式消息
    mov si, msg_real
    call print_string

    ; 加载C内核到内存0x1000（从磁盘第2扇区读取N个扇区）
    mov bx, 0x1000   ; 加载目标地址
    mov ah, 0x02     ; BIOS读扇区功能
    mov al, 32       ; 读取32个扇区（16KB），够放内核
    mov ch, 0        ; 柱面0
    mov cl, 2        ; 扇区2（扇区1是引导扇区）
    mov dh, 0        ; 磁头0
    int 0x13
    jc disk_error    ; 如果CF=1，读盘失败

    ; 关闭中断，加载GDT
    cli
    lgdt [gdt_descriptor]

    ; 设置CR0的保护模式位
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 远跳转进入保护模式
    jmp CODE_SEG:protected_start

disk_error:
    mov si, msg_error
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0e
    int 0x10
    jmp print_string
.done:
    ret

[BITS 32]
protected_start:
    ; 设置数据段
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000   ; 栈顶设在0x90000

    ; 调用C内核入口
    call 0x1000

    ; 如果C函数返回，挂起
    jmp $

%include "gdt.asm"

msg_real:  db 'Switching to protected mode...', 13, 10, 0
msg_error: db 'Disk read error!', 13, 10, 0

times 510-($-$$) db 0
dw 0xaa55
