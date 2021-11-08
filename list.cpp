//
//  Created by IvanBrekman on 03.11.2021.
//

#include "config.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cerrno>

#include "libs/baselib.h"
#include "libs/file_funcs.h"

#include "list.h"

#define UN poisons::UNINITIALIZED_INT
#define FR poisons::FREED_ELEMENT

//! List Constructor
//! \param lst      ptr to List object
//! \param capacity start List capacity (default BUFFER_DEFAULT_SIZE)
//! \return         1 if success, ese 0
int list_ctor(List* lst, int capacity) {
    ASSERT_IF(VALID_PTR(lst), "Invalid lst ptr", 0);
    ASSERT_IF(capacity > 0,   "Incorrect capacity: (<= 0)", 0);
    
    lst->head = lst->tail = 0;
    lst->is_sorted = 1;
    lst->capacity = capacity;

    lst->data = (ListElement*) calloc(capacity, sizeof(ListElement));

    for (int i = 0; i < capacity; i++) {
        lst->data[i] = {
            .value = (List_t)UN,
            .next  = i + 1,
            .prev  = UN
        };
    }

    lst->data[0].next = 0;
    lst->data[0].prev = 0;
    lst->data[capacity - 1].next = 0;
    lst->first_free = 1;

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        list_dump(lst, "Check init");
    }

    ASSERT_OK(lst, "Check corectness of list_ctor", 0);
    return 1;
}

//! List Destructor
//! \param lst      ptr to List object
//! \return         1 if success, ese 0
int list_dtor(List* lst) {
    ASSERT_OK(lst, "Check List before dtor call", 0);

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        int capacity = lst->capacity;
        for (int i = 0; i < capacity; i++) {
            lst->data[i] = {
                .value = (List_t)FR,
                .next  = FR,
                .prev  = FR
            };
        }
    }

    lst->capacity = lst->head = lst->tail = -1;
    lst->is_sorted  = -1;
    lst->first_free = -1;

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        list_dump(lst, "Check deinit");
    }

    FREE_PTR(lst->data, ListElement);
    return 1;
}

//! Get string description of error code
//! \param error_code code of error
//! \return           string error description
const char* list_error_desc(int error_code) {
    switch (error_code)
    {
        case errors::OK:
            return "ok";
        case errors::INVALID_LIST_PTR:
            return "Invalid list pointer";
        case errors::INCORRECT_HEAD_INDEX:
            return "Incorrect head index: index (< 0) or (>= capacity)";
        case errors::INCORRECT_TAIL_INDEX:
            return "Incorrect tail index: index (< 0) or (>= capacity)";
        case errors::INCORRECT_CAPACITY:
            return "Incorrect capacity: capacity (<= 0)";
        case errors::INCORRECT_SORTED_VAL:
            return "Incorrect is_sorted field: (< 0) or (> 1)";
        case errors::INCORRECT_FIFST_FREE:
            return "Incorrect first_free index: (< 0) or (>= capacity)";
        case errors::LST_EMPTY:
            return "List is empty";
        case errors::BAD_PH_INDEX:
            return "Function received bad physical index. Check element on this index, it most likely damaged";
        case errors::BAD_LOG_INDEX:
            return "Function received bad logical index. No element at this logical index";
        case errors::NOT_ENOUGH_MEMORY:
            return "Not enough memory to increase capacity";
        
        default:
            return "Unknown error";
    }
}

//! Function to detect errors in list
//! \param lst pointer to List object
//! \return    error code (0 if all is good)
int list_error(List* lst) {
    if (!VALID_PTR(lst)) {
        return errors::INVALID_LIST_PTR;
    }

    if (lst->capacity <= 0) {
        return errors::INCORRECT_CAPACITY;
    }

    if (0 > lst->first_free || lst->first_free >= lst->capacity) {
        return errors::INCORRECT_FIFST_FREE;
    }
    if (0 > lst->is_sorted || lst->is_sorted > 1) {
        return errors::INCORRECT_SORTED_VAL;
    }
    if (0 > lst->head || lst->head >= lst->capacity) {
        return errors::INCORRECT_HEAD_INDEX;
    }
    if (0 > lst->tail || lst->tail >= lst->capacity) {
        return errors::INCORRECT_TAIL_INDEX;
    }

    return errors::OK;
}

