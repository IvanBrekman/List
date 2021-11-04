//
//  Created by IvanBrekman on 03.11.2021.
//

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cerrno>

#include "config.h"
#include "libs/baselib.h"
#include "libs/file_funcs.h"

#include "list.h"

int list_ctor(List* lst) {
    assert(VALID_PTR(lst) && "Invalid lst ptr");
    
    lst->head = lst->tail = 0;
    lst->capacity = BUFFER_DEFAULT_SIZE;

    lst->buffer = (int*) calloc(lst->capacity, sizeof(int));
    lst->next   = (int*) calloc(lst->capacity, sizeof(int));

    for (int i = 0; i < lst->capacity; i++) {
        lst->buffer[i] = poisons::UNINITIALIZED_INT;
        lst->next[i]   = poisons::UNINITIALIZED_INT;
    }

    ASSERT_OK(lst, "Check corectness of list_ctor");
    return 0;
}

int list_dtor(List* lst) {
    ASSERT_OK(lst, "Check List before dtor call");

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        for (int i = 0; i < lst->capacity; i++) {
            lst->buffer[i] = poisons::FREED_ELEMENT;
            lst->next[i]   = poisons::FREED_ELEMENT;
        }
    }

    lst->capacity = lst->head = lst->tail = -1;

    FREE_PTR(lst->buffer, int);
    FREE_PTR(lst->next, int);

    return 0;
}

const char* list_error_desc(int error_code) {
    switch (error_code)
    {
        case errors::OK:
            return "ok";
        case errors::INVALID_LIST_PTR:
            return "Invalid list pointer";
        case errors::INCORRECT_HEAD_INDEX:
            return "Incorrect head index: index (< 0) or (>= BUFFER_SIZE)";
        case errors::INCORRECT_TAIL_INDEX:
            return "Incorrect tail index: index (< 0) or (>= BUFFER_SIZE)";
        case errors::INCORRECT_CAPACITY:
            return "Incorrect capacity: capacity (<= 0)";
        case errors::LST_EMPTY:
            return "List is empty";
        case errors::BAD_PH_INDEX:
            return "Funvtion received bad physical index (next index in lst->next[ph_index] is UNINITIALIZED";
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
    if (0 > lst->head || lst->head >= lst->capacity) {
        return errors::INCORRECT_HEAD_INDEX;
    }
    if (0 > lst->tail || lst->tail >= lst->capacity) {
        return errors::INCORRECT_TAIL_INDEX;
    }

    return errors::OK;
}

int find_free_cell(List* lst, int start_index) {
    ASSERT_OK(lst, "Check before find_free_cell func");
    assert(0 <= start_index && start_index < lst->capacity && "Incorrect start_index");

    for (int i = (start_index + 1) % lst->capacity, count = 0; count < lst->capacity; i++ % lst->capacity, count++) {
        if (lst->next[i] == poisons::UNINITIALIZED_INT && i != 0) {
            return i;
        }
    }

    return -1;
}

int resize_list_capacity(List* lst, int new_size) {
    ASSERT_OK(lst, "Check before resize_list_capacity func");
    assert(new_size > 0 && "Incorrect new_size (<= 0)");

    int* new_buf  = (int*) realloc(lst->buffer, new_size * sizeof(int));
    int* new_next = (int*) realloc(lst->next,   new_size * sizeof(int));

    if (!VALID_PTR(new_buf) || !VALID_PTR(new_next)) {
        ERROR_DUMP(lst, "Not enough memory");

        errno = errors::NOT_ENOUGH_MEMORY;
        return errors::NOT_ENOUGH_MEMORY;
    }

    lst->buffer   = new_buf;
    lst->next     = new_next;

    for (int i = lst->capacity; i < new_size; i++) {
        lst->buffer[i] = poisons::UNINITIALIZED_INT;
        lst->next[i]   = poisons::UNINITIALIZED_INT;
    }

    lst->capacity = new_size;

    ASSERT_OK(lst, "Check after resize_list_capacity func");
    return lst->capacity;
}

int   push_back(List* lst, int value) {
    ASSERT_OK(lst, "Check before push_back func");

    if (lst->head == lst->tail && lst->tail == 0) {
        lst->buffer[1] = value;
        lst->next[1]   = 0;

        lst->head = lst->tail = 1;

        return 1;
    }

    int next_index = find_free_cell(lst, lst->tail);

    if (next_index == -1) {
        lst->capacity = resize_list_capacity(lst, lst->capacity * 2);

        next_index = find_free_cell(lst, lst->tail);

        PRINT_WARNING("!WARNING! List is to small. List capacity has increased, but it`s to slow.\n"
                      "          Recreate List with bigger capacity to speed up list working.\n");
    }
    assert(0 <= next_index && next_index < lst->capacity && "Incorrect next index");

    lst->buffer[next_index] = value;
    lst->next[next_index]   = 0;

    lst->next[lst->tail] = next_index;
    lst->tail = next_index;

    ASSERT_OK(lst, "Check after push_back func");
    return next_index;
}

int    pop_back(List* lst) {
    ASSERT_OK(lst, "Check before pop_back func");
    
    if (lst->next[lst->head] == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Cannot pop from empty lst");

        errno = errors::LST_EMPTY;
        return errors::LST_EMPTY;
    }
    int pop_val = lst->buffer[lst->head];

    if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
        lst->buffer[lst->head] = poisons::FREED_ELEMENT;
    }
    
    if (lst->head == lst->tail) {
        lst->next[lst->head] = poisons::UNINITIALIZED_INT;
        lst->head = lst->tail = 1;
        return pop_val;
    }

    int tmp_head = lst->head;
    lst->head = lst->next[lst->head];
    lst->next[tmp_head] = poisons::UNINITIALIZED_INT;
    
    ASSERT_OK(lst, "Check after pop_back func");
    return pop_val;
}

