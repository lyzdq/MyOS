; boot.asm - 阶段1：引导扇区，打印 "Hello, MyOS!"
; 编译: nasm -f bin boot.asm -o boot.bin

[BITS 16]           ; 实模式16位
[ORG 0x7c00]        ; BIOS加载到0x7c00

start:
    ; 设置段寄存器
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00   ; 栈从0x7c00向下增长

    ; 打印字符串
    mov si, msg
    call print_string

    ; 挂起（无限循环）
    jmp $

print_string:
    lodsb            ; 加载[si]到al，si++
    or al, al        ; 检查是否到字符串末尾(0)
    jz .done
    mov ah, 0x0e     ; BIOS teletype函数
    int 0x10
    jmp print_string
.done:
    ret

msg: db 'Hello, MyOS!', 13, 10, 0

; 填充到510字节，最后2字节是启动签名
times 510-($-$$) db 0
dw 0xaa55