//! Function find free cell
//! \param lst ptr to List object
//! \return    free cell index (0 if list is full)
int find_free_cell(List* lst) {
    ASSERT_OK(lst, "Check before find_free_cell func", -1);

    int free_cell   = lst->first_free;
    lst->first_free = lst->data[lst->first_free].next;

    ASSERT_OK(lst, "Check before find_free_cell func", -1);
    return free_cell;
}

//! Function resize capacity of list
//! \param lst      ptr to List object
//! \param new_size new capacity value
//! \return         new capacity (0 if error in func)
int resize_list_capacity(List* lst, int new_size) {
    ASSERT_OK(lst, "Check before resize_list_capacity func", 0);
    ASSERT_IF(new_size > lst->capacity, "Incorrect new_size. Should be (> capacity)", 0);

    PRINT_WARNING("!WARNING! List is to small. List capacity has increased, but it`s to slow.\n"
                  "          Recreate List with bigger capacity to speed up list working.\n");

    ListElement* new_data = (ListElement*) realloc(lst->data, new_size * sizeof(ListElement));

    if (!VALID_PTR(new_data)) {
        ERROR_DUMP(lst, "Not enough memory", (List_t)UN);

        errno = errors::NOT_ENOUGH_MEMORY;
        return errors::NOT_ENOUGH_MEMORY;
    }

    lst->data = new_data;

    int capacity = lst->capacity;
    for (int i = capacity; i < new_size; i++) {
        lst->data[i] = {
            .value = (List_t)UN,
            .next  = i + 1,
            .prev  = UN
        };
    }

    lst->first_free = capacity;
    lst->capacity = new_size;
    lst->data[new_size - 1].next = 0;

    ASSERT_OK(lst, "Check after resize_list_capacity func", 0);
    return lst->capacity;
}

//! Function sorts list by logical indexes
//! \param lst ptr to List object
//! \return    1 if success, else 0
int please_dont_use_sorted_by_next_values_func_because_it_too_slow__also_do_you_really_need_it__i_think_no__so_dont_do_stupid_things_and_better_look_at_memes_about_cats(List* lst) {
    ASSERT_OK(lst, "Check before sorting func", 0);

    int capacity = lst->capacity;
    ListElement* sorted_list = (ListElement*) calloc(capacity, sizeof(ListElement));

    if (!VALID_PTR(sorted_list)) {
        ERROR_DUMP(lst, "Not enough memory", (List_t)UN);

        errno = errors::NOT_ENOUGH_MEMORY;
        return errors::NOT_ENOUGH_MEMORY;
    }

    // Fill zero index---------------------------------------------------------
    sorted_list[0].value = UN;
    sorted_list[0].next  = 1;
    sorted_list[0].prev  = 1;
    // ------------------------------------------------------------------------

    int head_tmp = lst->head;
    for (int i = 1; ; head_tmp = lst->data[head_tmp].next, i++) {
        sorted_list[i] = lst->data[head_tmp];
        sorted_list[i].next = i + 1;
        sorted_list[i].prev = i - 1;

        if (lst->data[head_tmp].next == 0) {
            for (int index_zero = i + 1; index_zero < capacity; index_zero++) {
                sorted_list[index_zero] = {
                    .value = (List_t)UN,
                    .next  = index_zero + 1,
                    .prev  = UN
                };
            }

            sorted_list[i].next = 0;
            sorted_list[capacity - 1].next = 0;
            lst->tail = i;
            lst->first_free = i + 1;
            break;
        };
    }

    free(lst->data);
    lst->data = sorted_list;

    lst->head = 1;
    lst->is_sorted = 1;

    ASSERT_OK(lst, "Check after sorting func", 0);
    return 1;
}

