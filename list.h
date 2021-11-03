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

int list_ctor(List* lst);
int list_dtor(List* lst);

void print_list(List* lst);
void  dump_list(List* lst);

int push_back(List* lst, int value);
int  pop_back(List* lst);

int push_after(List* lst, int value, int ph_index);
int  pop_after(List* lst, int ph_index);
