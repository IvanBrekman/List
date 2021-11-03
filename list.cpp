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

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            lst->buffer[i] = poisons::UNINITIALIZED_INT;
            lst->next[i]   = poisons::UNINITIALIZED_INT;
        }
    }
    
    lst->head      = lst->tail    = 0;

    ASSERT_OK(lst, "Check corectness of list_ctor");
    return 0;
}
int list_dtor(List* lst) {
    ASSERT_OK(lst, "Check List before dtor call");

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            lst->buffer[i] = poisons::FREED_ELEMENT;
            lst->next[i]   = poisons::FREED_ELEMENT;
        }
    }

    lst->head = lst->tail = -1;

    return 0;
}

const char* list_error_desc(int error_code) {
    switch (error_code)
    {
        case list_errors::OK:
            return "ok";
        case list_errors::INVALID_LIST_PTR:
            return "Invalid list pointer";
        case list_errors::INCORRECT_HEAD_INDEX:
            return "Incorrect head index: index (< 0) or (>= BUFFER_SIZE)";
        case list_errors::INCORRECT_TAIL_INDEX:
            return "Incorrect tail index: index (< 0) or (>= BUFFER_SIZE)";
        case list_errors::LST_EMPTY:
            return "List is empty";
        case list_errors::LST_FULL:
            return "List is full";
        case list_errors::BAD_PH_INDEX:
            return "Funvtion received bad physical index (next index in lst->next[ph_index] is UNINITIALIZED";
        
        default:
            return "Unknown error";
    }
}
int         list_error(List* lst) {
    if (!VALID_PTR(lst)) {
        return list_errors::INVALID_LIST_PTR;
    }

    if (0 > lst->head || lst->head >= BUFFER_SIZE) {
        return list_errors::INCORRECT_HEAD_INDEX;
    }
    if (0 > lst->tail || lst->tail >= BUFFER_SIZE) {
        return list_errors::INCORRECT_TAIL_INDEX;
    }

    return list_errors::OK;
}

int find_free_cell(List* lst, int start_index) {
    ASSERT_OK(lst, "Check before find_free_cell func");
    assert(0 <= start_index && start_index < BUFFER_SIZE && "Incorrect start_index");

    for (int i = (start_index + 1) % BUFFER_SIZE, count = 0; count < BUFFER_SIZE; i++ % BUFFER_SIZE, count++) {
        if (lst->next[i] == poisons::UNINITIALIZED_INT && i != 0) {
            return i;
        }
    }

    return -1;
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
        ERROR_DUMP(lst, "Cannot push to full list");
        return list_errors::LST_FULL;
    }

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
        return list_errors::LST_EMPTY;
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
    assert(0 <= ph_index && ph_index < BUFFER_SIZE && "Incorrect ph_index");

    if (lst->tail == ph_index) {
        return push_back(lst, value);
    }

    if (lst->next[ph_index] == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Push after invalid element. Incorrect physical index");
        return list_errors::BAD_PH_INDEX;
    }

    int next_index = find_free_cell(lst, ph_index);

    if (next_index == -1) {
        ERROR_DUMP(lst, "Cannot push to full list");
        return list_errors::LST_FULL;
    }

    lst->buffer[next_index] = value;
    lst->next[next_index] = lst->next[ph_index];
    lst->next[ph_index] = next_index;

    ASSERT_OK(lst, "Check after push_after func");
    return next_index;
}
int   pop_after(List* lst, int ph_index) {
    ASSERT_OK(lst, "Check before pop_after func");
    assert(0 <= ph_index && ph_index < BUFFER_SIZE && "Incorrect ph_index");

    if (lst->next[lst->head] == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Cannot pop from empty lst");
        return list_errors::LST_EMPTY;
    }
    if (lst->next[ph_index] == poisons::UNINITIALIZED_INT) {
        ERROR_DUMP(lst, "Pop after invalid element. Incorrect physical index");
        return list_errors::BAD_PH_INDEX;
    }
    if (lst->tail == ph_index) {
        ERROR_DUMP(lst, "Pop next after last element. Incorrect physical index")
        return list_errors::BAD_PH_INDEX;
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

    fprintf(log, "    Head: %d %s\n"
                 "    Tail: %d %s\n",
            lst->head, 0 > lst->head || lst->head >= BUFFER_SIZE ? COLORED_OUTPUT("(BAD)", RED, log) : "",
            lst->tail, 0 > lst->tail || lst->tail >= BUFFER_SIZE ? COLORED_OUTPUT("(BAD)", RED, log) : "");
    

    fprintf(log, "             ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fprintf(log, "%3d  ", i);
    }
    fprintf(log, "\n");

    fprintf(log, "              ");
    for (int i = 0 ; i < BUFFER_SIZE; i++) {
        if      (i == lst->head && i == lst->tail) fprintf(log, COLORED_OUTPUT(" B ", PURPLE, log));
        else if (i == lst->head) fprintf(log, COLORED_OUTPUT(" H ", RED, log));
        else if (i == lst->tail) fprintf(log, COLORED_OUTPUT(" T ", GREEN, log));
        else                     fprintf(log, "   ");

        fprintf(log, "  ");
    }
    fprintf(log, "\n");

    fprintf(log, "    Buffer: [ ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if      (lst->buffer[i] == poisons::UNINITIALIZED_INT) fprintf(log, COLORED_OUTPUT(" un", CYAN, log));
        else if (lst->buffer[i] == poisons::FREED_ELEMENT)     fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                   fprintf(log, "%3d", lst->buffer[i]);

        if (i + 1 < BUFFER_SIZE) fprintf(log, "%s", sep);
    }
    fprintf(log, " ]%s", end);

    fprintf(log, "    Next:   [ ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if      (lst->next[i] == poisons::UNINITIALIZED_INT) fprintf(log, COLORED_OUTPUT(" un", ORANGE, log));
        else if (lst->next[i] == poisons::FREED_ELEMENT)     fprintf(log, COLORED_OUTPUT(" fr", RED, log));
        else                                                 fprintf(log, "%3d", lst->next[i]);

        if (i + 1 < BUFFER_SIZE) fprintf(log, "%s", sep);
    }
    fprintf(log, " ] %s\n", end);
    
    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");
}
