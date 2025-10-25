
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>

#include "commands_funks.h" 
#include "assembler.h"
#include "assembler_struct_enum.h"


Assembler_err assembler_init(Assembler *assembler) {
    assert(assembler != NULL);
    
    memset(assembler->program, 0, sizeof(assembler->program));
    memset(assembler->labels, -1, sizeof(assembler->labels)); 
    assembler->labels_count = 0;
    assembler->program_size = 0;
    
    return ASS_NO_ERROR;
}


void skip_space(const char *line, int *ind) {
    while (line[*ind] == ' ') 
        (*ind)++;
    return;
}
void copy_command(const char *line, int *ind, char *command) {
    size_t len_command = strcspn(&line[*ind], " \n"); 
    strncpy(command, &line[*ind], len_command);
    command[len_command] = '\0';
    *ind += len_command;
    return;
}
int is_comment_or_empty(const char *line, int ind) {
    return line[ind] == ';' || line[ind] == '\n' || line[ind] == '\0';
}


Assembler_err first_pass(Assembler *assembler, FILE *filestream, int *code_size) {
    char line[100] = {};
    *code_size = 0;

    while (fgets(line, sizeof(line), filestream)) {
        int ind = 0;
        skip_space(line, &ind);

        if (line[ind] == ';') 
            continue;
        
        if (line[ind] == ':') {
            ind++;
            int label_index = atoi(line + ind);
            
            if (label_index >= 0 && label_index < 20) { 
                assembler->labels[label_index] = *code_size;
            }
        }
        else if (line[ind] != '\n' && line[ind] != '\0') { 
            char command[10] = {};
            
            copy_command(line, &ind, command);
            
            Commands cmd_code = comparing_commands(command);
            
            if (cmd_code == PUSH || cmd_code == POPR || cmd_code == PUSHR || 
                cmd_code == JMP || cmd_code == CALL || cmd_code == POPM || cmd_code == PUSHM) {
                *code_size += 2; 
            }
            else if (cmd_code != ERROR) {
                *code_size += 1; 
            }
        }
    }
    return ASS_NO_ERROR;
}

Assembler_err second_pass(Assembler *assembler, FILE *filestream) {
    char line[100] = {};
    int count = 0;

    while (fgets(line, sizeof(line), filestream)) {
        char command[10] = {};
        char value_str[50] = {};
        int value = 0;
        int ind = 0;

        skip_space(line, &ind);

        if (line[ind] == ':') {
            ind++;
            while (line[ind] >= '0' && line[ind] <= '9') 
                ind++;
            skip_space(line, &ind);
            
            if (is_comment_or_empty(line, ind)) 
                continue;

        }
        
        if (is_comment_or_empty(line, ind)) 
            continue;

        copy_command(line, &ind, command);

        Commands cmd_code = comparing_commands(command);
        
        skip_space(line, &ind);

        if (cmd_code == PUSH) {
            if (line[ind] != '\0' && line[ind] != '\n') {
                value = atoi(line + ind);
                assembler->program[count++] = cmd_code;
                assembler->program[count++] = value;
            }    
        }
        else if (cmd_code == PUSHR || cmd_code == POPR || cmd_code == POPM || cmd_code == PUSHM) { 
            if (line[ind] != '\0' && line[ind] != '\n') {
                copy_command(line, &ind, value_str); //скопировали имя регистра
                //skip_space(line, &ind);

                Registers_name reg = comparing_registers(value_str);
                assembler->program[count++] = cmd_code;
                assembler->program[count++] = (int)reg;
            }
        }
        else if (cmd_code == JMP || cmd_code == CALL) {  
            if (line[ind] != '\0' && line[ind] != '\n') {
                if (line[ind] == ':') ind++;
                value = atoi(line + ind);
                assembler->program[count++] = cmd_code;
                assembler->program[count++] = value;
            }
        }
        else if (cmd_code != ERROR) {
            assembler->program[count++] = cmd_code;
        }
    }
    
    assembler->program_size = count;
    return ASS_NO_ERROR;
}

Assembler_err assembler_compile(Assembler *assembler, const char *source_file, const char *output_file) {
    assert(assembler != NULL);

    strncpy(assembler->source_file, source_file, sizeof(assembler->source_file) - 1);
    strncpy(assembler->output_file, output_file, sizeof(assembler->output_file) - 1);//xnj&&&&&&&&&&&&&&&&&&&
    FILE *filestream = fopen(assembler->source_file, "r");
    if (filestream == NULL) 
        return ASS_OPENFILE_ERROR;
    
    int code_size = 0;
 
    first_pass(assembler, filestream, &code_size);
    fseek(filestream, 0, SEEK_SET);
    second_pass(assembler, filestream);
    
    fclose(filestream);
    assembler_resolve_labels(assembler);
    return ASS_NO_ERROR;
}

