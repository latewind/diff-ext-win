/*
 * Copyright (c) 2003-2010, Sergey Zorin. All rights reserved.
 *
 * This software is distributable under the BSD license. See the terms of the
 * BSD license in the LICENSE file provided with this software.
 *
 */
#include <sys/types.h>
#include <io.h>
#include <rxposix.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/********************************************************************
*command line args:                                                 *
*  --input  - input file name. if '-' is specified, or unspecified  *
*             standart input is used.                               *
*  --output - output file name. if '-' is specified, or unspecified *
*             standart output is used.                              *
*  --help   - prints out usage help.                                *
********************************************************************/
/*typedef int (*UPDATE_FUNCTION)(int);*/
typedef struct UPDATE_INTERFACE {
  int (*update)(struct UPDATE_INTERFACE* this, int x);
} UPDATE;

typedef struct {
  UPDATE _super;
  int _x;
} SET;

int
increment(struct UPDATE_INTERFACE* this, int x) {
  return x+1;
}

int 
decrement(struct UPDATE_INTERFACE* this, int x) {
  return x-1;
}

int
set(struct UPDATE_INTERFACE* super, int x) {
  SET* this = (SET*)super;
  
  return this->_x;
}

int
keep(struct UPDATE_INTERFACE* this, int x) {
  return x;
}

int 
min(int x1, int x2) {
  int ret = x1;
  
  if(x2 < x1) {
    ret = x2;
  }
  
  return ret;
}

void
usage(char* program_name) {
  char* message = 
  "%s usage:\n"
  "%s [options]\n"
  "\n"
  "Options:\n"
  "\t--input\t\tinput file name. if '-' is specified, or unspecified\n"
  "\t\t\tstandart input is used.\n"
  "\t--output\toutput file name. if '-' is specified, or unspecified\n"
  "\t\t\tstandart output is used.\n"
  "\t--help\t\tprints out usage help.\n";

  printf(message, program_name, program_name);
}

int
init(char* update_string, UPDATE* update[4]) {
  int result = -1;
/*  char* update_re_str = "\\([[:digit:]]+|\\+|\\*\\)\\.\\([[:digit:]]+|\\+|\\*\\)\\.\\([[:digit:]]+|\\+|\\*\\)\\.\\([[:digit:]]+|\\+|\\*\\)";*/
  char* update_re_str = 
    "\\([[:digit:]]\\+\\|\\*\\|+\\|-\\)\\."
    "\\([[:digit:]]\\+\\|\\*\\|+\\|-\\)\\."
    "\\([[:digit:]]\\+\\|\\*\\|+\\|-\\)\\."
    "\\([[:digit:]]\\+\\|\\*\\|+\\|-\\)";
  regex_t update_re;
  regmatch_t version[5];
  char buffer[4096];
  int ret;
  
  memset(&update_re, 0, sizeof(regex_t));
  
  if((ret = regcomp(&update_re, update_re_str, REG_ICASE)) == 0) {
    if(regexec(&update_re, update_string, 5, version, 0) == 0) {
      int n;
      
      free(update[0]);
      free(update[1]);
      free(update[2]);
      free(update[3]);
      
      if(version[1].rm_so >= 0) { 
        n = min(version[1].rm_eo-version[1].rm_so, 4096);
        strncpy(buffer, update_string+version[1].rm_so, n);
        buffer[n] = '\0';
        
        if(buffer[0] == '*') {
          update[0] = malloc(sizeof(UPDATE));
          update[0]->update = keep;
        } else if(buffer[0] == '+') {
          update[0] = malloc(sizeof(UPDATE));
          update[0]->update = increment;
        } else if(buffer[0] == '-') {
          update[0] = malloc(sizeof(UPDATE));
          update[0]->update = decrement;
        } else {
          int x = atoi(buffer);
          SET* tmp = malloc(sizeof(SET));
          
          tmp->_super.update = set;
          tmp->_x = x;
          update[0] = tmp;
        }
      }
      if(version[2].rm_so > 0) { 
        n = min(version[2].rm_eo-version[2].rm_so, 4096);
        strncpy(buffer, update_string+version[2].rm_so, n);
        buffer[n] = '\0';
        
        if(buffer[0] == '*') {
          update[1] = malloc(sizeof(UPDATE));
          update[1]->update = keep;
        } else if(buffer[0] == '+') {
          update[1] = malloc(sizeof(UPDATE));
          update[1]->update = increment;
        } else if(buffer[0] == '-') {
          update[1] = malloc(sizeof(UPDATE));
          update[1]->update = decrement;
        } else {
          int x = atoi(buffer);
          SET* tmp = malloc(sizeof(SET));
          
          tmp->_super.update = set;
          tmp->_x = x;
          update[1] = tmp;
        }
      }
      if(version[3].rm_so > 0) { 
        n = min(version[3].rm_eo-version[3].rm_so, 4096);
        strncpy(buffer, update_string+version[3].rm_so, n);
        buffer[n] = '\0';
        
        if(buffer[0] == '*') {
          update[2] = malloc(sizeof(UPDATE));
          update[2]->update = keep;
        } else if(buffer[0] == '+') {
          update[2] = malloc(sizeof(UPDATE));
          update[2]->update = increment;
        } else if(buffer[0] == '-') {
          update[2] = malloc(sizeof(UPDATE));
          update[2]->update = decrement;
        } else {
          int x = atoi(buffer);
          SET* tmp = malloc(sizeof(SET));
          
          tmp->_super.update = set;
          tmp->_x = x;
          update[2] = tmp;
        }
      }
      if(version[4].rm_so > 0) { 
        n = min(version[4].rm_eo-version[4].rm_so, 4096);
        strncpy(buffer, update_string+version[4].rm_so, n);
        buffer[n] = '\0';
        
        if(buffer[0] == '*') {
          update[3] = malloc(sizeof(UPDATE));
          update[3]->update = keep;
        } else if(buffer[0] == '+') {
          update[3] = malloc(sizeof(UPDATE));
          update[3]->update = increment;
        } else if(buffer[0] == '-') {
          update[3] = malloc(sizeof(UPDATE));
          update[3]->update = decrement;
        } else {
          int x = atoi(buffer);
          SET* tmp = malloc(sizeof(SET));
          
          tmp->_super.update = set;
          tmp->_x = x;
          update[3] = tmp;
        }
      }
      
      result = 0;
    }
    
    regfree(&update_re);
  } else {
    regerror(ret, &update_re, buffer, sizeof(buffer)/sizeof(buffer[0]));
    fprintf(stderr, "update_version: %s (%s)\n", buffer, update_re_str);
  }

  return result;
}

