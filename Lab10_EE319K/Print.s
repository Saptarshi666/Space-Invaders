; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; SSD1306_OutChar   outputs a single 8-bit ASCII character
; SSD1306_OutString outputs a null-terminated string 

    IMPORT   SSD1306_OutChar
    IMPORT   SSD1306_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
    PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

N EQU 4
CNT EQU 0
FP RN 11

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec

 PUSH{ R11,R4,R5}
		PUSH{R0} ; store number N
		SUB SP,#4
		MOV FP, SP
		PUSH{LR}
		MOV R1,#0 ; R1 IS MY CNT REGISTER
		STR R1, [FP,#CNT]
		MOV R2, #10
STEP1	LDR R1, [FP,#CNT]
		ADD R1,#1
		STR R1, [FP,#CNT]
		MOV R3,R0
		MOV R4, R0
		UDIV R3, R2
		MOV R0,R3
		MUL R3,R2
		SUB R5,R4,R3
		PUSH{R5}
		CMP R0, #0
		BNE STEP1
STEP2	POP{R5}
		ADD R0,R5,#0x30
		BL SSD1306_OutChar
		LDR R1, [FP,#CNT]
		SUB R1, #1
		STR R1, [FP, #CNT]
		CMP R1, #0
		BHI STEP2
		POP{LR}
		ADD SP, #8
		POP {R11,R4,R5}


      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix

  PUSH{ R11,R0,LR,R4,R5}
		 ; store number N
		SUB SP,#4
		MOV FP, SP
		MOV R1,#0 ; R1 IS MY CNT REGISTER
		STR R1, [FP,#CNT]
		MOV R2, #10	
		CMP R0, #1000
		BLO InRange
		MOV R0, #0x2A ; output *.** for outof range numbers
		BL SSD1306_OutChar
		MOV R0, #0x2E
		BL SSD1306_OutChar
		MOV R0, #0x2A
		BL SSD1306_OutChar
		MOV R0, #0x2A
		BL SSD1306_OutChar
		B OutFixExit

InRange
		LDR R1, [FP,#CNT]
		ADD R1,#1
		STR R1, [FP,#CNT]
		MOV R3,R0
		MOV R4, R0
		UDIV R3, R2
		MOV R0,R3
		MUL R3,R2
		SUB R5,R4,R3
		PUSH{R5}

		LDR R1, [FP,#CNT]
		CMP R1, #3
		BLT InRange
		
		POP{R5}
		ADD R0,R5,#0x30
		BL SSD1306_OutChar		
		MOV R0, #0x2E; 2 in outfix
		BL SSD1306_OutChar
		POP{R5}
		ADD R0,R5,#0x30
		BL SSD1306_OutChar
		POP{R5}
		ADD R0,R5,#0x30
		BL SSD1306_OutChar
OutFixExit
		
		ADD SP, #8
		POP{R11,LR,R4,R5}
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
