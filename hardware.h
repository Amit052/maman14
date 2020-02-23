#pragma once
typedef struct word {
	char bits[16];
}word;

typedef struct reg {
	word value;
}reg;

typedef struct cpu{
	reg r1, r2,r3, r4, r5, r6,r7, r0;
}cpu;

typedef struct ram{
	word memory[4096];
}ram;