void
update_version(FILE* input, FILE* output, UPDATE* update[4]) {
  char buffer[4096];
  char* fileversion_re_str =
    /* 1 */ "\\(^[[:space:]]*FILEVERSION[[:space:]]*\\)"
    /* 2 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 3 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 4 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 5 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 6 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 7 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 8 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 9 */ "\\([[:space:]]*$\\)";
  char* productversion_re_str =
    /* 1 */ "\\(^[[:space:]]*PRODUCTVERSION[[:space:]]*\\)"
    /* 2 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 3 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 4 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 5 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 6 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 7 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 8 */ "\\([[:digit:]][[:digit:]]*\\)"
    /* 9 */ "\\([[:space:]]*$\\)";
  char* fileversion_str_re_str =
    /* 1 */ "\\(^[[:space:]]*VALUE[[:space:]]*\\)"
    /* 2 */ "\\(\"FileVersion\"\\)"
    /* 3 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 4 */ "\\(\"[^\"]*\"\\)"
    /* 5 */ "\\([[:space:]]*$\\)";
  char* productversion_str_re_str =
    /* 1 */ "\\(^[[:space:]]*VALUE[[:space:]]*\\)"
    /* 2 */ "\\(\"ProductVersion\"\\)"
    /* 3 */ "\\([[:space:]]*,[[:space:]]*\\)"
    /* 4 */ "\\(\"[^\"]*\"\\)"
    /* 5 */ "\\([[:space:]]*$\\)";
  int ret;
  regex_t fileversion_re;
  regex_t productversion_re;
  regex_t fileversion_str_re;
  regex_t productversion_str_re;
  int re_compiled = 1;

  memset(&fileversion_re, 0, sizeof(regex_t));
  memset(&productversion_re, 0, sizeof(regex_t));

  if((ret=regcomp(&fileversion_re, fileversion_re_str, REG_ICASE)) != 0) {
    regerror(ret, &fileversion_re, buffer, sizeof(buffer)/sizeof(buffer[0]));
    fprintf(stderr, "update_version: %s (%s)\n", buffer, fileversion_re_str);
    re_compiled = 0;
  }
  
  if((ret=regcomp(&productversion_re, productversion_re_str, REG_ICASE)) != 0) {
    regerror(ret, &fileversion_re, buffer, sizeof(buffer)/sizeof(buffer[0]));
    fprintf(stderr, "update_version: %s (%s)\n", buffer, productversion_re_str);
    re_compiled = 0;
  }
  
  if((ret=regcomp(&fileversion_str_re, fileversion_str_re_str, REG_ICASE)) != 0) {
    regerror(ret, &fileversion_str_re, buffer, sizeof(buffer)/sizeof(buffer[0]));
    fprintf(stderr, "update_version: %s (%s)\n", buffer, fileversion_str_re_str);
    re_compiled = 0;
  }
  
  if((ret=regcomp(&productversion_str_re, productversion_str_re_str, REG_ICASE)) != 0) {
    regerror(ret, &productversion_str_re, buffer, sizeof(buffer)/sizeof(buffer[0]));
    fprintf(stderr, "update_version: %s (%s)\n", buffer, productversion_str_re_str);
    re_compiled = 0;
  }
  
  if(re_compiled) {
    int major = -1;
    int minor = -1;
    int patch = -1;
    int build = -1;
    int file_major = -1;
    int file_minor = -1;
    int file_patch = -1;
    int file_build = -1;
    int product_major = -1;
    int product_minor = -1;
    int product_patch = -1;
    int product_build = -1;
    
    while(fgets(buffer, 4096, input)) {
      regmatch_t version[10];
      int file_version = regexec(&fileversion_re, buffer, 10, version, 0);
      
      if((file_version == 0) || 
         (regexec(&productversion_re, buffer, 10, version, 0) == 0)) { /* version variable is reused, so we have to test file_version match first*/
        char tmp[8] = "";
        char space[5][4096] = {"", "", "", "", ""};
      
        if(version[1].rm_so >= 0) { /* may be 0 for the first one */
          strncpy(space[0], buffer+version[1].rm_so, min(version[1].rm_eo-version[1].rm_so, 4096));
        }
        if(version[2].rm_so > 0) {
          memset(tmp, 0, sizeof(tmp)/sizeof(tmp[0]));
          strncpy(tmp, buffer+version[2].rm_so, min(version[2].rm_eo-version[2].rm_so, 8));
          major = atoi(tmp);
        }
        if(version[3].rm_so > 0) {
          strncpy(space[1], buffer+version[3].rm_so, min(version[3].rm_eo-version[3].rm_so, 4096));
        }
        if(version[4].rm_so > 0) {
          memset(tmp, 0, sizeof(tmp)/sizeof(tmp[0]));
          strncpy(tmp, buffer+version[4].rm_so, min(version[4].rm_eo-version[4].rm_so, 8));
          minor = atoi(tmp);
        }
        if(version[5].rm_so > 0) {
          strncpy(space[2], buffer+version[5].rm_so, min(version[5].rm_eo-version[5].rm_so, 4096));
        }
        if(version[6].rm_so > 0) {
          memset(tmp, 0, sizeof(tmp)/sizeof(tmp[0]));
          strncpy(tmp, buffer+version[6].rm_so, min(version[6].rm_eo-version[6].rm_so, 8));
          patch = atoi(tmp);
        }
        if(version[7].rm_so > 0) {
          strncpy(space[3], buffer+version[7].rm_so, min(version[7].rm_eo-version[7].rm_so, 4096));
        }
        if(version[8].rm_so > 0) {
          memset(tmp, 0, sizeof(tmp)/sizeof(tmp[0]));
          strncpy(tmp, buffer+version[8].rm_so, min(version[8].rm_eo-version[8].rm_so, 8));
          build = atoi(tmp);
        }
        if(version[9].rm_so > 0) {
          strncpy(space[4], buffer+version[9].rm_so, min(version[9].rm_eo-version[9].rm_so, 4096));
        }
        
        major = update[0]->update(update[0], major);
        minor = update[1]->update(update[1], minor);
        patch = update[2]->update(update[2], patch);
        build = update[3]->update(update[3], build);
        
        fprintf(output, "%s%d%s%d%s%d%s%d%s", space[0], major, space[1], minor, space[2], patch, space[3], build, space[4]);
        
        if(file_version == 0) {
          file_major = major;
          file_minor = minor;
          file_patch = patch;
          file_build = build;
        } else {
          product_major = major;
          product_minor = minor;
          product_patch = patch;
          product_build = build;
        }
      } else if(((file_version = regexec(&fileversion_str_re, buffer, 10, version, 0)) == 0) || 
        (regexec(&productversion_str_re, buffer, 10, version, 0) == 0)) {
        char space[4][4096] = {"", "", "", ""};
        
        if(version[1].rm_so >= 0) { /* may be 0 for the first one */
          strncpy(space[0], buffer+version[1].rm_so, min(version[1].rm_eo-version[1].rm_so, 4096));
        }
        if(version[2].rm_so > 0) {
          strncpy(space[1], buffer+version[2].rm_so, min(version[2].rm_eo-version[2].rm_so, 4096));
        }
        if(version[3].rm_so > 0) {
          strncpy(space[2], buffer+version[3].rm_so, min(version[3].rm_eo-version[3].rm_so, 4096));
        }
        if(version[5].rm_so > 0) {
          strncpy(space[3], buffer+version[5].rm_so, min(version[5].rm_eo-version[5].rm_so, 4096));
        }
        
        if(file_version) {
          major = file_major;
          minor = file_minor;
          patch = file_patch;
        } else {
          major = product_major;
          minor = product_minor;
          patch = product_patch;
        }
        
        fprintf(output, "%s%s%s\"%d.%d.%d\"%s", space[0], space[1], space[2], major, minor, patch, space[3]);
      } else {
        fputs(buffer, output);
      }
    }
  }
  
  regfree(&fileversion_re);
  regfree(&productversion_re);
  regfree(&fileversion_str_re);
  regfree(&productversion_str_re);
}

