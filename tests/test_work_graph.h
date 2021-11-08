//
//  Created by IvanBrekman on 08.11.2021.
//

#ifndef LIST_TESTH
#define LIST_TESTH

#include "../config.h"

#include <stdio.h>

#include "../list.h"
#include "../libs/baselib.h"
#include "../libs/file_funcs.h"

int test_work_graph() {
    List lst = { };
    list_ctor(&lst);

    for (int i = 0; i < 5; i++) {
        push_back(&lst, (i + 1) * 10);
        LOG_DUMP_GRAPH(&lst, "Add new value", list_dump_graph);

        printf("press n to continue...\n");
        char ans = '\0';
        while (scanf(" %c", &ans) != 1 || ans != 'n'){
            printf("press n to continue...\n");
            while (getchar() != '\n') continue;
        }
        printf("next...\n");
    }

    return 1;
}

#endif // LIST_TESTH
