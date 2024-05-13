List of things that were changed as a low-level optimization compared to the original implementation:
- Matrix multiplication assembly method generation
- Matrix multiplication replaced by assembly reference in all positions in the code
- Changed 'pi' and 'SF' functions so we can use a matrix multiplication function that stores directly into the result matrix
- Matrix systemization assembly method generation
- Matrix systemization replaced by assembly reference in all positions in the code
- Functions using various systemization sizes altered to include a function pointer to the assembly systemization reference