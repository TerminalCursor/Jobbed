# Jobbed

## Supplemental Links
 - https://www.raspberrypi.com/documentation/computers/processors.html
 - https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf
 - https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf
 - https://github.com/eggman/raspberrypi/tree/master/qemu-raspi2
 - https://github.com/eggman/FreeRTOS-raspi3/blob/master/Demo/FreeRTOS_tick_config.c
 - https://developer.arm.com/documentation/dui0802/b/A64-General-Instructions/A64-general-instructions-in-alphabetical-order?lang=en
 - https://wiki.osdev.org/ARM_RaspberryPi
 - https://elinux.org/RPi_Framebuffer
 - https://github.com/bztsrc/raspi3-tutorial
 - https://github.com/rust-embedded/rust-raspberrypi-OS-tutorials

 - // PRIV LEVL: https://developer.arm.com/documentation/den0013/d/ARM-Processor-Modes-and-Registers
 - // MODES: https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/The-System-Level-Programmers--Model/ARM-processor-modes-and-ARM-core-registers/ARM-processor-modes?lang=en
 - // EXCEPTION HANDLING: https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/The-System-Level-Programmers--Model/Exception-handling?lang=en
 - // MODE REGISTERS: https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/The-System-Level-Programmers--Model/ARM-processor-modes-and-ARM-core-registers/ARM-core-registers?lang=en

## Todo
 - Write Thesis Paper
 - Semaphore Implementation
 - Return if the thread can be added
 - Mutex lock sequentially
 - Modularize
 - Add in remaining checks (scheduling)

## SVC Calls
 - Sysinfo
 - Reboot?
 - Mutex Create
 - Mutex Lock
 - Mutex Release
 - Schedule
 - Yield
 - Fork
 - Uart Write Buffer Add (Add reoccuring task to flush write buffer - RotatingBuffer)


Note: SYS and USER share registers but differ in privilege


# Modes
 - From https://developer.arm.com/documentation/den0013/d/ARM-Processor-Modes-and-Registers
 - User: 10:10000 PL0
 - FIQ : 11:10001 PL1
 - IRQ : 12:10010 PL1
 - SVC : 13:10011 PL1
 - ABT : 17:10111 PL1
 - HVC : 1A:11010 PL1
 - UND : 1B:11011 PL1
 - SYS : 1F:11111 PL1

# Registers
 - From https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/The-System-Level-Programmers--Model/ARM-processor-modes-and-ARM-core-registers/ARM-core-registers?lang=en
 - User: r0-12_usr,sp_usr,lr_usr,CPSR
 - FIQ: r8-r12_fiq,sp_fiq,lr_fiq,SPSR_fiq
 - IRQ: sp_irq,lr_irq,SPSR_irq
 - SVC: sp_svc,lr_svc,SPSR_svc
 - ABT: sp_abt,lr_abt,SPSR_abt
 - UND: sp_und,lr_und,SPSR_und
 - SYS: Same as user

# Program Status Registers
 - https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/The-System-Level-Programmers--Model/ARM-processor-modes-and-ARM-core-registers/Program-Status-Registers--PSRs-?lang=en#CIHJBHJA
