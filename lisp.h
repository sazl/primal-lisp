#ifndef _LISP_H
#define _LISP_H

/*----------------------------------------------------------------------------*/

typedef enum   LispType     LispType;
typedef struct LispString   LispString;
typedef struct LispList     LispList;
typedef union  LispValUnion LispValUnion;
typedef struct LispVal      LispVal;

/*----------------------------------------------------------------------------*/

enum LispType {
    LISP_BOOLEAN,
    LISP_NUMBER,
    LISP_ATOM,
    LISP_STRING,
    LISP_LIST
};

struct LispString {
    unsigned int length;
    char*        data;
};

struct LispList {
    LispVal*  head;
    LispList* tail;
};

union LispValUnion {
    bool        boolean;
    int         number;
    LispString* str;
    LispList*   list;
};

struct LispVal {
    LispType     type;
    LispValUnion value;
};

/*----------------------------------------------------------------------------*/

LispString* lisp_string_new   (size_t length, char* data);
LispString* lisp_string_clone (LispString* str          );
void        lisp_string_free  (LispString* str          );

/*----------------------------------------------------------------------------*/

LispList* lisp_list_new     (void                         );
LispList* lisp_list_clone   (LispList* list               );
LispList* lisp_list_cons    (LispVal* head, LispList* tail);
LispList* lisp_list_reverse (LispList* list               );
void      lisp_list_print   (LispList* list               );
void      lisp_list_free    (LispList* list               );

/*----------------------------------------------------------------------------*/

LispVal* lisp_val_new       (LispType type            );
LispVal* lisp_val_clone     (LispVal* val             );
void     lisp_val_free      (LispVal* val             );
LispVal* lisp_val_num       (int num                  );
LispVal* lisp_val_string    (size_t length, char* data);
LispVal* lisp_val_atom      (size_t length, char* data);
LispVal* lisp_val_list      (LispList* list           );
void     lisp_val_print     (LispVal* val             );
bool     lisp_val_is_atomic (LispVal* val             );

/*----------------------------------------------------------------------------*/

LispVal* lisp_read (FILE* f);
LispVal* lisp_eval (LispVal* val);

/*----------------------------------------------------------------------------*/

#endif
