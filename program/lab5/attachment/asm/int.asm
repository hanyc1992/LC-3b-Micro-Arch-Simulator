		.ORIG x1200

		ADD R6, R6, #-2
		STW R0, R6, #0
		ADD R6, R6, #-2
		STW R1, R6, #0
		ADD R6, R6, #-2
		STW R2, R6, #0
	
		LEA R0, LOC
		LDW R0, R0, #0
		LEA R1, NUM
		LDW R1, R1, #0
	
LOOP	LDW R2, R0, #0
		AND R2, R2, #-2
		STW R2, R0, #0
		ADD R0, R0, #2
		ADD R1, R1, #-1
		BRp LOOP
		
		LDW R2, R6, #0
		ADD R6, R6, #2
		LDW R1, R6, #0
		ADD R6, R6, #2
		LDW R0, R6, #0
		ADD R6, R6, #2
	
		RTI

LOC		.FILL x1000
NUM		.FILL #128
		.END