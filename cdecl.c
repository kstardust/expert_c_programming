#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_TOKEN_LENGTH 256
#define MAX_TOKENS 256

#define STRCMP(s, op, c) (strcmp((s), (c)) op 0)

#define STACK_INIT(s) ((s)->sp = -1)
#define STACK_TOP(s) ((s)->stack[(s)->sp])
#define STACK_PUSH(s, t) ((s)->stack[++((s)->sp)] = t)
#define STACK_POP(s) ((s)->stack[((s)->sp)--])
#define STACK_EMPTY(s) ((s)->sp == -1)

enum TokenType { TOKEN_IDENTIFIER, TOKEN_QUALIFIER, TOKEN_TYPE};

struct token {
  char type;
  char string[MAX_TOKEN_LENGTH];
};

struct token_stack {
  int sp;
  struct token stack[MAX_TOKENS];
};

enum TokenType
get_token_type(const char* token)
{
  if (STRCMP(token, ==, "const")) return TOKEN_QUALIFIER;
  if (STRCMP(token, ==, "volatile")) return TOKEN_QUALIFIER;
  if (STRCMP(token, ==, "unsigned")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "signed")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "char")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "int")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "double")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "float")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "union")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "struct")) return TOKEN_TYPE;
  if (STRCMP(token, ==, "void")) return TOKEN_TYPE;  
  return TOKEN_IDENTIFIER;
}

int
get_next_token(const char *cp, struct token* t, size_t *len)
{
  char c = 0;
  size_t i = 0;
  int length = 0;

  while (isspace(cp[i])) {
    i++;
  }

  while ((c = cp[i])) {
    if (isspace(c)) {
      break;
    }
    i++;
    if (c == '(' || c == ')' || c == '[' || c == ']'|| c == '*') {
      if (length == 0) {
        t->type = c;
	*len = i;
        return 1;
      } else {
	// move back	
	i--;
	break;
      }
    }
    
    t->string[length++] = c;
  }

  *len = i;
  t->string[length] = 0;
  t->type = get_token_type(t->string);

  return length != 0;
}

size_t
read_until_first_identifier(struct token_stack *sp, const char *cp)
{
  struct token t;
  size_t offset = 0, n = 0;
  while ((get_next_token(cp+offset, &t, &n))) {
    offset += n;
    STACK_PUSH(sp, t);
    if (t.type == TOKEN_IDENTIFIER) {
      // clear n
      n = 0;
      break;
    }
  }
  offset += n;
  return offset;
}

void
print_stack(struct token_stack s)
{
  struct token_stack *sp = &s;
  struct token t;
  while (!STACK_EMPTY(sp)) {
    t = STACK_POP(sp);
    switch (t.type) {
    case TOKEN_QUALIFIER:
    case TOKEN_IDENTIFIER:
    case TOKEN_TYPE:
      printf("Type: %d String: %s\n", t.type, t.string);
      break;
    default:
      printf("%c\n", t.type);
      break;
    }
  }
}

void
print_token(struct token t)
{
  switch (t.type) {
  case TOKEN_IDENTIFIER:
    printf("%s ", t.string);
    break;
  case TOKEN_TYPE:
    printf("%s ", t.string);
    break;
  case TOKEN_QUALIFIER:
    printf("%s ", t.string);    
    break;
  case '*':
    printf("pointer to ");
    break;
  case '[':
    printf("array of ");
    break;    
  case '(':
    printf("function returning ");
    break;    
  default:
    printf("invalid type");
    break;
  };
}

size_t
process_array(const char* input)
{
  int i = 0;
  int brackets = 1;
  char c = 0;
  while (brackets != 0 && (c = input[i++])) {
    if (c == '[')
      brackets++;
    else if (c == ']')
      brackets--;
  }
  return i;
}

size_t
process_function(const char* input)
{
  int i = 0;
  int braces = 1;
  char c = 0;
  while (braces != 0 && (c = input[i++])) {
    if (c == '(')
      braces++;
    else if (c == ')')
      braces--;
  }
  return i;
}

void
parse_remain(struct token_stack *sp, const char *input)
{
  struct token t;
  size_t offset = 0, n = 0;
  while ((get_next_token(input + offset, &t, &n))) {
    offset += n;
    // parse right side
    if (t.type == '[') {
      offset += process_array(input + offset);
      print_token(t);
    } else if (t.type == '(') {
      offset += process_function(input + offset);
      print_token(t);
    } else if (t.type == ')') {
      continue;
    }

    // left side
    while (!STACK_EMPTY(sp)) {
      t = STACK_POP(sp);
      if (t.type == '(') {
	break;
      } else {
	print_token(t);
	if (t.type == '*' || t.type == TOKEN_QUALIFIER) {
	  continue;
	}
	break;
      }
    }
  }

  while (!STACK_EMPTY(sp)) {
    print_token(STACK_POP(sp));
  }
}

void
cdecl(const char *input)
{
  struct token_stack s;
  struct token_stack* sp = &s;
  STACK_INIT(sp);

  size_t n = read_until_first_identifier(sp, input);
  print_stack(s);
  struct token id = STACK_POP(sp);
  if (id.type != TOKEN_IDENTIFIER) {
    printf("cannot find identifier\n");
    return;
  }

  print_token(id);
  printf("is a ");
  parse_remain(sp, input+n);
  putchar('\n');
}

int main(int argc, char ** argv)
{
  char *line = NULL;
  size_t size = 0;
  while (getline(&line, &size, stdin) > 0) {
    cdecl(line);
    free(line);
  }

  return 0;
}
