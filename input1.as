.extern W
.entry LENGTH
MAIN:   mov @r3,LENGTH
LOOP:	jmp L1
	prn -5
	bne W
	sub @r1, @r4
	bne L3
L1:	inc K
.entry LOOP
	jmp W
END:	stop
STR: 	.string "aaa a"
LENGTH: .data 6, -9,15
Kk:	.data 22
.extern L3
.entry way