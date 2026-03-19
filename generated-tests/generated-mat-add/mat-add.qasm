.include "../share/vc4inc/vc4.qinc"
mov ra0, unif
mov ra1, unif
mov ra2, unif
mov ra3, unif
mov ra4, unif
mov ra5, unif
mov ra6, unif
mov ra7, unif
mov ra8, unif
mov rb0, unif
mov rb1, unif
mov rb2, unif
mov rb3, unif
mov r1, unif
mov r0, rb3
shr r2, r0, 3
shl r3, r2, 3
sub r3, r0, r3
mul24 r0, r1, r2
sub.setf r2, r1, r3
mov.ifn  r2, r1
mov.ifnn r2, r3
add r0, r0, r2
add ra4, ra4, r0
add ra6, ra6, r0
add ra8, ra8, r0
mov ra9, 4
mov r0, 4
mul24 r0, r0, rb0
mov ra10, r0
mov r0, rb2
shr r0, r0, 6
mov ra11, r0
mov r0, rb3
shr r2, r0, 3
shl r3, r2, 3
sub r3, r0, r3
sub.setf r0, r1, r3
mov r0, r2
add.ifn r0, r0, 1
mov ra12, r0
mov rb0, 256
shl rb1, r1, 2
shl rb2, r1, 6
shl rb3, r1, 9
mov rb6, ra0
mov r0, ra3
mov r1, ra9
mul24 r0, r0, r1
add rb6, rb6, r0
mov r0, ra4
mov r1, ra10
mul24 r0, r0, r1
add rb6, rb6, r0
mov rb7, ra1
mov r0, ra5
mov r1, ra9
mul24 r0, r0, r1
add rb7, rb7, r0
mov r0, ra6
mov r1, ra10
mul24 r0, r0, r1
add rb7, rb7, r0
mov r0, ra7
mov r1, ra9
mul24 r0, r0, r1
add ra2, ra2, r0
mov r0, ra8
mov r1, ra10
mul24 r0, r0, r1
add ra2, ra2, r0
mov rb5, ra12


:loop_1
mov rb4, ra11


:loop_0
mov r0, vdr_setup_0(3, 16, 4, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb6
mov -, vr_wait
mov r0, vpm_setup(4, 1, h32(0))
add vr_setup, r0, rb1
mov rb8, vpm
mov rb9, vpm
mov rb10, vpm
mov rb11, vpm
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 4, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb7
mov -, vr_wait
mov r0, vpm_setup(4, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb8, rb8, r0
mov r0, vpm
add rb9, rb9, r0
mov r0, vpm
add rb10, rb10, r0
mov r0, vpm
add rb11, rb11, r0
mov -, vr_wait
mov r2, vpm_setup(4, 1, h32(0))
add vw_setup, r2, rb1
mov vpm, rb8
mov vpm, rb9
mov vpm, rb10
mov vpm, rb11
mov -, vw_wait
mov r2, vdw_setup_0(4, 16, dma_h32(0, 0))
add vw_setup, r2, rb3
mov vw_addr, ra2
mov -, vw_wait
mov r0, rb6
add rb6, r0, rb0
mov r0, rb7
add rb7, r0, rb0
add ra2, ra2, rb0
mov r0, rb4
sub.setf rb4, r0, 1
brr.anynz -, :loop_0
nop
nop
nop
mov r0, ra10
mov r1, ra11
mul24 r1, r1, rb0
sub r0, r0, r1
add rb6, rb6, r0
add rb7, rb7, r0
add ra2, ra2, r0
mov r0, rb5
sub.setf rb5, r0, 1
brr.anynz -, :loop_1
nop
nop
nop
thrend
nop
nop

