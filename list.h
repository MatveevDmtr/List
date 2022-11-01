#include <stdlib.h>

#define USER_MODE      1
#define HASH_MODE      2
#define HARDDEBUG_MODE 3

#define LIST_MODE USER_MODE

#define StructListInit(name)                              \
        StructureListInit(#name,                          \
                           __PRETTY_FUNCTION__,           \
                           __FILE__,                      \
                           __LINE__);


#define allocate_array(type, num_elems)                             \
({                                                                  \
    type* location__ = (type*) calloc (num_elems, sizeof (type));   \
    log ("Type_calloc: %s\n", #type);                               \
    log ("Sizeof type: %d\n", sizeof (type*));                      \
    Assert(location__ == NULL);                                     \
    log("Calloc returned: %p\n", location__);                       \
    location__;                                                     \
})

// START DUMP  DSL

#define dumpline(...)  fprintf(dump_file, __VA_ARGS__);
#define dumphtml(...)  fprintf(html_file, __VA_ARGS__);
// FINISH DUMP DSL
const char* DUMP_FILE_NAME = "dump.dot";
const char* PICNAME        = "graphCode";
const char* FILE_EXTENSION = ".png";
const char* HTML_FILE_NAME = "htmldump.html";


typedef unsigned long long bird_t;

typedef int elem_t;

typedef struct struct_elem
{
    elem_t elem;

    size_t next;
    size_t prev;
} elem_s;

typedef struct list_info
{
    elem_s*            Ptr;
    size_t             Size;
    size_t             Capacity;
    size_t             Free;

    bool               DeadInside;
} list_t;

const size_t MIN_LEN_LIST = 4;

const elem_t POISONED_ELEM = -13;

const size_t MAX_LEN_PICNAME = 15;

const size_t MAX_LEN_CONSOLE_CMD = 50;

const size_t MAX_LEN_NUM_DUMP = 10;

const int PREV_FOR_FREE = -1;

list_t StructureListInit(const char* name,
                           const char* func,
                           const char* file,
                           int line);

int ListCtor(list_t* lst);

int InsertBeforeElem(list_t* lst, size_t i_anc, elem_t elem);

int InsertAfterElem (list_t* lst, size_t i_anc, elem_t elem);

int ListVerify(list_t* lst);

elem_t DeleteElem(list_t* lst, size_t i_del);

void GraphListDump(const list_t* lst, const char* picname);

int HTMLDump(const list_t* lst, const char* occasion);

int LogCritError(int errcode, const char* func, int line);

int ListRecalloc(list_t* lst, size_t mode);
