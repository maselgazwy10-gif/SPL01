#include <stdio.h>

#include <unistd.h>
int pwd_main() {
    
char my_array[4096];    

char *x = getcwd (my_array ,4096);
if (x ==0)

printf("Error pwd failed");

else{
printf ("%s\n",my_array);
return 0;
}
    // Write your code here
    // Do not write a main() function. Instead, deal with pwd_main() as the main function of your program.
}
