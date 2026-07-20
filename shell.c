#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (strcmp(argv[1],"echo")==0){
        
     for (int i = 2 ; i < argc ; i++ ){
     if (i == (argc -1) ){
         
      printf("%s", argv[i]);   
     }
     else{
        
     printf ("%s",argv[i]);
     printf (" ");
     
     }
     }
         
         
     printf ("\n");}
     
     else if (strcmp(argv[1],"exit" )== 0){
         
         
         printf("Good Bye\n");
         
     }
       else {
            
            printf("Invalid command\n");
            
        }
        return 0;
    }
    

