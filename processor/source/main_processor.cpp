#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>

#include "processor.h"

int main(int argc, char *argv[]) { 
   
    if (argc != 2) {
        printf("НЕкорректный ввод");
        return 1;
    }

    Processor processor;
    Processor_err err = processor_init(&processor, 100);
    if (err != NO_ERROR) {
        printf("Ошибка инициализации процессора\n");
        return 1;
    }
    err = bite_code_read(&processor, argv[1]);
    if (err != NO_ERROR) {
        printf("Ошибка чтения бфйткода\n");
        return 1;
    }
    
    err = run_bytecode(&processor);
    if (err != NO_ERROR) {
        printf("Ошибка исполнения байткода\n");
        return 1;
    }

    printf("-----------------------------------\n");
    // processor_dump(&processor);    
    
    processor_destroy(&processor);
}