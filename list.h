//
//  Created by IvanBrekman on 03.11.2021.
//

const int BUFFER_SIZE = 10;

struct List {
    int buffer[BUFFER_SIZE] = {  0 };
    int   next[BUFFER_SIZE] = { -1 };

    int head = -1;
    int tail = -1;
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

enum list_errors {
    OK = 0,

    INVALID_LIST_PTR = -1,

    INCORRECT_HEAD_INDEX = -2,
    INCORRECT_TAIL_INDEX = -3,

    LST_EMPTY            = -4,
    LST_FULL             = -5,

    BAD_PH_INDEX         = -6
};

int list_ctor(List* lst);
int list_dtor(List* lst);

const char* list_error_desc(int error_code);
int list_error(List* lst);

int find_free_cell(List* lst, int start_index);

int push_back(List* lst, int value);
int  pop_back(List* lst);

int push_after(List* lst, int value, int ph_index);
int  pop_after(List* lst, int ph_index);

void print_list(List* lst, const char* sep=", ", const char* end="\n");
void  list_dump(List* lst, const char* reason, FILE* log=stdout, const char* sep=", ", const char* end="\n");