//! Function gets element by logical_index
//! \param lst       ptr to List object
//! \param log_index logical index
//! \return          element by logical index (poisons::UNINITIALIZED_INT if error in func)
List_t get(List* lst, int log_index) {
    ASSERT_OK(lst, "Check before get func", (List_t)UN);
    ASSERT_IF(0 <= log_index && log_index < lst->capacity - 1, "Incorrect logical index. Should be (> 0) and (< capacity)", (List_t)UN);

    if (lst->is_sorted) {
        LOG1(printf("Quick get\n"););
        List_t value = lst->data[(lst->head) + log_index].value;

        if (value == UN || value == FR) {
            ERROR_DUMP(lst, "List index out of range", (List_t)UN);

            errno = errors::BAD_LOG_INDEX;
            return errors::BAD_LOG_INDEX;
        }

        return value;
    }

    LOG1(printf("Long get\n"););
    int head_tmp = lst->head;
    for (int i = 0; i < log_index; head_tmp = lst->data[head_tmp].next, i++) {
        continue;
    }

    return lst->data[head_tmp].value;
}

//! Function inserts value after ph_index
//! \param lst      ptr to List object
//! \param value    inserted value
//! \param ph_index physical index of element, after which need to insert
//! \return         physical index of inserted element
int push_index(List* lst, List_t value, int ph_index) {
    ASSERT_OK(lst, "Check before push_index func", 0);
    ASSERT_IF(0 <= ph_index && ph_index < lst->capacity, "Incorrect ph_index. Index should be (>= 0) and (< capacity)", 0);

    if (lst->data[ph_index].prev == UN) {
        ERROR_DUMP(lst, "Push after invalid element. Incorrect physical index", 0);

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }

    // Find next_index where insert--------------------------------------------
    int next_index = find_free_cell(lst);

    if (next_index == 0) {
        resize_list_capacity(lst, lst->capacity * 2);

        next_index = find_free_cell(lst);
    }
    ASSERT_IF(0 <= next_index && next_index < lst->capacity, "Incorrect next index", 0);
    // ------------------------------------------------------------------------

    // Adding new element data-------------------------------------------------
    lst->data[next_index] = {
        .value = value,
        .next  = lst->data[ph_index].next,
        .prev  = ph_index
    };
    // ------------------------------------------------------------------------

    lst->data[lst->data[ph_index].next].prev = next_index;  // Changing prev value for element, before which inserted element
    lst->data[ph_index].next = next_index;                  // Changing next value for element, after which we insert

    // Updating head and tail index (if it need)-------------------------------
    if (ph_index == 0) {
        lst->head = next_index;
    }
    if (lst->tail == ph_index) {
        lst->tail = next_index;
    }
    lst->is_sorted = 0;
    // ------------------------------------------------------------------------

    ASSERT_OK(lst, "Check after push_index func", 0);
    return next_index;
}

//! Function pops element by ph_index
//! \param lst      ptr to List object
//! \param ph_index physical index of popped element
//! \return         popped value
List_t pop_index(List* lst, int ph_index) {
    ASSERT_OK(lst, "Check before pop_index func", (List_t)UN);
    ASSERT_IF(0 < ph_index && ph_index < lst->capacity, "Incorrect ph_index. Index should be (> 0) and (< capacity)", (List_t)UN);

    if (lst->head == lst->tail && lst->tail == 0) {
        ERROR_DUMP(lst, "Cannot pop from empty lst",(List_t)UN);

        errno = errors::LST_EMPTY;
        return errors::LST_EMPTY;
    }
    if (lst->data[ph_index].prev == UN) {
        ERROR_DUMP(lst, "Pop invalid element. Incorrect physical index",(List_t)UN);

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }

    List_t pop_val = lst->data[ph_index].value;
    int next_index = lst->data[ph_index].next;
    int prev_index = lst->data[ph_index].prev;

    // Updating head and tail index (if it need)-------------------------------
    if (ph_index == lst->head) {
        lst->head = next_index;
    }
    if (ph_index == lst->tail) {
        lst->tail = prev_index;
    }
    lst->is_sorted = 0;
    // ------------------------------------------------------------------------

    lst->data[prev_index].next = next_index;    // Changing next value for element, after which we delete
    lst->data[next_index].prev = prev_index;    // Changing prev value for element, before which deleted element

    // Deleting new element data-----------------------------------------------
    lst->data[ph_index] = {
        .value = (List_t)FR,
        .next  = lst->first_free,
        .prev  = UN
    };
    
    lst->first_free = ph_index;                 // Updating first_free index (making deleting index as first free)
    // ------------------------------------------------------------------------

    ASSERT_OK(lst, "Check after pop_index func", (List_t)UN);
    return pop_val;
}

