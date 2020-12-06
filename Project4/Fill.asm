// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.
@SCREEN 
D = A 
@pointer    // Variable that stores the current address of screen register. 
M = D

(LOOP)
@KBD    // Detect the keyboard
D = M 
@REVERSE
D; JEQ      // Jump to reverse brunch if keyboard does not have input.  
@KBD   
D = A   // Check if the pointer reach boundary
@pointer 
D = D - M 
@LOOP 
D; JEQ      // Boundary has been reached. 
@pointer 
A = M 
M = -1 
@pointer 
M = M + 1
@LOOP 
0; JMP

(REVERSE)
@pointer 
D = M
@SCREEN 
D = D - A
@LOOP 
D; JEQ      // Current pointer already points to the start address of screen. 
@pointer 
AM = M - 1
M = 0
@LOOP 
0; JMP