int  push_after(List* lst, int value, int ph_index) {
    ASSERT_OK(lst, "Check before push_after func");
    assert(0 <= ph_index && ph_index < lst->capacity && "Incorrect ph_index");

    if (lst->tail == ph_index) {
        return push_back(lst, value);
    }

    if (lst->next[ph_index] == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Push after invalid element. Incorrect physical index");

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }

    int next_index = find_free_cell(lst, ph_index);

    if (next_index == -1) {
        lst->capacity = resize_list_capacity(lst, lst->capacity * 2);

        next_index = find_free_cell(lst, ph_index);

        PRINT_WARNING("!WARNING! List is to small. List capacity has increased, but it`s to slow.\n"
                      "          Recreate List with bigger capacity to speed up list working.\n");
    }
    assert(0 <= next_index && next_index < lst->capacity && "Incorrect next index");

    lst->buffer[next_index] = value;
    lst->next[next_index] = lst->next[ph_index];
    lst->next[ph_index] = next_index;

    ASSERT_OK(lst, "Check after push_after func");
    return next_index;
}

int   pop_after(List* lst, int ph_index) {
    ASSERT_OK(lst, "Check before pop_after func");
    assert(0 <= ph_index && ph_index < lst->capacity && "Incorrect ph_index");

    if (lst->next[lst->head] == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Cannot pop from empty lst");

        errno = errors::LST_EMPTY;
        return errors::LST_EMPTY;
    }
    if (lst->next[ph_index] == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Pop after invalid element. Incorrect physical index");

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }
    if (lst->tail == ph_index) {
        ERROR_DUMP(lst, "Pop next after last element. Incorrect physical index");

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }

    int next_index = lst->next[ph_index];
    int pop_val = lst->buffer[next_index];

    if (next_index == lst->tail) {
        lst->tail = ph_index;
    }

    lst->next[ph_index] = lst->next[next_index];
    lst->next[next_index] = poisons::UNINITIALIZED_INT;

    ASSERT_OK(lst, "Check after pop_after func");
    return pop_val;
}

