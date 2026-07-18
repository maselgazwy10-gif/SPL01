#include <stdio.h>
#include <string.h>

#define NULLl 0
int echo_main(int argc, char *argv[]) {
    
    
   
    
    for (int i =1 ; i < argc ; i++){
        
        
         
   
    if (i== argc -1 ){ 
    printf ("%s", argv [i]);
    
    break ;
    }
     printf ("%s", argv[i]);
     printf (" ");
    
    
    }
    printf("\n");
return 0;
    // Write your code here
    // Do not write a main() function. Instead, deal with echo_main() as the main function of your program.
}
