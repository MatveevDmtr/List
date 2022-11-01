#include <stdio.h>
#include <math.h>
//#include <TXLib.h>
#define LOGGING


#include "list.h"
#include "D:\\Programming\\C\\Ded_course_1_sem\\Stack\\GetPoison.h"

#include "D:\\Programming\\C\\Ded_course_1_sem\\Processor_v4\\logging.h"

enum ERRCODES
{
     OK                 , // 0
     SEGFAULT           , // 1
     ZOMBIE             , // 2
     NULLPTR            , // 3
     SIZEPOISONED       , // 4
     NEGCAP             , // 5
     DEBUGINFOERROR     , // 6
     REALLOCERROR       , // 7
     STACKOVERFLOW      , // 8
     FREE_ERROR         , // 9
     CHAINERROR         , // 10
     CONSTR_ERROR         // 11
};

enum REALLOC_MODES
{
    SORTING = 1,
    FAST    = 2
};




int main()
{
    printf("startuem\n");

    list_t lst = StructListInit(lst);

    ListCtor(&lst);

    log("list created\n");

    InsertAfterElem (&lst, 0, 65);
    HTMLDump(&lst, "1 ins after");
    InsertBeforeElem(&lst, 1, 75);
    HTMLDump(&lst, "2 ins before");
    InsertAfterElem (&lst, 2, 85);
    HTMLDump(&lst, "3 ins after");

    DeleteElem(&lst, 1);
    ListPop(&lst);
    ListAppend(&lst, 228);

    HTMLDump(&lst, "After delete");
    //HTMLDump(&lst, "EXAMPLE");

}

list_t StructureListInit(const char* name,
                           const char* func,
                           const char* file,
                           int line)
{
    void*              Ptr             = NULL;
    size_t             Size            = 0;
    size_t             Capacity        = 0;
    size_t             Free            = 0;

    list_t temp_list =
    {
        .Ptr               = (elem_s*) getPoison(Ptr),
        .Size              = getPoison(Size),
        .Capacity          = getPoison(Capacity),
        .Free              = getPoison(Free),
        .DeadInside        = 1
    };

    return temp_list;
}

int ListCtor(list_t* lst)
{
    static size_t num_dump = 0;

    if (lst == NULL || lst == getPoison(lst))
    {
        LogError(SEGFAULT);

        return SEGFAULT;
    }

    if (!(lst->DeadInside))
    {
        LogError(CONSTR_ERROR);

        return CONSTR_ERROR;
    }

    char* temp_ptr = (char*) calloc(MIN_LEN_LIST, sizeof(elem_s));

    if (temp_ptr == NULL)
    {
        LogError(REALLOCERROR);

        return REALLOCERROR;
    }

    lst->Ptr                = (elem_s*)(temp_ptr);

    lst->DeadInside         = 0;
    lst->Size               = 0;
    lst->Capacity           = MIN_LEN_LIST;
    lst->Free               = 1;

    for (size_t i = 0; i < lst->Capacity; i++)
    {
        lst->Ptr[i] = {.elem = POISONED_ELEM,
                       .next = i + 1,
                       .prev = PREV_FOR_FREE};
    }

    lst->Ptr[0].next = 0;
    lst->Ptr[0].prev = 0;

    ListVerify(lst);

    return 0;
}

int ListAppend(list_t* lst, elem_t elem)
{
    InsertAfterElem(lst, lst->Ptr[0].prev, elem);

    return 0;
}

elem_t ListPop(list_t* lst)
{
    return DeleteElem(lst, lst->Ptr[0].prev);
}

int InsertBeforeElem(list_t* lst, size_t i_anc, elem_t elem)
{
    log("start insert before\n");

    InsertAfterElem(lst, lst->Ptr[i_anc].prev, elem);

    return 0;
}

