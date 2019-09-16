#include <windows.h>
#include <stdio.h>


void nEraser(char* chaine)
{
    size_t i = 0;

    for(i = 0; i < strlen(chaine) + 1; i++)
        if(chaine[i] == '\n')
            chaine[i] = '\0';
}