//! Function inserts value after tail
//! \param lst   ptr to List object
//! \param value inserted value
//! \return      index of inserted element
int push_back(List* lst, List_t value) {
    ASSERT_OK(lst, "Check before push_back func", 0);

    return push_index(lst, value, lst->tail);
}

//! Function pops vaue by tail index
//! \param lst ptr to List object
//! \return    popped value
List_t pop_back(List* lst) {
    ASSERT_OK(lst, "Check before pop_back func", (List_t)UN);

    int tmp_sorted = lst->is_sorted;
    List_t pop_val = pop_index(lst, lst->tail);
    lst->is_sorted = tmp_sorted;

    return pop_val;
}

// Function inserts before head index
//! \param lst   ptr to List object
//! \param value inserted value
//! \return      index of inserted element
int push_front(List* lst, List_t value) {
    ASSERT_OK(lst, "Check before push_front func", 0);

    return push_index(lst, value, 0);
}

//! Function pops element by head index
//! \param lst ptr to List object
//! \return    popped value
List_t pop_front(List* lst) {
    ASSERT_OK(lst, "Check before pop_front func", (List_t)UN);

    int tmp_sorted = lst->is_sorted;
    List_t pop_val = pop_index(lst, lst->head);
    lst->is_sorted = tmp_sorted;

    return pop_val;
}

//! Function prints list for user
//! \param lst ptr to List object
//! \param sep ptr to sep string (default ", ")
//! \param end ptr to end string (default "\n")
//! \return    1 if success, else 0
int print_list(List* lst, const char* sep, const char* end) {
    ASSERT_OK(lst, "Check before print_list func", 0);
    ASSERT_IF(VALID_PTR(sep), "Invalid sep ptr", 0);
    ASSERT_IF(VALID_PTR(end), "Invalid end ptr", 0);

    if (lst->head == lst->tail) {
        printf("[  ]%s", end);
        return 1;
    }

    int head_tmp = lst->head;

    printf("[ ");
    for ( ; ; head_tmp = lst->data[head_tmp].next) {
        printf("%3d", lst->data[head_tmp].value);

        if (lst->data[head_tmp].next == 0) break;
        else printf("%s", sep);
    }
    printf(" ]%s", end);

    return 1;
}

