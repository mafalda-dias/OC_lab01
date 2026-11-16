.data
.align 4
buffer: .zero 2048

.text
.global main
main:
    la      x12, buffer
    addi    x6,  x12, 512
    addi    x15, x12, 256
    addi    x11, x12, 768
    addi    x17, x15, 0
    addi    x16, x6, 0
    addi    x10, x6, 0
    addi    x13, x11, 0
    addi    x14, x12, 0
    addi    x31, x0, 123
    addi    x30, x0, 234
    addi    x29, x0, 345
    addi    x28, x0, 456

loop1:
    sw      x31, 0(x14)
    sw      x30, 0(x17)
    sw      x29, 0(x10)
    sw      x28, 0(x13)
    addi    x14, x14, 4
    addi    x17, x17, 4
    addi    x10, x10, 4
    addi    x13, x13, 12
    bne     x14, x15, loop1

loop2:
    lw      x13, 0(x15)
    lw      x14, 0(x11)
    addi    x15, x15, 4
    addi    x13, x13, 567
    addi    x14, x14, 678
    sw      x13, 0(x12)
    sw      x14, 0(x16)
    addi    x12, x12, 4
    addi    x11, x11, 12
    addi    x16, x16, 4
    bne     x15, x6, loop2

    addi    x10, x0, 0

    li      a7, 10
    ecall