Assembler_err assembler_resolve_labels(Assembler *assembler) { //из индекса метки получаю реальные адреса
    assert(assembler != NULL);
    
    for (int i = 0; i < assembler->program_size; i++) {
        if (assembler->program[i] == JMP || assembler->program[i] == CALL) {
            int label_index = assembler->program[i + 1]; //индекс метки в массиве меток (для :2 это 2)
            
            int address = assembler->labels[label_index];//адрес, куда указывает метка
            assembler->program[i + 1] = address;            
            i++; 
        }
    }
    return ASS_NO_ERROR;
}

Assembler_err assembler_save_to_file(Assembler *assembler) { // записывает массив с кодом в файл
    FILE *filestream = fopen(assembler->output_file, "w");
    if (filestream == NULL) {
        printf("Ошибка открытия файла \n");
        return ASS_OPENFILE_ERROR;
    }
    
    for (int i = 0; i < assembler->program_size; i++) {
        fprintf(filestream, "%d ", assembler->program[i]);
    }
    
    fclose(filestream);
    return ASS_NO_ERROR;

}

Assembler_err assembler_destroy(Assembler *assembler) {
    assert(assembler != NULL);
    
    memset(assembler->program, 0, sizeof(assembler->program));
    memset(assembler->labels, -1, sizeof(assembler->labels));
    assembler->program_size = 0;
    
    return ASS_NO_ERROR;
}

Commands comparing_commands(const char *command) { 
    if (strcmp(command, "PUSH") == 0)
        return PUSH;
    if (strcmp(command, "ADD") == 0)    
        return ADD;
    if (strcmp(command, "SUB") == 0)    
        return SUB;
    if (strcmp(command, "MUL") == 0)    
        return MUL;
    if (strcmp(command, "DIV") == 0)    
        return DIV;
    if (strcmp(command, "OUT") == 0)    
        return OUT;
    if (strcmp(command, "HLT") == 0)    
        return HLT;
    if (strcmp(command, "SQRT") == 0)   
        return SQRT;
    if (strcmp(command, "POPR") == 0)   
        return POPR;
    if (strcmp(command, "PUSHR") == 0)   
        return PUSHR;
    if (strcmp(command, "JMP") == 0)    
        return JMP;
    if (strcmp(command, "CALL") == 0)   
        return CALL;   
    if (strcmp(command, "RET") == 0)    
        return RET;
    if (strcmp(command, "POPM") == 0)   
        return POPM;
    if (strcmp(command, "PUSHM") == 0)   
        return PUSHM;
    if (strcmp(command, "DRAW") == 0)   
        return DRAW;
    else                                
        return ERROR;
}

Registers_name comparing_registers(const char *reg_name) {

    if (strcmp(reg_name, "RAX") == 0) 
        return RAX;
    if (strcmp(reg_name, "RBX") == 0) 
        return RBX;
    if (strcmp(reg_name, "RCX") == 0)     
        return RCX;
    if (strcmp(reg_name, "RDX") == 0) 
        return RDX;
    
    printf("Ошибка: неизвестный регистр '%s'\n", reg_name);
    return ROX; 
}


Assembler_err disasm_commands_data(int program[]) { // проверка ассемблирования
    int index = 0;
    while (program[index] != HLT) {
        switch (program[index]) {
            case 0:
                printf("HLT\n");
                break;
            case 1:
            printf("PUSH %d\n", program[index + 1]);
                index += 2;
                break;
            case 2:
                printf("ADD\n");
                index++;
                break;
            case 3:
                printf("SUB\n");
                index++;
                break;
            case 4:
                printf("MUL\n");
                index++;
                break;
            case 5:
                printf("DIV\n");
                index++;
                break;
            case 6:
                printf("OUT\n");
                index++;
                break;
            case 7:
                printf("SQRT\n");
                index++;
                break;
            case 42:
                printf("POPR\n");
                index+=2;
                break;
            case 33:
                printf("PUSHR\n");
                index+=2;
                break;
            
            case 666:
                printf("ERROR\n");
                index++;
                break;
            default:
                printf("ERROR!!!\n");          
        }      
    }
    
    return ASS_NO_ERROR;
}















