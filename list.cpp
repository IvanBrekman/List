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

int list_ctor(List* lst, int capacity) {
    ASSERT_IF(VALID_PTR(lst), "Invalid lst ptr", 0);
    ASSERT_IF(capacity > 0,   "Incorrect capacity: (<= 0)", 0);
    
    lst->head = lst->tail = 0;
    lst->is_sorted = 1;
    lst->capacity = capacity;

    lst->data = (ListElement*) calloc(capacity, sizeof(ListElement));

    for (int i = 0; i < capacity; i++) {
        // TODO function or directly assign?
        // fill_list_element(&lst->data[i], (List_t)poisons::UNINITIALIZED_INT, i + 1, poisons::UNINITIALIZED_INT);
        lst->data[i].value = (List_t)poisons::UNINITIALIZED_INT;
        lst->data[i].next  = i + 1;
        lst->data[i].prev  = poisons::UNINITIALIZED_INT;
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

int list_dtor(List* lst) {
    ASSERT_OK(lst, "Check List before dtor call", 0);

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        int capacity = lst->capacity;
        for (int i = 0; i < capacity; i++) {
            lst->data[i].value = (List_t)poisons::FREED_ELEMENT;
            lst->data[i].next  = poisons::FREED_ELEMENT;
            lst->data[i].prev  = poisons::FREED_ELEMENT;
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

int find_free_cell(List* lst, int start_index) {
    ASSERT_OK(lst, "Check before find_free_cell func", -1);
    ASSERT_IF(0 <= start_index && start_index < lst->capacity, "Incorrect start_index", -1);

    int free_cell = lst->first_free;
    lst->first_free = lst->data[lst->first_free].next;

    ASSERT_OK(lst, "Check before find_free_cell func", -1);
    return free_cell;
}

int resize_list_capacity(List* lst, int new_size) {
    ASSERT_OK(lst, "Check before resize_list_capacity func", 0);
    ASSERT_IF(new_size > lst->capacity, "Incorrect new_size. Should be (> capacity)", 0);

    PRINT_WARNING("!WARNING! List is to small. List capacity has increased, but it`s to slow.\n"
                  "          Recreate List with bigger capacity to speed up list working.\n");

    ListElement* new_data = (ListElement*) realloc(lst->data, new_size * sizeof(ListElement));

    if (!VALID_PTR(new_data)) {
        ERROR_DUMP(lst, "Not enough memory");

        errno = errors::NOT_ENOUGH_MEMORY;
        return errors::NOT_ENOUGH_MEMORY;
    }

    lst->data = new_data;

    int capacity = lst->capacity;
    for (int i = capacity; i < new_size; i++) {
        lst->data[i].value = (List_t)poisons::UNINITIALIZED_INT;
        lst->data[i].next  = i + 1;
        lst->data[i].prev  = poisons::UNINITIALIZED_INT;
    }

    lst->first_free = capacity;
    lst->capacity = new_size;

    ASSERT_OK(lst, "Check after resize_list_capacity func", 0);
    return lst->capacity;
}

int please_dont_use_sorted_by_next_values_func_because_it_too_slow__also_do_you_really_need_it__i_think_no__so_dont_do_stupid_things_and_better_look_at_memes_about_cats(List* lst) {
    ASSERT_OK(lst, "Check before sorting func", 0);

    int capacity = lst->capacity;
    ListElement* sorted_list = (ListElement*) calloc(capacity, sizeof(ListElement));

    if (!VALID_PTR(sorted_list)) {
        ERROR_DUMP(lst, "Not enough memory");

        errno = errors::NOT_ENOUGH_MEMORY;
        return errors::NOT_ENOUGH_MEMORY;
    }

    sorted_list[0].value = poisons::UNINITIALIZED_INT;
    sorted_list[0].next  = 1;
    sorted_list[0].prev  = 1;

    int head_tmp = lst->head;
    for (int i = 1; ; head_tmp = lst->data[head_tmp].next, i++) {
        sorted_list[i] = lst->data[head_tmp];
        sorted_list[i].next = i + 1;
        sorted_list[i].prev = i - 1;

        if (lst->data[head_tmp].next == 0) {
            for (int index_zero = i + 1; index_zero < capacity; index_zero++) {
                sorted_list[index_zero].value = poisons::UNINITIALIZED_INT;
                sorted_list[index_zero].next  = index_zero + 1;
                sorted_list[index_zero].prev  = poisons::UNINITIALIZED_INT;
            }

            sorted_list[i].next = 0;
            sorted_list[capacity - 1].next = 0;
            lst->tail = i;
            lst->first_free = i + 1;
            break;
        };
    }

    // !TODO free data?
    lst->data = sorted_list;
    lst->head = 1;
    lst->is_sorted = 1;

    ASSERT_OK(lst, "Check after sorting func", 0);
    return 1;
}

List_t get(List* lst, int log_index) {
    ASSERT_OK(lst, "Check before get func", (List_t)poisons::UNINITIALIZED_INT);
    ASSERT_IF(0 <= log_index && log_index < lst->capacity - 1, "Incorrect logical index. Should be (> 0) and (< capacity)", (List_t)poisons::UNINITIALIZED_INT);

    if (lst->is_sorted) {
        LOG1(printf("Quick get\n"););
        List_t value = lst->data[(lst->head) + log_index].value;

        if (value == poisons::UNINITIALIZED_INT || value == poisons::FREED_ELEMENT) {
            ERROR_DUMP(lst, "List index out of range");

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

void fill_list_element(ListElement* el_ptr, List_t value, int next, int prev) {
    el_ptr->value = value;
    el_ptr->next  = next;
    el_ptr->prev  = prev;
}

int push_index(List* lst, List_t value, int ph_index) {
    ASSERT_OK(lst, "Check before push_index func", 0);
    ASSERT_IF(0 <= ph_index && ph_index < lst->capacity, "Incorrect ph_index. Index should be (>= 0) and (< capacity)", 0);

    if (ph_index == 0 && (lst->head != 0 || lst->tail != 0)) {
        ERROR_DUMP(lst, "Push to 0 address while there are another elements in list. Incorrect physical index");

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }
    if (lst->data[ph_index].prev == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Push after invalid element. Incorrect physical index");

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }

    // Find next_index where insert--------------------------------------------
    int next_index = find_free_cell(lst, ph_index);

    if (next_index == 0) {
        lst->capacity = resize_list_capacity(lst, lst->capacity * 2);

        next_index = find_free_cell(lst, ph_index);
    }
    ASSERT_IF(0 <= next_index && next_index < lst->capacity, "Incorrect next index", 0);
    // ------------------------------------------------------------------------

    // Adding new element data-------------------------------------------------
    lst->data[next_index].value = value;
    lst->data[next_index].next  = lst->data[ph_index].next;
    lst->data[next_index].prev  = ph_index;
    // ------------------------------------------------------------------------

    lst->data[lst->data[ph_index].next].prev = next_index;  // Changing prev value for element, before which inserted element
    lst->data[ph_index].next = next_index;                  // Changing next value for element, after which we insert

    // Updating head and tail index (if it need)-------------------------------
    if (lst->head == lst->tail && lst->tail == 0) {
        lst->head = 1;
    }
    if (lst->tail == ph_index) {
        lst->tail = next_index;
    }
    lst->is_sorted = 0;
    // ------------------------------------------------------------------------

    ASSERT_OK(lst, "Check after push_index func", 0);
    return next_index;
}

List_t pop_index(List* lst, int ph_index) {
    ASSERT_OK(lst, "Check before pop_index func", (List_t)poisons::UNINITIALIZED_INT);
    ASSERT_IF(0 < ph_index && ph_index < lst->capacity, "Incorrect ph_index. Index should be (> 0) and (< capacity)", (List_t)poisons::UNINITIALIZED_INT);

    if (lst->head == lst->tail && lst->tail == 0) {
        ERROR_DUMP(lst, "Cannot pop from empty lst");

        errno = errors::LST_EMPTY;
        return errors::LST_EMPTY;
    }
    if (lst->data[ph_index].prev == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Pop invalid element. Incorrect physical index");

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
    lst->data[ph_index].value = poisons::FREED_ELEMENT;
    lst->data[ph_index].next  = lst->first_free;
    lst->data[ph_index].prev  = poisons::UNINITIALIZED_INT;

    lst->first_free = ph_index;                 // Updating first_free index (making deleting index as first free)
    // ------------------------------------------------------------------------

    ASSERT_OK(lst, "Check after pop_index func", (List_t)poisons::UNINITIALIZED_INT);
    return pop_val;
}

int push_back(List* lst, List_t value) {
    ASSERT_OK(lst, "Check before push_back func", 0);

    return push_index(lst, value, lst->tail);
}

List_t pop_back(List* lst) {
    ASSERT_OK(lst, "Check before pop_back func", (List_t)poisons::UNINITIALIZED_INT);

    List_t pop_val = pop_index(lst, lst->tail);
    lst->is_sorted = 1;

    return pop_val;
}

int push_front(List* lst, List_t value) {
    ASSERT_OK(lst, "Check before push_front func", 0);

    // TODO how write push front logic?
    return push_index(lst, value, lst->head);
}

List_t pop_front(List* lst) {
    ASSERT_OK(lst, "Check before pop_front func", (List_t)poisons::UNINITIALIZED_INT);

    List_t pop_val = pop_index(lst, lst->head);
    lst->is_sorted = 1;

    return pop_val;
}

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
    for ( int i = 0; i++ < 10; head_tmp = lst->data[head_tmp].next) {
        printf("%3d", lst->data[head_tmp].value);

        if (lst->data[head_tmp].next == 0) break;
        else printf("%s", sep);
    }
    printf(" ]%s", end);

    return 1;
}

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
        else if (i == lst->head)                    fprintf(log, COLORED_OUTPUT(" H ", RED, log));
        else if (i == lst->tail)                    fprintf(log, COLORED_OUTPUT(" T ", GREEN, log));
        else                                        fprintf(log, "   ");

        fprintf(log, "  ");
    }
    fprintf(log, "\n");

    fprintf(log, "    Buffer: [ ");
    for (int i = 0; i < capacity; i++) {
        if      (lst->data[i].value == (List_t)poisons::UNINITIALIZED_INT)  fprintf(log, COLORED_OUTPUT(" un", CYAN, log));
        else if (lst->data[i].value == (List_t)poisons::FREED_ELEMENT)      fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                                fprintf(log, "%3d", lst->data[i].value);

        if (i + 1 < capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ]%s", end);

    fprintf(log, "    Next:   [ ");
    for (int i = 0; i < capacity; i++) {
        if      (lst->data[i].next == poisons::UNINITIALIZED_INT)   fprintf(log, COLORED_OUTPUT(" un", ORANGE, log));
        else if (lst->data[i].next == poisons::FREED_ELEMENT)       fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                        fprintf(log, "%3d", lst->data[i].next);

        if (i + 1 < capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ] %s", end);

    fprintf(log, "    Prev:   [ ");
    for (int i = 0; i < capacity; i++) {
        if      (lst->data[i].prev == poisons::UNINITIALIZED_INT)   fprintf(log, COLORED_OUTPUT(" un", ORANGE, log));
        else if (lst->data[i].prev == poisons::FREED_ELEMENT)       fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                        fprintf(log, "%3d", lst->data[i].prev);

        if (i + 1 < capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ] %s\n", end);

    fprintf(log, "    First_free: %d %s\n", lst->first_free, lst->first_free >= -1 && lst->first_free < capacity ? "" : COLORED_OUTPUT("(BAD)", RED, log));
    
    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}