int InsertAfterElem(list_t* lst, size_t i_anc, elem_t elem)
{
    ListVerify(lst);

    log("start insert\n");

    size_t new_next = lst->Ptr[i_anc].next;
    size_t i_ins    = lst->Free;

    lst->Free = lst->Ptr[i_ins].next;

    lst->Ptr[i_ins].elem = elem;

    lst->Ptr[i_ins].next = new_next;
    lst->Ptr[i_ins].prev = i_anc;

    lst->Ptr[i_anc   ].next = i_ins;
    lst->Ptr[new_next].prev = i_ins;

    lst->Size++;

    if (lst->Free >= lst->Capacity - 1)
    {
        log("Recalloc necessary\n");

        HTMLDump(lst, " Before Recalloc");

        ListRecalloc(lst, SORTING);

        HTMLDump(lst, " After Recalloc");
    }

    ListVerify(lst);

    return 0;
}

elem_t DeleteElem(list_t* lst, size_t i_del)
{
    log("start delete\n");

    ListVerify(lst);

    elem_t del_elem = lst->Ptr[i_del].elem;
    size_t del_next = lst->Ptr[i_del].next;
    size_t del_prev = lst->Ptr[i_del].prev;
    size_t old_free = lst->Free;

    lst->Free = i_del;

    lst->Ptr[i_del].elem = POISONED_ELEM;
    lst->Ptr[i_del].next = old_free;
    lst->Ptr[i_del].prev = PREV_FOR_FREE;

    lst->Ptr[del_prev].next = del_next;
    lst->Ptr[del_next].prev = del_prev;

    lst->Size--;   //make PushFree

    ListVerify(lst);

    return del_elem;
}

int ListVerify(list_t* lst)
{
    if (lst == NULL)
    {
        LogError(SEGFAULT);
        return SEGFAULT;
    }
    if (lst->DeadInside == 1)
    {
        LogError(ZOMBIE);
        return ZOMBIE;
    }
    if (lst->Ptr == NULL)
    {
        LogError(NULLPTR);
        return NULLPTR;
    }
    if (lst->Size < 0)                  LogError(SIZEPOISONED);
    if (lst->Capacity < MIN_LEN_LIST)   LogError(NEGCAP);
    if (lst->Capacity < lst->Size)      LogError(STACKOVERFLOW);
    if (lst->Free > lst->Capacity - 1)  LogError(FREE_ERROR);

    size_t i_next = 0;

    for (size_t i = 0; i < lst->Capacity - 1; i++)
    {
        i_next = lst->Ptr[i].next;

        if (lst->Ptr[i].prev != PREV_FOR_FREE)
        {
            if (lst->Ptr[i_next].prev != i)
            {
                LogError(CHAINERROR);

                log("Elements with phys addresses %d and %d have invalid connection\n", i, i_next);
            }
        }
        else
        {
            if (lst->Ptr[i_next].prev != PREV_FOR_FREE)
            {
                LogError(CHAINERROR);

                log("Free element with phys address %d points not at a free element %d\n", i, i_next);
            }
        }
    }

    return 0;
}

int LogCritError(int errcode, const char* func, int line)
{
    switch (errcode)
    {
        case OK:
            //print_log(FRAMED, "Everything OK: Stack is all right and can be used");
            break;

        case SEGFAULT:
            print_crit_errors("SEGMENTATION FAULT: Invalid Pointer to Structure of Stack", func, line);
            break;

        case ZOMBIE:
            print_crit_errors("DEADINSIDE ERROR: Stack doesn't exist", func, line);
            break;

        case NULLPTR:
            print_crit_errors("POINTER ERROR: Stack Pointer (pointer to buffer) is NULL", func, line);
            break;

        case SIZEPOISONED:
            print_crit_errors("SIZE ERROR: Stack Size is poisoned", func, line);
            break;

        case NEGCAP:
            print_crit_errors("CAPACITY ERROR: Stack Capacity has a Negative Value", func, line);
            break;

        case STACKOVERFLOW:
            print_crit_errors("STACK OVERFLOW ERROR: Size of Stack is bigger than its Capacity", func, line);
            break;

        case FREE_ERROR:
            print_crit_errors("FREE ERROR: Index of free unit is invalid", func, line);
            break;

        case CHAINERROR:
            print_crit_errors("CHAIN ERROR: Invalid Next-Prev Connection between elements", func, line);
            break;

        case CONSTR_ERROR:
            print_crit_errors("CONSTRUCTION ERROR: Trying to construct an existing List", func, line);
            break;

        default:
            print_crit_errors("DECODE ERROR: Unexpected Error Code", func, line);
            return 1;
    }

    return 0;
}

