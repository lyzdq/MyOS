[GLOBAL irq0_handler]
[GLOBAL irq1_handler]
[EXTERN timer_handler]
[EXTERN keyboard_handler]

irq0_handler:
    pusha
    call timer_handler
    popa
    iret

irq1_handler:
    pusha
    call keyboard_handler
    popa
    iret
