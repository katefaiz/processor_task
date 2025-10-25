#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>

#include "assembler.h"


int main(int argc, char *argv[]) { 
    
    if (argc != 3) {
        printf("НЕкорректный ввод");
        return 1;
    }
    Assembler assembler;
    Assembler_err err = assembler_init(&assembler);
    
    if (err != ASS_NO_ERROR) {
        printf("Ошибка инициализации ассемблера\n");
        return 1;
    }

    err = assembler_compile(&assembler, argv[1], argv[2]);
    if (err != ASS_NO_ERROR) {
        printf("Ошибка компиляции\n");
        return 1;
    }

    err = assembler_save_to_file(&assembler);
    if (err != ASS_NO_ERROR) {
        printf("Ошибка сохранения байт-кода\n");
        return 1;
    }
    
    assembler_destroy(&assembler);

    return 0;
}