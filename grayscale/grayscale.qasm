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
mov ra9, unif
mov ra10, unif
mov ra11, unif
mov rb0, unif
mov rb1, unif
mov rb2, unif
mov rb3, unif
mov ra16, unif
mov ra17, unif
mov ra18, unif
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
add ra5, ra5, r0
add ra7, ra7, r0
add ra9, ra9, r0
add ra11, ra11, r0
mov ra12, 4
mov r0, 4
mul24 r0, r0, rb0
mov ra13, r0
mov r0, rb2
shr r0, r0, 7
mov ra14, r0
mov r0, rb3
shr r2, r0, 3
shl r3, r2, 3
sub r3, r0, r3
sub.setf r0, r1, r3
mov r0, r2
add.ifn r0, r0, 1
mov ra15, r0
mov rb0, 512
shl rb1, r1, 3
shl rb2, r1, 7
shl rb3, r1, 10
mov rb6, ra0
mov r0, ra4
mov r1, ra12
mul24 r0, r0, r1
add rb6, rb6, r0
mov r0, ra5
mov r1, ra13
mul24 r0, r0, r1
add rb6, rb6, r0
mov rb7, ra1
mov r0, ra6
mov r1, ra12
mul24 r0, r0, r1
add rb7, rb7, r0
mov r0, ra7
mov r1, ra13
mul24 r0, r0, r1
add rb7, rb7, r0
mov rb8, ra2
mov r0, ra8
mov r1, ra12
mul24 r0, r0, r1
add rb8, rb8, r0
mov r0, ra9
mov r1, ra13
mul24 r0, r0, r1
add rb8, rb8, r0
mov r0, ra10
mov r1, ra12
mul24 r0, r0, r1
add ra3, ra3, r0
mov r0, ra11
mov r1, ra13
mul24 r0, r0, r1
add ra3, ra3, r0
mov rb5, ra15


:loop_1
mov rb4, ra14


:loop_0
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb6
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
fmul rb13, r0, ra16
mov r0, vpm
fmul rb14, r0, ra16
mov r0, vpm
fmul rb15, r0, ra16
mov r0, vpm
fmul rb16, r0, ra16
mov r0, vpm
fmul rb17, r0, ra16
mov r0, vpm
fmul rb18, r0, ra16
mov r0, vpm
fmul rb19, r0, ra16
mov r0, vpm
fmul rb20, r0, ra16
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb7
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r3, vpm
fmul r3, r3, ra17
fadd rb13, rb13, r3
mov r3, vpm
fmul r3, r3, ra17
fadd rb14, rb14, r3
mov r3, vpm
fmul r3, r3, ra17
fadd rb15, rb15, r3
mov r3, vpm
fmul r3, r3, ra17
fadd rb16, rb16, r3
mov r3, vpm
fmul r3, r3, ra17
fadd rb17, rb17, r3
mov r3, vpm
fmul r3, r3, ra17
fadd rb18, rb18, r3
mov r3, vpm
fmul r3, r3, ra17
fadd rb19, rb19, r3
mov r3, vpm
fmul r3, r3, ra17
fadd rb20, rb20, r3
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb8
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r3, vpm
fmul r3, r3, ra18
fadd rb13, rb13, r3
mov r3, vpm
fmul r3, r3, ra18
fadd rb14, rb14, r3
mov r3, vpm
fmul r3, r3, ra18
fadd rb15, rb15, r3
mov r3, vpm
fmul r3, r3, ra18
fadd rb16, rb16, r3
mov r3, vpm
fmul r3, r3, ra18
fadd rb17, rb17, r3
mov r3, vpm
fmul r3, r3, ra18
fadd rb18, rb18, r3
mov r3, vpm
fmul r3, r3, ra18
fadd rb19, rb19, r3
mov r3, vpm
fmul r3, r3, ra18
fadd rb20, rb20, r3
mov -, vr_wait
mov r2, vpm_setup(8, 1, h32(0))
add vw_setup, r2, rb1
mov vpm, rb13
mov vpm, rb14
mov vpm, rb15
mov vpm, rb16
mov vpm, rb17
mov vpm, rb18
mov vpm, rb19
mov vpm, rb20
mov -, vw_wait
mov r2, vdw_setup_0(8, 16, dma_h32(0, 0))
add vw_setup, r2, rb3
mov vw_addr, ra3
mov -, vw_wait
mov r0, rb6
add rb6, r0, rb0
mov r0, rb7
add rb7, r0, rb0
mov r0, rb8
add rb8, r0, rb0
add ra3, ra3, rb0
mov r0, rb4
sub.setf rb4, r0, 1
brr.anynz -, :loop_0
nop
nop
nop
mov r0, ra13
mov r1, ra14
mul24 r1, r1, rb0
sub r0, r0, r1
add rb6, rb6, r0
add rb7, rb7, r0
add rb8, rb8, r0
add ra3, ra3, r0
mov r0, rb5
sub.setf rb5, r0, 1
brr.anynz -, :loop_1
nop
nop
nop
thrend
nop
nop

