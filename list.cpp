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

    lst->data = (ListElement*) calloc(lst->capacity, sizeof(ListElement));

    for (int i = 0; i < lst->capacity; i++) {
        lst->data[i].value = (List_t)poisons::UNINITIALIZED_INT;
        lst->data[i].next  = poisons::UNINITIALIZED_INT;
    }

    ASSERT_OK(lst, "Check corectness of list_ctor");
    return 0;
}

int list_dtor(List* lst) {
    ASSERT_OK(lst, "Check List before dtor call");

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        for (int i = 0; i < lst->capacity; i++) {
            lst->data[i].value = (List_t)poisons::FREED_ELEMENT;
            lst->data[i].next  = poisons::FREED_ELEMENT;
        }
    }

    lst->capacity = lst->head = lst->tail = -1;

    FREE_PTR(lst->data, ListElement);
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
        if (lst->data[i].next == poisons::UNINITIALIZED_INT && i != 0) {
            return i;
        }
    }

    return -1;
}

int resize_list_capacity(List* lst, int new_size) {
    ASSERT_OK(lst, "Check before resize_list_capacity func");
    assert(new_size > 0 && "Incorrect new_size (<= 0)");

    ListElement* new_data = (ListElement*) realloc(lst->data, new_size * sizeof(ListElement));

    if (!VALID_PTR(new_data)) {
        ERROR_DUMP(lst, "Not enough memory");

        errno = errors::NOT_ENOUGH_MEMORY;
        return errors::NOT_ENOUGH_MEMORY;
    }

    lst->data = new_data;

    for (int i = lst->capacity; i < new_size; i++) {
        lst->data[i].value = (List_t)poisons::UNINITIALIZED_INT;
        lst->data[i].next  = poisons::UNINITIALIZED_INT;
    }

    lst->capacity = new_size;

    ASSERT_OK(lst, "Check after resize_list_capacity func");
    return lst->capacity;
}

int push_back(List* lst, List_t value) {
    ASSERT_OK(lst, "Check before push_back func");

    if (lst->head == lst->tail && lst->tail == 0) {
        lst->data[1].value = value;
        lst->data[1].next  = 0;

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

    lst->data[next_index].value = value;
    lst->data[next_index].next  = 0;

    lst->data[lst->tail].next = next_index;
    lst->tail = next_index;

    ASSERT_OK(lst, "Check after push_back func");
    return next_index;
}

List_t pop_back(List* lst) {
    ASSERT_OK(lst, "Check before pop_back func");
    
    if (lst->data[lst->head].next == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Cannot pop from empty lst");

        errno = errors::LST_EMPTY;
        return errors::LST_EMPTY;
    }
    List_t pop_val = lst->data[lst->head].value;

    if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
        lst->data[lst->head].value = (List_t)poisons::FREED_ELEMENT;
    }
    
    if (lst->head == lst->tail) {
        lst->data[lst->head].next = poisons::UNINITIALIZED_INT;
        lst->head = lst->tail = 1;
        return pop_val;
    }

    int tmp_head = lst->head;
    lst->head = lst->data[lst->head].next;
    lst->data[tmp_head].next = poisons::UNINITIALIZED_INT;
    
    ASSERT_OK(lst, "Check after pop_back func");
    return pop_val;
}

int push_after(List* lst, List_t value, int ph_index) {
    ASSERT_OK(lst, "Check before push_after func");
    assert(0 <= ph_index && ph_index < lst->capacity && "Incorrect ph_index");

    if (lst->tail == ph_index) {
        return push_back(lst, value);
    }

    if (lst->data[ph_index].next == poisons::UNINITIALIZED_INT) {
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

    lst->data[next_index].value = value;
    lst->data[next_index].next  = lst->data[ph_index].next;
    lst->data[ph_index].next = next_index;

    ASSERT_OK(lst, "Check after push_after func");
    return next_index;
}

int pop_after(List* lst, int ph_index) {
    ASSERT_OK(lst, "Check before pop_after func");
    assert(0 <= ph_index && ph_index < lst->capacity && "Incorrect ph_index");

    if (lst->data[lst->head].next == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Cannot pop from empty lst");

        errno = errors::LST_EMPTY;
        return errors::LST_EMPTY;
    }
    if (lst->data[ph_index].next == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Pop after invalid element. Incorrect physical index");

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }
    if (lst->tail == ph_index) {
        ERROR_DUMP(lst, "Pop next after last element. Incorrect physical index");

        errno = errors::BAD_PH_INDEX;
        return errors::BAD_PH_INDEX;
    }

    int next_index = lst->data[ph_index].next;
    List_t pop_val = lst->data[next_index].value;

    if (next_index == lst->tail) {
        lst->tail = ph_index;
    }

    lst->data[ph_index].next = lst->data[next_index].next;
    lst->data[next_index].next = poisons::UNINITIALIZED_INT;

    ASSERT_OK(lst, "Check after pop_after func");
    return pop_val;
}

void print_list(List* lst, const char* sep, const char* end) {
    ASSERT_OK(lst, "Check before print_list func");
    assert(VALID_PTR(sep)    && "Invalid sep ptr");
    assert(VALID_PTR(end)    && "Invalid end ptr");

    if (lst->data[lst->head].next == poisons::UNINITIALIZED_INT) {
        printf("[  ]%s", end);
        return;
    }

    int head_tmp = lst->head;

    printf("[ ");
    for ( ; ; head_tmp = lst->data[head_tmp].next) {
        printf("%3d", lst->data[head_tmp].value);

        if (lst->data[head_tmp].next == 0) break;
        else printf("%s", sep);
    }
    printf(" ]%s", end);
}

void list_dump(List* lst, const char* reason, FILE* log, const char* sep, const char* end) {
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
        if      (i == lst->head && i == lst->tail)  fprintf(log, COLORED_OUTPUT(" B ", PURPLE, log));
        else if (i == lst->head)                    fprintf(log, COLORED_OUTPUT(" H ", RED, log));
        else if (i == lst->tail)                    fprintf(log, COLORED_OUTPUT(" T ", GREEN, log));
        else                                        fprintf(log, "   ");

        fprintf(log, "  ");
    }
    fprintf(log, "\n");

    fprintf(log, "    Buffer: [ ");
    for (int i = 0; i < lst->capacity; i++) {
        if      (lst->data[i].value == (List_t)poisons::UNINITIALIZED_INT)  fprintf(log, COLORED_OUTPUT(" un", CYAN, log));
        else if (lst->data[i].value == (List_t)poisons::FREED_ELEMENT)      fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                                fprintf(log, "%3d", lst->data[i].value);

        if (i + 1 < lst->capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ]%s", end);

    fprintf(log, "    Next:   [ ");
    for (int i = 0; i < lst->capacity; i++) {
        if      (lst->data[i].next == poisons::UNINITIALIZED_INT)   fprintf(log, COLORED_OUTPUT(" un", ORANGE, log));
        else if (lst->data[i].next == poisons::FREED_ELEMENT)       fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                        fprintf(log, "%3d", lst->data[i].next);

        if (i + 1 < lst->capacity) fprintf(log, "%s", sep);
    }
    fprintf(log, " ] %s\n", end);
    
    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");
}
