.include "../share/vc4inc/vc4.qinc"
mov ra0, unif
mov ra1, unif
mov ra2, unif
mov ra3, unif
mov ra4, unif
mov ra5, unif
mov rb0, unif
mov rb1, unif
mov r1, unif
mov r0, rb1
shr r0, r0, 4
shr r2, r0, 3
shl r3, r2, 3
sub r3, r0, r3
mul24 r0, r1, r2
sub.setf r2, r1, r3
mov.ifn  r2, r1
mov.ifnn r2, r3
add r0, r0, r2
shl r0, r0, 4
add ra3, ra3, r0
add ra4, ra4, r0
add ra5, ra5, r0
mov ra6, 4
mov r0, rb1
shr r0, r0, 4
shr r2, r0, 3
shl r3, r2, 3
sub r3, r0, r3
sub.setf r0, r1, r3
mov r0, r2
add.ifn r0, r0, 1
mov ra7, r0
mov rb0, 64
shl rb1, r1, 2
shl rb2, r1, 6
shl rb3, r1, 9
mov rb5, ra0
mov r0, ra3
mov r1, ra6
mul24 r0, r0, r1
add rb5, rb5, r0
mov rb6, ra1
mov r0, ra4
mov r1, ra6
mul24 r0, r0, r1
add rb6, rb6, r0
mov r0, ra5
mov r1, ra6
mul24 r0, r0, r1
add ra2, ra2, r0
mov rb4, ra7


:loop_0
mov r0, vdr_setup_0(3, 16, 1, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb5
mov -, vr_wait
mov r0, vpm_setup(1, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
mov -, vr_wait
mov r2, vdr_setup_0(3, 16, 1, vdr_h32(1, 0, 0))
add vr_setup, r2, rb2
mov vr_addr, rb6
mov -, vr_wait
mov r2, vpm_setup(1, 1, h32(0))
add vr_setup, r2, rb1
mov r2, vpm
mov -, vr_wait
add r0, r0, r2
mov r2, vpm_setup(1, 1, h32(0))
add vw_setup, r2, rb1
mov vpm, r0
mov r2, vdw_setup_0(1, 16, dma_h32(0, 0))
add vw_setup, r2, rb3
mov vw_addr, ra2
mov -, vw_wait
mov r0, rb5
add rb5, r0, rb0
mov r0, rb6
add rb6, r0, rb0
add ra2, ra2, rb0
mov r0, rb4
sub.setf rb4, r0, 1
brr.anynz -, :loop_0
nop
nop
nop
thrend
nop
nop