//! Function dumps list info
//! \param lst    ptr to List object
//! \param reason ptr to reason string
//! \param log    ptr to log file (default stdout)
//! \param sep    ptr to sep string (default ", ")
//! \param end    ptr to end string (default "\n")
//! \return       1 if success, else 0
int list_dump(List* lst, const char* reason, FILE* log, const char* sep, const char* end) {
    ASSERT_IF(VALID_PTR(lst),    "Invalid lst ptr", 0);
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr", 0);
    
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);
    ASSERT_IF(VALID_PTR(sep),    "Invalid sep ptr", 0);
    ASSERT_IF(VALID_PTR(end),    "Invalid end ptr", 0);

    fprintf(log, COLORED_OUTPUT("|-------------------------          List  Dump          -------------------------|\n", ORANGE, log));
    FPRINT_DATE(log);
    fprintf(log, COLORED_OUTPUT("%s\n", BLUE, log), reason);
    int err = list_error(lst);
    int capacity = lst->capacity;

    fprintf(log, "    List state: %d ", err);
    if (err != 0) fprintf(log, COLORED_OUTPUT("(%s)\n\n", RED,   log), list_error_desc(err));
    else          fprintf(log, COLORED_OUTPUT("(%s)\n\n", GREEN, log), list_error_desc(err));

    fprintf(log, "    Is_sorted: %s %s\n"
                 "         Head: %d %s\n"
                 "         Tail: %d %s\n"
                 "     Capacity: %d %s\n\n",
            lst->is_sorted == 0 ? COLORED_OUTPUT("no", PURPLE, log) : lst->is_sorted == 1 ? COLORED_OUTPUT("yes", BLUE, log) : to_string(lst->is_sorted),
                            (0 > lst->is_sorted || lst->is_sorted > 1)    ? COLORED_OUTPUT("(BAD)", RED, log) : "",
            lst->head,      (0 > lst->head || lst->head >= capacity) ?      COLORED_OUTPUT("(BAD)", RED, log) : "",
            lst->tail,      (0 > lst->tail || lst->tail >= capacity) ?      COLORED_OUTPUT("(BAD)", RED, log) : "",
            capacity,              (capacity <= 0)        ?                 COLORED_OUTPUT("(BAD)", RED, log) : ""
    );

    fprintf(log, "             ");
    for (int i = 0; i < capacity; i++) {
        fprintf(log, "%3d  ", i);
    }
    fprintf(log, "\n");

    fprintf(log, "              ");
    for (int i = 0 ; i < capacity; i++) {
        if      (i == lst->head && i == lst->tail)  fprintf(log, COLORED_OUTPUT(" B ", PURPLE, log));
        else if (i == lst->head)                    fprintf(log, COLORED_OUTPUT(" H ", BLUE, log));
        else if (i == lst->tail)                    fprintf(log, COLORED_OUTPUT(" T ", GREEN, log));
        else                                        fprintf(log, "   ");

        fprintf(log, "  ");
    }
    fprintf(log, "\n");

    fprintf(log, "    Buffer: [ ");
    for (int i = 0; i < capacity; i++) {
        if      (lst->data[i].value == (List_t)UN)  fprintf(log, COLORED_OUTPUT(" un", CYAN, log));
        else if (lst->data[i].value == (List_t)FR)  fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                        fprintf(log, "%3d", lst->data[i].value);

        if (i + 1 < capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ]%s", end);

    fprintf(log, "    Next:   [ ");
    for (int i = 0; i < capacity; i++) {
        if      (lst->data[i].next == UN)   fprintf(log, COLORED_OUTPUT(" un", ORANGE, log));
        else if (lst->data[i].next == FR)   fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                fprintf(log, "%3d", lst->data[i].next);

        if (i + 1 < capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ] %s", end);

    fprintf(log, "    Prev:   [ ");
    for (int i = 0; i < capacity; i++) {
        if      (lst->data[i].prev == UN)   fprintf(log, COLORED_OUTPUT(" un", ORANGE, log));
        else if (lst->data[i].prev == FR)       fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                        fprintf(log, "%3d", lst->data[i].prev);

        if (i + 1 < capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ] %s\n", end);

    fprintf(log, "    First_free: %d %s\n", lst->first_free, lst->first_free >= 0 && lst->first_free < capacity ? "" : COLORED_OUTPUT("(BAD)", RED, log));
    
    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}

//! Function make graph dump of list info
//! \param lst    ptr to List object
//! \param reason ptr to reason string
//! \param log    ptr to log file
//! \param sep    ptr to sep string (default ", ")
//! \param end    ptr to end string (default "\n")
//! \return       1 if success, else 0
int list_dump_graph(List* lst, const char* reason, FILE* log, const char* sep, const char* end) {
    ASSERT_IF(VALID_PTR(lst),    "Invalid lst ptr", 0);
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr", 0);
    
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);
    ASSERT_IF(VALID_PTR(sep),    "Invalid sep ptr", 0);
    ASSERT_IF(VALID_PTR(end),    "Invalid end ptr", 0);

    FILE* dot_file = open_file("logs/dot_file.txt", "w");

    fputs("digraph structs {\n", dot_file);
    fputs("    rankdir=LR\n\n", dot_file);


    int capacity = lst->capacity;
    char* node_str = (char*) calloc(MAX_NODE_STR_SIZE, sizeof(char));
    sprintf(node_str, "    cell_head [ shape=component label=\"head | %d\" color=\"%s\" ]\n"
                      "    cell_tail [ shape=component label=\"tail | %d\" color=\"%s\" ]\n"
                      "    cell_capacity [ shape=component label=\"capacity | %d\" color=\"%s\" ]\n"
                      "    cell_head -> cell_tail -> cell_capacity[arrowhead=\"none\"]\n\n",
            lst->head, 0 < lst->head && lst->head < capacity ? "blue"  : "red",
            lst->tail, 0 < lst->tail && lst->tail < capacity ? "green" : "red",
            capacity, capacity > 0 ? "black" : "red"
    );
    fputs(node_str, dot_file);

    for (int i = 0; i < capacity; i++) {
        ListElement el = lst->data[i];
        node_str = (char*) calloc(MAX_NODE_STR_SIZE, sizeof(char));
        sprintf(node_str, "    cell_%d [ shape=record, label=< %d<br/><br/>"
                    " value =<font color=\"%s\">%s</font><br/>"
                    "  next =<font color=\"%s\">%s</font><br/>"
                    "  prev =<font color=\"%s\">%s</font>"
                    "> color = \"%s\" ]\n",
                i, i,
                el.value == (List_t)UN ? "blue" : el.value == (List_t)FR ? "red" : "black", el.value == (List_t)UN ? "un" : el.value == (List_t)FR ? "fr" : to_string(el.value),
                el.next  == UN ? "orange" : el.next == FR ? "red": "black", el.next == UN ? "un" : el.next == FR ? "fr" : to_string(el.next),
                el.prev  == UN ? "orange" : el.prev == FR ? "red": "black", el.prev == UN ? "un" : el.prev == FR ? "fr" : to_string(el.prev),
                i == lst->head && i == lst->tail ? "purple" : i == lst->head ? "blue" : i == lst->tail ? "green" : "black"
        );
        fputs(node_str, dot_file);

        if (i != 0) {
            if (el.next != UN && el.next != FR && el.next != 0) {
                sprintf(node_str, "    cell_%d -> cell_%d\n", i, el.next);
                fputs(node_str, dot_file);
            }
            if (el.prev != UN && el.prev != FR) {
                sprintf(node_str, "    cell_%d -> cell_%d\n", i, el.prev);
                fputs(node_str, dot_file);
            }
        }
        fputs("\n", dot_file);
    }

    int err = list_error(lst);
    sprintf(node_str, "    cell_free [ shape=component label=\"first free | %d\" color=\"%s\" ]\n\n"
                      "    cell_is_sorted [shape=component label=\"is_sorted | %s\" color=\"%s\" ]\n"
                      "    cell_state [ shape=component label=\"state | %d (%s)\" color=\"%s\" ]\n"
                      "    cell_state -> cell_is_sorted[arrowhead=\"none\"]\n",
            lst->first_free, 0 <= lst->first_free && lst->first_free < capacity ? "black" : "red",
            lst->is_sorted == 0 ? "no" : lst->is_sorted == 1 ? "yes" : to_string(lst->is_sorted), 0 <= lst->is_sorted && lst->is_sorted <= 1 ? "black" : "red",
            err, list_error_desc(err), err == 0 ? "green" : "red"
    );
    fputs(node_str, dot_file);

    fputs("}\n", dot_file);
    fclose(dot_file);

    system("dot -v -Tpng logs/dot_file.txt -o logs/graph.png");

    fputs("<h1 align=\"center\">Dump List</h1>\n<pre>\n", log);
    list_dump(lst, reason, log, sep, end);
    fputs("</pre>\n<img src=\"logs/graph.png\">\n\n", log);

    FREE_PTR(node_str, char);
    return 1;
}