void print_list(List* lst, const char* sep, const char* end) {
    ASSERT_OK(lst, "Check before print_list func");
    assert(VALID_PTR(sep)    && "Invalid sep ptr");
    assert(VALID_PTR(end)    && "Invalid end ptr");

    if (lst->next[lst->head] == poisons::UNINITIALIZED_INT) {
        printf("[  ]%s", end);
        return;
    }

    int head_tmp = lst->head;

    printf("[ ");
    for ( ; ; head_tmp = lst->next[head_tmp]) {
        printf("%3d", lst->buffer[head_tmp]);

        if (lst->next[head_tmp] == 0) break;
        else printf("%s", sep);
    }
    printf(" ]%s", end);
}

void  list_dump(List* lst, const char* reason, FILE* log, const char* sep, const char* end) {
    assert(VALID_PTR(lst)    && "Invalid lst ptr");
    assert(VALID_PTR(log)    && "Invalid log ptr");
    
    assert(VALID_PTR(reason) && "Invalid reason ptr");
    assert(VALID_PTR(sep)    && "Invalid sep ptr");
    assert(VALID_PTR(end)    && "Invalid end ptr");

    fprintf(log, COLORED_OUTPUT("|-------------------------          List  Dump          -------------------------|\n", ORANGE, log));
    FPRINT_DATE(log);
    fprintf(log, COLORED_OUTPUT(" %s\n\n", BLUE, log), reason);
    int err = list_error(lst);

    fprintf(log, "    List state: %d ", err);
    if (err != 0) fprintf(log, COLORED_OUTPUT("(%s)\n", RED,   log), list_error_desc(err));
    else          fprintf(log, COLORED_OUTPUT("(%s)\n", GREEN, log), list_error_desc(err));

    fprintf(log, "        Head: %d %s\n"
                 "        Tail: %d %s\n"
                 "    Capacity: %d %s\n",
            lst->head, 0 > lst->head || lst->head >= lst->capacity ? COLORED_OUTPUT("(BAD)", RED, log) : "",
            lst->tail, 0 > lst->tail || lst->tail >= lst->capacity ? COLORED_OUTPUT("(BAD)", RED, log) : "",
            lst->capacity,          (lst->capacity <= 0)      ?      COLORED_OUTPUT("(BAD)", RED, log) : "");
    

    fprintf(log, "             ");
    for (int i = 0; i < lst->capacity; i++) {
        fprintf(log, "%3d  ", i);
    }
    fprintf(log, "\n");

    fprintf(log, "              ");
    for (int i = 0 ; i < lst->capacity; i++) {
        if      (i == lst->head && i == lst->tail) fprintf(log, COLORED_OUTPUT(" B ", PURPLE, log));
        else if (i == lst->head) fprintf(log, COLORED_OUTPUT(" H ", RED, log));
        else if (i == lst->tail) fprintf(log, COLORED_OUTPUT(" T ", GREEN, log));
        else                     fprintf(log, "   ");

        fprintf(log, "  ");
    }
    fprintf(log, "\n");

    fprintf(log, "    Buffer: [ ");
    for (int i = 0; i < lst->capacity; i++) {
        if      (lst->buffer[i] == poisons::UNINITIALIZED_INT) fprintf(log, COLORED_OUTPUT(" un", CYAN, log));
        else if (lst->buffer[i] == poisons::FREED_ELEMENT)     fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                   fprintf(log, "%3d", lst->buffer[i]);

        if (i + 1 < lst->capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ]%s", end);

    fprintf(log, "    Next:   [ ");
    for (int i = 0; i < lst->capacity; i++) {
        if      (lst->next[i] == poisons::UNINITIALIZED_INT) fprintf(log, COLORED_OUTPUT(" un", ORANGE, log));
        else if (lst->next[i] == poisons::FREED_ELEMENT)     fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                 fprintf(log, "%3d", lst->next[i]);

        if (i + 1 < lst->capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ] %s\n", end);
    
    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");
}
