//
//  Created by IvanBrekman on 03.11.2021.
//

#include <cstdio>
#include <cerrno>

#include "libs/baselib.h"
#include "libs/file_funcs.h"

#include "list.h"

int main(void) {
    List lst = { };

    errno = 0;
    list_ctor(&lst);
    printf("errno: %d\n", errno);
    list_dump(&lst, "Check dump");

#if 1
    print_list(&lst);
    for (int i = 0; i < 5; i++) {
        push_back(&lst, (i + 1) * 10);
        list_dump(&lst, "Check adding");
        print_list(&lst);
    }
    pop_back(&lst);
    list_dump(&lst, "Check deleting");
    print_list(&lst);

    for (int i = 0; i < 5; i++) {
        push_back(&lst, (i + 1) * 10);
        list_dump(&lst, "Check adding");
        print_list(&lst);
    }

    for (int i = 0; i < 9; i++) {
        pop_back(&lst);
        list_dump(&lst, "Check deleting");
        print_list(&lst);
    }

    for (int i = 0; i < 10; i++) {
        push_back(&lst, (i + 1) * 10);
        list_dump(&lst, "Check adding");
        print_list(&lst);
    }

    for (int i = 0; i < 5; i++) {
        pop_front(&lst);
        list_dump(&lst, "Check deleting front");
        print_list(&lst);
    }

    please_dont_use_sorted_by_next_values_func_because_it_too_slow__also_do_you_really_need_it__i_think_no__so_dont_do_stupid_things_and_better_look_at_memes_about_cats(&lst);
    list_dump(&lst, "Check sorting");
    print_list(&lst);

    pop_front(&lst);
    list_dump(&lst, "Check deleting front");
    print_list(&lst);

    printf("Number: %d\n", get(&lst, 2));
    push_index(&lst, -13, 2);
    list_dump(&lst, "Check adding");
    print_list(&lst);

    please_dont_use_sorted_by_next_values_func_because_it_too_slow__also_do_you_really_need_it__i_think_no__so_dont_do_stupid_things_and_better_look_at_memes_about_cats(&lst);
    list_dump(&lst, "Check sorting");
    print_list(&lst);

    FILE* graph_log = open_file("log.html", "w");
    list_dump_graph(&lst, "Check dtor", graph_log);
    fclose(graph_log);

#endif

#if 0
    int ph_index = push_index(&lst, 10, 0);
    list_dump(&lst, "Check adding");
    print_list(&lst);

    push_index(&lst, 20, 1);
    list_dump(&lst, "Check adding");
    print_list(&lst);

    push_index(&lst, 30, ph_index);
    list_dump(&lst, "Check adding");
    print_list(&lst);

    please_dont_use_sorted_by_next_values_func_because_it_too_slow__also_do_you_really_need_it__i_think_no__so_dont_do_stupid_things_and_better_look_at_memes_about_cats(&lst);
    list_dump(&lst, "Check sorting");
    print_list(&lst);

    pop_front(&lst);
    list_dump(&lst, "Check adding");
    print_list(&lst);

    printf("Number with index (%d): %d\n", 0, get(&lst, 0));

    list_dump_graph(&lst, stdout);
#endif

    list_dtor(&lst);
    list_dump(&lst, "check dtor");

    return 0;
}
