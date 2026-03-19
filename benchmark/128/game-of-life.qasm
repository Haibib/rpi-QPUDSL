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
mov ra12, unif
mov ra13, unif
mov ra14, unif
mov ra15, unif
mov ra16, unif
mov ra17, unif
mov ra18, unif
mov ra19, unif
mov ra20, unif
mov ra21, unif
mov ra22, unif
mov ra23, unif
mov ra24, unif
mov ra25, unif
mov ra26, unif
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
add ra10, ra10, r0
add ra12, ra12, r0
add ra14, ra14, r0
add ra16, ra16, r0
add ra18, ra18, r0
add ra20, ra20, r0
add ra22, ra22, r0
add ra24, ra24, r0
add ra26, ra26, r0
mov ra27, 4
mov r0, 4
mul24 r0, r0, rb0
mov ra28, r0
mov r0, rb2
shr r0, r0, 7
mov ra29, r0
mov r0, rb3
shr r2, r0, 3
shl r3, r2, 3
sub r3, r0, r3
sub.setf r0, r1, r3
mov r0, r2
add.ifn r0, r0, 1
mov ra30, r0
mov rb0, 512
shl rb1, r1, 3
shl rb2, r1, 7
shl rb3, r1, 10
mov rb6, ra0
mov r0, ra9
mov r1, ra27
mul24 r0, r0, r1
add rb6, rb6, r0
mov r0, ra10
mov r1, ra28
mul24 r0, r0, r1
add rb6, rb6, r0
mov rb7, ra1
mov r0, ra11
mov r1, ra27
mul24 r0, r0, r1
add rb7, rb7, r0
mov r0, ra12
mov r1, ra28
mul24 r0, r0, r1
add rb7, rb7, r0
mov rb8, ra2
mov r0, ra13
mov r1, ra27
mul24 r0, r0, r1
add rb8, rb8, r0
mov r0, ra14
mov r1, ra28
mul24 r0, r0, r1
add rb8, rb8, r0
mov rb9, ra3
mov r0, ra15
mov r1, ra27
mul24 r0, r0, r1
add rb9, rb9, r0
mov r0, ra16
mov r1, ra28
mul24 r0, r0, r1
add rb9, rb9, r0
mov rb10, ra4
mov r0, ra17
mov r1, ra27
mul24 r0, r0, r1
add rb10, rb10, r0
mov r0, ra18
mov r1, ra28
mul24 r0, r0, r1
add rb10, rb10, r0
mov rb11, ra5
mov r0, ra19
mov r1, ra27
mul24 r0, r0, r1
add rb11, rb11, r0
mov r0, ra20
mov r1, ra28
mul24 r0, r0, r1
add rb11, rb11, r0
mov rb12, ra6
mov r0, ra21
mov r1, ra27
mul24 r0, r0, r1
add rb12, rb12, r0
mov r0, ra22
mov r1, ra28
mul24 r0, r0, r1
add rb12, rb12, r0
mov rb13, ra7
mov r0, ra23
mov r1, ra27
mul24 r0, r0, r1
add rb13, rb13, r0
mov r0, ra24
mov r1, ra28
mul24 r0, r0, r1
add rb13, rb13, r0
mov r0, ra25
mov r1, ra27
mul24 r0, r0, r1
add ra8, ra8, r0
mov r0, ra26
mov r1, ra28
mul24 r0, r0, r1
add ra8, ra8, r0
mov rb5, ra30


:loop_1
mov rb4, ra29