int
main(int argc, char** argv) {
  FILE* input = stdin;
  FILE* output = stdout;
  char* input_name = "-";
  char* output_name = "-";
  int status = 0;
  int remove_input = 0;
  UPDATE* update[4] = {0, 0, 0, 0};
  
  update[0] = malloc(sizeof(UPDATE));
  update[1] = malloc(sizeof(UPDATE));
  update[2] = malloc(sizeof(UPDATE));
  update[3] = malloc(sizeof(UPDATE));
  
  update[0]->update = keep;
  update[1]->update = keep;
  update[2]->update = keep;
  update[3]->update = increment;
  
  if(argc > 1) {
    int index = 1;
    while(index < argc) {
      if(strcmp(argv[index], "--input") == 0) {
        index++;
        input_name = argv[index];
        index++;
        
        if(input_name == 0) {
          usage(argv[0]);
          index = argc;
        }
      } else if(strcmp(argv[index], "--output") == 0) {
        index++;
        output_name = argv[index];
        index++;
        if(output_name == 0) {
          usage(argv[0]);
          index = argc;
        }
      } else if(strcmp(argv[index], "--help") == 0) {
        usage(argv[0]);
        index = argc;
        output_name = 0;
        input_name = 0;
        input = 0;
        output = 0;
      } else {
        if(init(argv[index], update) != 0) {
          fprintf(stderr, "Unknown option '%s'\n\n", argv[index]);
          usage(argv[0]);
          index = argc;
          input_name = 0;
          output_name = 0;
          input = 0;
          output = 0;
          usage(argv[0]);
          return -1;
        } else {
          index++;
        }
      }
    }
  }
    
  if((input != 0) && (output != 0)) {    
    if((strcmp(input_name, output_name) == 0) && (strcmp(input_name, "-") != 0)) {
/*      char template_name[] = "iv_XXXXXX"; // was used with mktemp() */
      char* tmp_name = tmpnam(0);
      int ret;
      
      ret = rename(input_name, tmp_name);
      if(ret != 0) {
        perror("rename");
      }
      else {
        remove_input = 1;
        
        input = fopen(tmp_name, "r");
        if(input == 0) {
          perror("fopen(...,\"r\")");
          fprintf(stderr, "Can not open '%s' for reading.\n", tmp_name);
        }
        else {
          output = fopen(output_name, "w");
          
          if(output == 0) {
            perror(argv[0]);
            fprintf(stderr, "Can not open '%s' for writing.\n", output_name);
            rename(tmp_name, input_name);
          }
          
          input_name = tmp_name;
        }
      }
    } else {
      if(strcmp(output_name, "-") != 0) {
        output = fopen(output_name, "w");
        
        if(output == 0) {
          fprintf(stderr, "Can not open '%s' for writing.\n", output_name);
        }
      }
      
      if(strcmp(input_name, "-") != 0) {
        input = fopen(input_name, "r");
  
        if(input == 0) {
          fprintf(stderr, "Can not open '%s' for reading.\n", input_name);
        }
      }
    }
    
   update_version(input, output, update);
    
    if(input != stdin) {
      fclose(input);
    }
    
    if(output != stdout) {
      fflush(output);
      fclose(output);
    }
    
    if(remove_input) {
      int ret;
      
      ret = remove(input_name);
      
      if(ret != 0) {
        perror("Can not remove tmp file");
      }
    }
  } else {
    status = 1;
  }
  
  return status;
}
