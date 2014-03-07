#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lisp.h"

/*----------------------------------------------------------------------------*/
/* LispString                                                                 */
/*----------------------------------------------------------------------------*/

LispString* lisp_string_new(size_t length, char* data)
{
    LispString* str = malloc(sizeof(LispString));
    str->length = length;
    str->data = malloc(sizeof(char) * (length+1));
    strncpy(str->data, data, length);
    str->data[length] = '\0';
    return str;
}

LispString* lisp_string_clone(LispString* str)
{
    LispString* clone = lisp_string_new(str->length, str->data);
    return clone;
}

void lisp_string_free(LispString* str)
{
    free(str->data);
    free(str);
}

/*----------------------------------------------------------------------------*/
/* LispList                                                                   */
/*----------------------------------------------------------------------------*/

LispList* lisp_list_new(void)
{
    LispList *list = malloc(sizeof(LispList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

LispList* lisp_list_clone(LispList* list)
{
    LispList* clone = NULL;
    while (list) {
        clone = lisp_list_cons(list->head, clone);
        list = list->tail;
    }
    return lisp_list_reverse(clone);
}

LispList* lisp_list_cons(LispVal* head, LispList* tail)
{
    LispList* list = lisp_list_new();
    list->head = head;
    list->tail = tail;
    return list;
}

LispList* lisp_list_reverse(LispList* list)
{
    LispList* next;
    LispList* previous = NULL;

    while (list) {
        next = list->tail;
        list->tail = previous;
        previous = list;
        list = next;
    }
    return previous;
}

void lisp_list_print(LispList* list)
{
    putchar('(');
    while (list) {
        if (list->head)
            lisp_val_print(list->head);
        list = list->tail;
        if (list)
            putchar(' ');
    }
    putchar(')');
}

void lisp_list_free(LispList* list)
{
    LispList* tmp;
    while (list) {
        tmp = list;
        if (list->head)
            lisp_val_free(list->head);
        list = list->tail;
        free(tmp);
    }
}

/*----------------------------------------------------------------------------*/
/* LispVal                                                                    */
/*----------------------------------------------------------------------------*/

LispVal* lisp_val_new(LispType type)
{
    LispVal* val = malloc(sizeof(LispVal));
    val->type = type;
    return val;
}

LispVal* lisp_val_clone(LispVal* val)
{
    if (!val)
        return NULL;
    
    switch (val->type) {
    case LISP_BOOLEAN:
        return NULL;
    case LISP_NUMBER:
        return lisp_val_num(val->value.number);
    case LISP_ATOM:
        return lisp_val_atom((val->value.str)->length, (val->value.str)->data);
    case LISP_STRING:
        return lisp_val_string((val->value.str)->length, (val->value.str)->data);
    case LISP_LIST: 
        return lisp_val_list(val->value.list);
    default:
        return NULL;
    }
}

void lisp_val_free(LispVal* val)
{
    switch (val->type) {
    case LISP_ATOM:
    case LISP_STRING:
        lisp_string_free(val->value.str);
        break;
    case LISP_LIST:
        lisp_list_free(val->value.list);
        break;
    default:
        break;
    }
    free(val);
}

LispVal* lisp_val_num(int num)
{
    LispVal* val = lisp_val_new(LISP_NUMBER);
    val->value.number = num;
    return val;
}

LispVal* lisp_val_string(size_t length, char* data)
{
    LispVal* val = lisp_val_new(LISP_STRING);
    val->value.str = lisp_string_new(length, data);
    return val;
}

LispVal* lisp_val_atom(size_t length, char* data)
{
    LispVal* val = lisp_val_new(LISP_ATOM);
    val->value.str = lisp_string_new(length, data);
    return val;
}

LispVal* lisp_val_list(LispList* list)
{
    LispVal* val = lisp_val_new(LISP_LIST);
    val->value.list = lisp_list_clone(list);
    return val;
}

void lisp_val_print(LispVal* val)
{
    if (!val) {
        printf("NULL");
        return;
    }
    
    switch (val->type) {
    case LISP_BOOLEAN:
        break;
    case LISP_NUMBER:
        printf("%d", val->value.number);
        break;
    case LISP_ATOM:
        printf("%s", (val->value.str)->data);
        break;
    case LISP_STRING:
        printf("\"%s\"", (val->value.str)->data);
        break;
    case LISP_LIST: 
        lisp_list_print(val->value.list);
        break;
    }
}

bool lisp_val_is_atomic(LispVal* val)
{
    return (val->type != LISP_LIST);
}

/*----------------------------------------------------------------------------*/
/* Parser                                                                     */
/*----------------------------------------------------------------------------*/

char peek(FILE* f)
{
    char c = fgetc(f);
    ungetc(c, f);
    return c;
}

char get_peek(FILE* f)
{
    fgetc(f);
    return peek(f);
}

void eat_space(FILE* f)
{
    char c = peek(f);
    while (isspace(c))
        c = get_peek(f);
}

LispVal* parse_number(FILE* f)
{
    int num = 0;
    char c = peek(f);
    if (!isdigit(c))
        return NULL;

    while (isdigit(c)) {
        num = (num*10) + (c-'0');
        c = get_peek(f);
    }
    return lisp_val_num(num);
}

LispVal* parse_atom(FILE* f)
{
    int length = 0;
    char data[100];
    char c = peek(f);

    while (isalpha(c) && (!feof(f))) {
        data[length] = c;
        length++;
        c = get_peek(f);
    }
    return lisp_val_atom(length, data);
}

LispVal* parse_string(FILE* f)
{
    int length = 0;
    char data[4096];
    char c = peek(f);
    if (c != '"')
        return NULL;

    c = get_peek(f);
    while ((c != '"') && (!feof(f))) {
        data[length] = c;
        length++;
        c = get_peek(f);
    }
    fgetc(f);
    return lisp_val_string(length, data);
}

LispVal* parse_list(FILE* f)
{
    char c = peek(f);
    if (c != '(')
        return NULL;
    
    fgetc(f);
    LispList* list = NULL;
    LispVal* val = lisp_read(f);

    while (val != NULL) {
        list = lisp_list_cons(val, list);
        val = lisp_read(f);
    }
    list = lisp_list_reverse(list);
    return lisp_val_list(list);
}

LispVal* lisp_read(FILE* f)
{
    eat_space(f);
    char c = peek(f);
    if (isdigit(c))
        return parse_number(f);
    else if (isalpha(c))
        return parse_atom(f);
    else if (c == '"')
        return parse_string(f);
    else if (c == '(')
        return parse_list(f);
    else
        return NULL;
}

/*----------------------------------------------------------------------------*/
/* Evaluator                                                                  */
/*----------------------------------------------------------------------------*/

LispVal* lisp_eval(LispVal* val)
{
    return lisp_val_clone(val);
}

void lisp_repl_file(FILE* f)
{
    while (!feof(f)) {
        LispVal* val = lisp_read(f);
        if (!val)
            break;
        LispVal* res = lisp_eval(val);
        lisp_val_print(res);
        putchar('\n');
        lisp_val_free(val);
        lisp_val_free(res);
    }
}

/*----------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    FILE* f = fopen(argv[1], "r");
    lisp_repl_file(f);
    return 0;
}
