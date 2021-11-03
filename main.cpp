//
//  Created by IvanBrekman on 03.11.2021.
//

#include <cstdio>

#include "libs/baselib.h"

#include "list.h"

int main(int argc, char** argv) {
    List lst = { };

    list_ctor(&lst);
    list_dump(&lst, "Check dump");

#if 0
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
#endif

#if 1
    int ph_index = push_back(&lst, 10);
    
    for (int i = 0; i < 5; i++) {
        push_after(&lst, (i + 2) * 10, ph_index);
        list_dump(&lst, "Check adding");
        print_list(&lst);
    }

    for (int i = 0; i < 3; i++) {
        pop_after(&lst, ph_index);
        list_dump(&lst, "Check deleting");
        print_list(&lst);
    }

    pop_after(&lst, 3);
    list_dump(&lst, "Check deleting");
    print_list(&lst);

    pop_after(&lst, 1);
    list_dump(&lst, "Check deleting");
    print_list(&lst);

    pop_after(&lst, 1);
    list_dump(&lst, "Check deleting");
    print_list(&lst);
#endif

    list_dtor(&lst);

    return 0;
}
