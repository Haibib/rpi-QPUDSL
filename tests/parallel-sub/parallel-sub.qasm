.include "../share/vc4inc/vc4.qinc"
mov   ra0, unif #A
mov   ra1, unif #B
mov   ra2, unif #C
mov   ra3, unif #iterations = n / 16
mov   ra4, unif #qpu_num (which qpu core this is)

mov rb0, 128
shl rb1, ra4, 2 #Multiply by 4 to get base row
shl rb2, ra4, 6 #vdr_setup_0 shift (4 + 2)
shl rb3, ra4, 9 #vdw_setup_0 shift (7 + 2)

:loop
    # Load 2 rows from DMA to VPM at y = 0
    mov r0, vdr_setup_0(3, 16, 2, vdr_h32(1, 0, 0))
    add vr_setup, r0, rb2
    mov vr_addr, ra0
    mov -, vr_wait

    # Load 2 rows from DMA to VPM at y = 2
    mov r0, vdr_setup_0(3, 16, 2, vdr_h32(1, 2, 0))
    add vr_setup, r0, rb2
    mov vr_addr, ra1
    mov -, vr_wait

    # Mov 4 rows from VPM to registers
    mov r0, vpm_setup(4, 1, h32(0))
    add vr_setup, r0, rb1
    mov r0, vpm_setup(2, 1, h32(0))
    add vw_setup, r0, rb1
    mov ra5, vpm
    mov ra6, vpm
    mov rb5, vpm
    mov rb6, vpm
    mov -, vr_wait

    sub r0, ra5, rb5
    sub r1, ra6, rb6
    mov vpm, r0
    mov vpm, r1
    mov -, vw_wait

    mov r0, vdw_setup_0(2, 16, dma_h32(0,0))
    add vw_setup, r0, rb3
    mov vw_addr, ra2
    mov -, vw_wait

    sub.setf ra3, ra3, 2
    # After a branch, the 3 subsequent instructions are executed
    brr.anynz -, :loop
    add ra0, ra0, rb0
    add ra1, ra1, rb0
    add ra2, ra2, rb0

:end
thrend
mov interrupt, 1
nop
nop