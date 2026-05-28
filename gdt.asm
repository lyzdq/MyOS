; gdt.asm - 全局描述符表（GDT），保护模式必需

gdt_start:

gdt_null:       ; 第一个描述符必须为空
    dd 0
    dd 0

gdt_code:       ; 代码段描述符
    dw 0xffff       ; 段界限 0-15
    dw 0            ; 基地址 0-15
    db 0            ; 基地址 16-23
    db 10011010b    ; 访问字节：存在、特权级0、可执行、可读
    db 11001111b    ; 标志+段界限：32位、4K粒度、段界限16-19
    db 0            ; 基地址 24-31

gdt_data:       ; 数据段描述符
    dw 0xffff
    dw 0
    db 0
    db 10010010b    ; 访问字节：存在、特权级0、可写
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT大小
    dd gdt_start                ; GDT起始地址

; 段选择子常量
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
