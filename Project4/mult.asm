// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.
// move the smaller value of R0 and R1 to R3, greater one to R2

@R1
D = M 
@R0 
D = D - M;  // D = R1 - R0
@MovR0
D; JGE 
// Move R1 to R3, R0 to R4
@R1
D = M
@R3
M = D 
@R0 
D = M 
@R4 
M = D
@Mul
0; JMP

(MovR0)     // Move R0 to R3, R1 to R4
@R0
D = M 
@R3 
M = D
@R1 
D = M 
@R4 
M = D

(Mul)   // Compute multiplication, using value in R3 and R4
@R2 
M = 0
// Check if R3 is 0
@R3 
D = M
@Finished
D; JEQ
// R3 is not 0. 
(Loop)
@R4 
D = M
@R2 
M = M + D
@R3 
MD = M - 1
@Finished
D; JEQ 
@Loop 
0; JMP

(Finished)  // The program is finished.
@Finished
0; JMP