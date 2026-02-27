# Custom-C-CPP-STD  

(Work in progress)  
This is an experiment to see if you can fix the shortcomings of C within C++, while ignoring 95% of C++ features.  

Problems with C (in my opinion):  
1. const char* - strings have no length and are simply a pointer to one character. The length is determined by a char with the value of 0.
2. arrays not having length - When you pass an array to a function, it decays to a raw pointer without a length.
3. macros/raw integers as errors - errors should have a type, there should be a clear list of them and they should be printable.
4. Bad names in C standard library - atoi, itoa, perror etc.
5. C standard library presumes everything is valid
6. Variance of the length of integers. int can be 16 or 32 bits.
7. One pass parsing, header files, linking errors, forward declarations etc.
8. No defer or RAII. Early returns after allocations require goto or repetitive frees.

 