:loop_0
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb6
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov rb18, vpm
mov rb19, vpm
mov rb20, vpm
mov rb21, vpm
mov rb22, vpm
mov rb23, vpm
mov rb24, vpm
mov rb25, vpm
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb7
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb18, rb18, r0
mov r0, vpm
add rb19, rb19, r0
mov r0, vpm
add rb20, rb20, r0
mov r0, vpm
add rb21, rb21, r0
mov r0, vpm
add rb22, rb22, r0
mov r0, vpm
add rb23, rb23, r0
mov r0, vpm
add rb24, rb24, r0
mov r0, vpm
add rb25, rb25, r0
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb8
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb18, rb18, r0
mov r0, vpm
add rb19, rb19, r0
mov r0, vpm
add rb20, rb20, r0
mov r0, vpm
add rb21, rb21, r0
mov r0, vpm
add rb22, rb22, r0
mov r0, vpm
add rb23, rb23, r0
mov r0, vpm
add rb24, rb24, r0
mov r0, vpm
add rb25, rb25, r0
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb9
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb18, rb18, r0
mov r0, vpm
add rb19, rb19, r0
mov r0, vpm
add rb20, rb20, r0
mov r0, vpm
add rb21, rb21, r0
mov r0, vpm
add rb22, rb22, r0
mov r0, vpm
add rb23, rb23, r0
mov r0, vpm
add rb24, rb24, r0
mov r0, vpm
add rb25, rb25, r0
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb10
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb18, rb18, r0
mov r0, vpm
add rb19, rb19, r0
mov r0, vpm
add rb20, rb20, r0
mov r0, vpm
add rb21, rb21, r0
mov r0, vpm
add rb22, rb22, r0
mov r0, vpm
add rb23, rb23, r0
mov r0, vpm
add rb24, rb24, r0
mov r0, vpm
add rb25, rb25, r0
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb11
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb18, rb18, r0
mov r0, vpm
add rb19, rb19, r0
mov r0, vpm
add rb20, rb20, r0
mov r0, vpm
add rb21, rb21, r0
mov r0, vpm
add rb22, rb22, r0
mov r0, vpm
add rb23, rb23, r0
mov r0, vpm
add rb24, rb24, r0
mov r0, vpm
add rb25, rb25, r0
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb12
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb18, rb18, r0
mov r0, vpm
add rb19, rb19, r0
mov r0, vpm
add rb20, rb20, r0
mov r0, vpm
add rb21, rb21, r0
mov r0, vpm
add rb22, rb22, r0
mov r0, vpm
add rb23, rb23, r0
mov r0, vpm
add rb24, rb24, r0
mov r0, vpm
add rb25, rb25, r0
mov -, vr_wait
mov r0, vdr_setup_0(3, 16, 8, vdr_h32(1, 0, 0))
add vr_setup, r0, rb2
mov vr_addr, rb13
mov -, vr_wait
mov r0, vpm_setup(8, 1, h32(0))
add vr_setup, r0, rb1
mov r0, vpm
add rb18, rb18, r0
mov r0, vpm
add rb19, rb19, r0
mov r0, vpm
add rb20, rb20, r0
mov r0, vpm
add rb21, rb21, r0
mov r0, vpm
add rb22, rb22, r0
mov r0, vpm
add rb23, rb23, r0
mov r0, vpm
add rb24, rb24, r0
mov r0, vpm
add rb25, rb25, r0
mov -, vr_wait
mov r2, vpm_setup(8, 1, h32(0))
add vw_setup, r2, rb1
mov vpm, rb18
mov vpm, rb19
mov vpm, rb20
mov vpm, rb21
mov vpm, rb22
mov vpm, rb23
mov vpm, rb24
mov vpm, rb25
mov -, vw_wait
mov r2, vdw_setup_0(8, 16, dma_h32(0, 0))
add vw_setup, r2, rb3
mov vw_addr, ra8
mov -, vw_wait
mov r0, rb6
add rb6, r0, rb0
mov r0, rb7
add rb7, r0, rb0
mov r0, rb8
add rb8, r0, rb0
mov r0, rb9
add rb9, r0, rb0
mov r0, rb10
add rb10, r0, rb0
mov r0, rb11
add rb11, r0, rb0
mov r0, rb12
add rb12, r0, rb0
mov r0, rb13
add rb13, r0, rb0
add ra8, ra8, rb0
mov r0, rb4
sub.setf rb4, r0, 1
brr.anynz -, :loop_0
nop
nop
nop
mov r0, ra28
mov r1, ra29
mul24 r1, r1, rb0
sub r0, r0, r1
add rb6, rb6, r0
add rb7, rb7, r0
add rb8, rb8, r0
add rb9, rb9, r0
add rb10, rb10, r0
add rb11, rb11, r0
add rb12, rb12, r0
add rb13, rb13, r0
add ra8, ra8, r0
mov r0, rb5
sub.setf rb5, r0, 1
brr.anynz -, :loop_1
nop
nop
nop
thrend
nop
nop

