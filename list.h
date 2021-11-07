//
//  Created by IvanBrekman on 03.11.2021.
//

const int BUFFER_DEFAULT_SIZE = 10;

typedef int List_t;

struct ListElement {
    List_t value;
    int    next;
};

struct List {
    ListElement* data = NULL;

    int head = -1;
    int tail = -1;

    int capacity = -1;
};

#define ASSERT_OK(obj, reason) {                                                    \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE && list_error(obj)) {                       \
        list_dump(obj, reason);                                                     \
        if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                   \
            FILE* log = open_file("log.txt", "a");                                  \
            list_dump(obj, reason, log);                                            \
            fclose(log);                                                            \
        }                                                                           \
        assert(0 && "verify failed");                                               \
    } else if (list_error(obj)) {                                                   \
        errno = list_error(obj);                                                    \
    }                                                                               \
}

#define ERROR_DUMP(obj, reason) {                                                   \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE) {                                          \
        list_dump(obj, reason);                                                     \
        if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                   \
            FILE* log = open_file("log.txt", "a");                                  \
            list_dump(obj, reason, log);                                            \
            fclose(log);                                                            \
        }                                                                           \
        assert(0 && reason);                                                        \
    }                                                                               \
}

enum errors {
    OK                   =  0,

    INVALID_LIST_PTR     = -1,

    INCORRECT_HEAD_INDEX = -2,
    INCORRECT_TAIL_INDEX = -3,
    INCORRECT_CAPACITY   = -4,

    LST_EMPTY            = -5,

    BAD_PH_INDEX         = -6,
    NOT_ENOUGH_MEMORY    = -7
};

int list_ctor(List* lst);
int list_dtor(List* lst);

const char* list_error_desc(int error_code);
int         list_error(List* lst);

int       find_free_cell(List* lst, int start_index);
int resize_list_capacity(List* lst, int new_size);

int    push_back(List* lst, List_t value);
List_t  pop_back(List* lst);

int   push_after(List* lst, List_t value, int ph_index);
List_t pop_after(List* lst, int ph_index);

void  print_list(List* lst, const char* sep=", ", const char* end="\n");
void   list_dump(List* lst, const char* reason, FILE* log=stdout, const char* sep=", ", const char* end="\n");
