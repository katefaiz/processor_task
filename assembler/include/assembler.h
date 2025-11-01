#ifndef AS_H
#define AS_H

#include "stack_funks.h" 
#include "assembler_struct_enum.h"

Assembler_err assembler_init(Assembler *assembler);
Assembler_err assembler_compile(Assembler *assembler, const char *source_file);
Assembler_err assembler_resolve_labels(Assembler *assembler);
Assembler_err assembler_save_to_file(Assembler *assembler,  const char *output_file);
Assembler_err assembler_destroy(Assembler *assembler);
Registers_name comparing_registers(const char *reg_name);
Commands comparing_commands(const char *command);
Assembler_err disasm_commands_data(int program[]);
Assembler_err second_pass(Assembler *assembler, FILE *filestream);
Assembler_err first_pass(Assembler *assembler, FILE *filestream, int *code_size);
int is_comment_or_empty(const char *line, int ind);
void copy_command(const char *line, int *ind, char *command);
void skip_space(const char *line, int *ind);


#endif //AS_H