int HTMLDump(const list_t* lst, const char* occasion)
{
    Assert(lst == NULL);

    log("lst pointer in HTMLDump: %p\n", lst);

    static size_t num_dump;  //init in ListCtor

    char curr_picname[MAX_LEN_PICNAME] = "";

    char str_num_dump[MAX_LEN_NUM_DUMP] = "";

    itoa(num_dump, str_num_dump, 10);

    strcpy(curr_picname, PICNAME);
    strcat(curr_picname, str_num_dump);
    strcat(curr_picname, FILE_EXTENSION);

    log("picture name: %s\n", curr_picname);
    //picname made

    GraphListDump(lst, curr_picname);

    char html_piccmd[MAX_LEN_CONSOLE_CMD] = "<img src=\"";

    strcat(html_piccmd, curr_picname);
    strcat(html_piccmd, "\" alt=\"ERROR: Picture Not Found\">");

    char* r_mode = "a";
    if (num_dump == 0)          r_mode = "w";

    FILE* html_file = fopen(HTML_FILE_NAME, r_mode);

    if (html_file == NULL)
    {
        print_log(FRAMED, "FILE ERROR: Cant't open HTML file\n");
    }

    dumphtml("<pre>\n");
    dumphtml("<b>\n");
    dumphtml("<font size=6>");
    dumphtml("List dump (");
    dumphtml("%d", num_dump);
    dumphtml(")\n");
    dumphtml("<i>");
    dumphtml("<font size=4>");
    dumphtml("\nOcassion for DUMP: %s\n\n", occasion);
    dumphtml("%s\n\n", html_piccmd);
    dumphtml("<\pre>\n");

    num_dump++;

    fclose(html_file);
}

int ListRecalloc(list_t* lst, size_t mode)
{
    switch(mode)
    {
        case SORTING:
        {
            elem_s* temp_ptr = allocate_array(elem_s, lst->Capacity * 2);

            if (temp_ptr == NULL)
            {
                print_log(FRAMED, "CALLOC ERROR: Can't find memory for the list");

                return -1;
            }
            *temp_ptr = *(lst->Ptr);

            lst->Capacity *= 2;

            temp_ptr[0] = lst->Ptr[0];

            temp_ptr[0].next = 1;
            temp_ptr[0].prev = lst->Size;

            size_t i_next = lst->Ptr[0].next;

            for (size_t i = 0; i < lst->Size; i++)
            {
                log("i_next = %d\n", i_next);

                temp_ptr[i + 1] = lst->Ptr[i_next];

                temp_ptr[i + 1].next = i + 2;

                temp_ptr[i + 1].prev = i;

                i_next = lst->Ptr[i_next].next;
            }

            lst->Ptr = temp_ptr;

            lst->Ptr[lst->Size].next = 0;
            lst->Ptr[lst->Size].prev = lst->Size - 1;

            HTMLDump(lst, "after copy");

            for (size_t i = lst->Size + 1; i < lst->Capacity; i++)
            {
                lst->Ptr[i].next = i + 1;

                lst->Ptr[i].prev = -1;

                lst->Ptr[i].elem = POISONED_ELEM;
            }

            lst->Free = lst->Size + 1;

            break;
        }
        case 2:
        {
            elem_s* temp_ptr = (elem_s*)realloc(lst->Ptr, lst->Capacity * 2);
            if (temp_ptr == NULL)
            {
                print_log(FRAMED, "RECALLOC ERROR: Can't find memory for the list");

                return -1;
            }

            lst->Ptr = temp_ptr;
            break;
        }
    }

    return 0;
}

