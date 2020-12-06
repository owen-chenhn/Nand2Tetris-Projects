Project 8 - MPCS 52011 - Intro to Computer Systems

Name: Haoning Chen

Notes to grader:
1.  The code "VMTranslate.cpp" accepts a directory as input and searches all the vm files under the directory. But 
this code can only work in Windows system. The compilation of this code in Linux system will fail. 
To complie the code (in Windows): 
    make VMTranslate
or (if make command is not available):
     g++ -std=c++11 -Wall -O2 -lm VMTranslate.cpp -o VMTranslate

To run the code:
    ./VMTranslate [/path/to/directory]

Adding bootstrap makes the test of BasicLoop, FibonacciSeries and SimpleFunction fail. Hence, to test these three tests,
simply comment the line that calls function bootstrap() in the code. 
E.g., to run the test with bootstrap (like FibonacciElement and StaticsTest), remain the code as:
    // Write bootstrap code
    bootstrap(w_f);

To run the test without bootstrap (for BasicLoop, FibonacciSeries and SimpleFunction):
    // Write bootstrap code
    // bootstrap(w_f);


2. If you must grade my code in Linux or Mac system, you can use VMTranslate2.cpp, which can run in 
all the systems. But this code only takes a list of vm files under the same directory as input arguments. Because I didn't 
have time to implement the file search functionality under specified directory. So it only accepts a list of vm files as input.
But the output asm file has correct file name, i.e., the name of the directory. 
(Hope this will not cause points being deducted.)

To run VMTranslate2.cpp (e.g., in Linux system):
    make VMTranslate2
    ./VMTranslate2 [path/to/directory/file1.vm] [path/to/directory/file2.vm] [path/to/directory/file3.vm] ......

The others are the same. Commenting the function call of bootstrap to test BasicLoop, FibonacciSeries and SimpleFunction 
can also work in VMTranslate2.cpp. 