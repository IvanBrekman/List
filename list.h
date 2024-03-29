//
//  Created by IvanBrekman on 03.11.2021.
//

#ifndef LIST_LISTH
#define LIST_LISTH

const int BUFFER_DEFAULT_SIZE = 10;
const int MAX_NODE_STR_SIZE   = 300;

typedef int List_t;

// List structure--------------------------------------------------------------
struct ListElement {
    List_t value;
    int    next;
    int    prev;
};

struct List {
    ListElement* data = NULL;

    int head = -1;
    int tail = -1;

    int capacity   = -1;

    int is_sorted  = -1;
    int first_free = -1;
};
// ----------------------------------------------------------------------------

#define ASSERT_OK(obj, reason, ret) {                                               \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE && list_error(obj)) {                       \
        list_dump(obj, reason);                                                     \
        LOG_DUMP(obj, reason, list_dump);                                           \
        LOG_DUMP_GRAPH(obj, reason, list_dump_graph)                                \
                                                                                    \
        ASSERT_IF(0, "verify failed", ret);                                         \
    } else if (list_error(obj)) {                                                   \
        LOG_DUMP_GRAPH(obj, reason, list_dump_graph)                                \
        errno = list_error(obj);                                                    \
        return ret;                                                                 \
    }                                                                               \
}

#define ERROR_DUMP(obj, reason, ret) {                                              \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE) {                                          \
        list_dump(obj, reason);                                                     \
        LOG_DUMP(obj, reason, list_dump);                                           \
        LOG_DUMP_GRAPH(obj, reason, list_dump_graph)                                \
                                                                                    \
        ASSERT_IF(0, reason, ret);                                                  \
    }                                                                               \
}

enum errors {
    OK                   =   0,

    INVALID_LIST_PTR     =  -1,

    INCORRECT_HEAD_INDEX =  -2,
    INCORRECT_TAIL_INDEX =  -3,
    INCORRECT_CAPACITY   =  -4,
    INCORRECT_SORTED_VAL =  -5,
    INCORRECT_FIFST_FREE =  -6,

    LST_EMPTY            =  -7,

    BAD_PH_INDEX         =  -8,
    BAD_LOG_INDEX        =  -9,
    NOT_ENOUGH_MEMORY    = -10
};

int list_ctor(List* lst, int capacity=BUFFER_DEFAULT_SIZE);
int list_dtor(List* lst);

const char* list_error_desc(int error_code);
int         list_error(List* lst);

// Help functions--------------------------------------------------------------
int       find_free_cell(List* lst);
int resize_list_capacity(List* lst, int new_size);
int please_dont_use_sorted_by_next_values_func_because_it_too_slow__also_do_you_really_need_it__i_think_no__so_dont_do_stupid_things_and_better_look_at_memes_about_cats(List* lst);
// ----------------------------------------------------------------------------

List_t get(List* lst, int log_index);
void fill_list_element(ListElement* el_ptr, List_t value, int next, int prev);

// Push/pop functions----------------------------------------------------------
int   push_index(List* lst, List_t value, int ph_index);
List_t pop_index(List* lst, int ph_index);

int    push_back(List* lst, List_t value);
List_t  pop_back(List* lst);

int   push_front(List* lst, List_t value);
List_t pop_front(List* lst);
// ----------------------------------------------------------------------------

// Info functions--------------------------------------------------------------
int      print_list(List* lst, const char* sep=", ", const char* end="\n");
int       list_dump(List* lst, const char* reason, FILE* log=stdout, const char* sep=", ", const char* end="\n");
int list_dump_graph(List* lst, const char* reason, FILE* log,        const char* sep=", ", const char* end="\n");
// ----------------------------------------------------------------------------

#endif // LIST_LISTH
