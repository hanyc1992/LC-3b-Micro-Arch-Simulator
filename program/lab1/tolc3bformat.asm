	.ORIG x3000
	LEA R1, LC3A
	LDW R1, R1, #0		;R1 = x4000
	LDW R1, R1, #0		;R1 = read first address
	LEA R2, LC3BA
	LDW R2, R2, #0		;R2 = x4002
	LDW R2, R2, #0		;R2 = write first address

	ADD R1, R1, #-2
	ADD R2, R2, #-1

LOOP	ADD R1, R1, #2
	ADD R2, R2, #1
	LDB R3, R1, #0		;condition code set by LDB
	STB R3, R2, #0
	BRnp LOOP		;reach the end of string
	HALT

LC3A	.FILL x4000
LC3BA	.FILL x4002

	.END
