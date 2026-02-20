.include "../share/vc4inc/vc4.qinc"
mov   ra0, unif #A
mov   ra1, unif #B
mov   ra2, unif #C
mov   ra3, unif #iterations = n / 16
mov   ra4, unif #qpu_num (which qpu core this is)

    # 2 64 byte vectors are batched in one iteration for 128 bytes
mov rb0, 128
shl rb1, ra4, 2 #Multiply by 4 to get base row y values used to shift VPM start-row for vpm_setup
shl rb2, ra4, 6 #(4 + 2) add to shift vdr_setup_0 y coordinate by 4 * qpu_num rows
shl rb3, ra4, 7 #(7 + 2) add to shift vdw_setup_0 y coordinate by 4 * qpu_num rows

:loop
        # Load 2 rows from DMA to VPM at y = 0, x = 0
        # mpitch = 3 is the distance in bytes between the start of successive rows in memory 
        # (encoded as a small value since byte pitch is multiple of 8 and 3 gets 8 * 2 ^ 3 = 64 bytes)
        # rowlen = 16 transfers rows of 16 × 32-bit words
        # nrows = 2 transfer 2 rows due to batching
        # vdr_h32's vpitch = 1 selects how many VPM rows separate successive elements (1 means consecutive VPM rows)
    mov r0, vdr_setup_0(3, 16, 2, vdr_h32(1, 0, 0))
        # Since vdr_setup_0 macros can't use registers, we have to add rb2 manually to get correct y position based on QPU num
    add vr_setup, r0, rb2
    mov vr_addr, ra0
    mov -, vr_wait

        # Load 2 rows from DMA to VPM at y = 2, x = 0
    mov r0, vdr_setup_0(3, 16, 2, vdr_h32(1, 2, 0))
    add vr_setup, r0, rb2
    mov vr_addr, ra1
    mov -, vr_wait

        # Mov 4 rows from VPM to registers
        # num = 4 performs 4 reads from VPM into registers
        # stride = 1 increments the VPM y by 1 row after each read
        # dma = h32(0) starts at VPM y = 0 so we can add rb1 to shift it
    mov r0, vpm_setup(4, 1, h32(0))
    add vr_setup, r0, rb1
    mov r0, vpm_setup(2, 1, h32(0))
    add vw_setup, r0, rb1
    mov ra5, vpm
    mov ra6, vpm
    mov rb5, vpm
    mov rb6, vpm
    mov -, vr_wait

    add r0, ra5, rb5
    add r1, ra6, rb6
    mov vpm, r0
    mov vpm, r1
    mov -, vw_wait

    # We only write out 2 rows
    mov r0, vdw_setup_0(2, 16, dma_h32(0, 0))
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