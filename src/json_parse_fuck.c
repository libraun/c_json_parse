#include "stack.h"

#define BUFFER_SIZE 4096

struct JSON_OBJECT {
  char* key;
  void* val;

  char type;
  struct JSON_OBJECT* next;
};

struct JSON_OBJECT* parse_json(char *filename) {
  char buf[BUFFER_SIZE];
  memset(buf, 0, sizeof(buf));
  
  FILE *file_ptr = fopen(filename, "r");
  if (file_ptr != NULL) {

    fread(buf, sizeof(char), BUFFER_SIZE, file_ptr);
    fclose(file_ptr);
    long unsigned int len = strlen(buf);

    struct JSON_OBJECT *ret = parse_tokens(buf, len, 0, len);

    return ret;
  }
  return NULL;
}

void print_tabs(unsigned int num_tabs) {
  for (int i = 0; i < num_tabs; ++i) {

    printf("\t");
  }
}

void print_json_object(struct JSON_OBJECT* object,
		       unsigned int level) {
  struct JSON_OBJECT *iter_ptr = object;
  printf("{\n");
  while (iter_ptr != NULL) {
    print_tabs(level);
    printf("'%s' : ", iter_ptr->key);
    if (iter_ptr->type == NESTED_OBJECT_TYPE) {
      print_json_object((struct JSON_OBJECT *) iter_ptr->val,
			level+1);
      print_tabs(level);
      printf("},\n");
    } else {
      printf("'%s',\n",(char *) iter_ptr->val);
    }
    iter_ptr = iter_ptr->next;
  }
  if (level == 1) {
    printf("}\n");
  }
} 

long unsigned int find_nested_object_end(char buf[],
					 long unsigned int start,
					 long unsigned int size) {
  long unsigned int idx = start;
  unsigned int opening_bracket_count = 1;
  unsigned int closing_bracket_count = 0;

  while (opening_bracket_count != closing_bracket_count
	 && idx < size) {
    switch (buf[idx]) {
    case '{':
      opening_bracket_count = opening_bracket_count + 1;
      break;
    case '}':
      closing_bracket_count = closing_bracket_count + 1;
      break;
    default: break;
    }
    ++idx;
  }
  return idx;
}

void add_to_object(struct JSON_OBJECT *item,
		   struct JSON_OBJECT **head) {
  struct JSON_OBJECT *iter = *head;
  while (iter->next != NULL) {
    iter = iter->next;
  }
  iter->next = item;
}

struct JSON_OBJECT* parse_tokens(char buf[],
				 long unsigned int size,
				 long unsigned int start,
				 long unsigned int end) {
  if (buf[start] != '{') {
    printf("fuck\n");
    return NULL;
  }
  struct JSON_OBJECT* parsed_object =
    (struct JSON_OBJECT *) malloc(sizeof(struct JSON_OBJECT));
  parsed_object->key = "DEFAULT";
  parsed_object->val = "DEFAULT_VAL";

  parsed_object->type = SIMPLE_OBJECT_TYPE;
  
  struct JSON_OBJECT* iter_object =
    (struct JSON_OBJECT *) malloc(sizeof(struct JSON_OBJECT));

  parsed_object->next = iter_object;
  
  long unsigned int buf_iloc = start + 1;

  char cur_char;

  struct JSON_OBJECT *iter_ptr = iter_object;
  while (buf_iloc < end && buf[buf_iloc] != 0) {

    print_json_object(parsed_object,1);
    cur_char = buf[buf_iloc];
    
    switch(cur_char) {
    case '{':
      long unsigned int nested_obj_end =
	find_nested_object_end(buf, buf_iloc+1, end);
      if (nested_obj_end == size) return NULL;
      struct JSON_OBJECT *nested_object =
   	parse_tokens(buf,size,buf_iloc,nested_obj_end);

      iter_object->val = nested_object;
      iter_object->type = NESTED_OBJECT_TYPE;
      
      buf_iloc = nested_obj_end + 1;
      break;
      
    case '"':
      long unsigned int start_cpy = buf_iloc+1;
      do {
	++buf_iloc;
      } while (buf[buf_iloc] != '"'
	       && buf_iloc < end);
      
      char* token = (char *)malloc(sizeof(char) * (buf_iloc-start_cpy));
      strncpy(token,&buf[start_cpy],buf_iloc-start_cpy);
      if (iter_object->val != NULL) {
	if (iter_object->next == NULL) {
	  iter_object->next = malloc(sizeof(struct JSON_OBJECT));
	}
	iter_object = iter_object->next;
      }
      
      if (iter_object->key == NULL) {
	iter_object->key = token;
      } else {
	iter_object->val = token;
	iter_object->type = SIMPLE_OBJECT_TYPE;
      }
      ++buf_iloc;
      break;
      
    case ',':
      // add_to_object(iter_object,&parsed_object);
      //free(iter_object);
      iter_object->next = (struct JSON_OBJECT*)
	malloc(sizeof(struct JSON_OBJECT));
      //      iter_object = iter_object->next;
      
      ++buf_iloc;
      break;
      
    default:
      ++buf_iloc;
      break;
    }
  }
  return parsed_object;
}