void GraphListDump(const list_t* lst, const char* picname)
{
    log("lst pointer: %p\n", lst);

    FILE* dump_file = fopen(DUMP_FILE_NAME, "w");

    if (dump_file == NULL)
    {
        print_log(FRAMED, "FILE ERROR: Cant't open file\n");
    }
    dumpline("digraph {\n");
    dumpline("graph [dpi = 100]");
    dumpline("rankdir = LR;\n");
    dumpline("splines = \"ortho\";\n");
    dumpline("node [ shape=record ];\n");

    dumpline("LIST [label = \"LIST|size: %d|Capacity: %d\", style = \"filled\", fillcolor = \"indigo\", fontcolor = \"yellow\"];\n", lst->Size, lst->Capacity);
    dumpline("LIST -> struct0 [style=\"invis\" weight = 1000];\n");
    for (int i = 0; i < lst->Capacity; i++)
    {
        dumpline("index%d[label = \"index: %d\", style = \"filled\" fillcolor = \"coral\"];\n", i, i);

        if (i > 0)
            dumpline("index%d -> index%d [style = \"invis\" weight = 200];\n", i - 1, i);
    }

    dumpline("INDEX [style = \"filled\", fillcolor = \"lightslateblue\"]\n");
    dumpline("INDEX -> index0[style = \"invis\" weight = 900]\n");

    for (int i = 0; i < lst->Capacity; i++)
    {
        int elem = lst->Ptr[i].elem;
        int next = lst->Ptr[i].next;
        int prev = lst->Ptr[i].prev;

        char* color = NULL;

        if (i == 0)
        {
            dumpline("struct%d [\nlabel = \"<data>elem: nil|<next>next: %d|<prev>prev: %d\", style = \"filled\", fillcolor = \"cyan\" \n];\n", i, next, prev);
            dumpline("struct%d -> struct%d [weight = 0] [ color=red  ]\n", i, lst->Ptr[i].next);
            continue;
        }
        if (prev == PREV_FOR_FREE)
            color = "lightgrey";
        else
            color = "aquamarine";


        dumpline("struct%d [\nlabel = \"<data>elem: %d|<next>next: %d|<prev>prev: %d\", style = \"filled\", color = \"black\", fillcolor = \"%s\" \n];\n", i, elem, next, prev, color);
        dumpline("struct%d -> struct%d [dir=none weight=900 style=\"invis\" constraint=true];\n", i - 1, i);

        if (i < lst->Capacity - 1)
        {
            dumpline("struct%d -> struct%d [weight = 0, constraint=false, color=red  ]\n", i, lst->Ptr[i].next);
        }
    }

    /*for (int i = 1; i < lst->Capacity; i++)
    {
        //dumpline("struct%d:<prev> -> struct%d:<next> [weight = 2] [color=green]\n", i, lst->Ptr[i].prev);
    }*/

    dumpline("structFree [\nlabel = \"free: %d\", style = \"filled\", fillcolor = \"green\"]\n", lst->Free);
    dumpline("structFree -> struct%d [weight = 0] [color = darkgoldenrod2]\n", lst->Free);

    dumpline("}");

    fclose(dump_file);

    static size_t num_dump;  //init in ListCtor

    log("start making console cmd\n");

    char console_cmd[MAX_LEN_CONSOLE_CMD] = "";

    strcpy(console_cmd, "dot -T png -o ");
    strcat(console_cmd, picname);
    strcat(console_cmd, " ");
    strcat(console_cmd, DUMP_FILE_NAME);

    log("console cmd: %s\n", console_cmd);

    printf("%d\n", system("cd D:"));
    printf("%d\n", system("cd D:\\Programming\\C\\Ded_course_1_sem\\List"));
    printf("%d\n", system(console_cmd));
}
