#include "rsm_find_func_addr.h"

DEF_SCHED_SHOW_TASK DEF_SCHED_SHOW_TASK_TMP = NULL;
DEF_PUT_TASK_STRUCT DEF_PUT_TASK_STRUCT_TMP =  NULL;


struct rsm_sym_addr rsm_sym_addr_table[SYSCHK_MAX_SYM_UNFOUND] = {
    {"sched_show_task", (unsigned long *)&DEF_SCHED_SHOW_TASK_TMP},
    {"__put_task_struct", (unsigned long *)&DEF_PUT_TASK_STRUCT_TMP},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
};


/*
*  kallsyms_lookup_name_init  -  get the address of kallsyms_lookup_name
*/

/*
*  rsm_sym_addr_init  -  find the address of function 
*/
int rsm_sym_addr_init(void)
{
    unsigned long addr;
    int i = 0;
    while(rsm_sym_addr_table[i].name && SYSCHK_MAX_SYM_UNFOUND > i){
        addr = kallsyms_lookup_name(rsm_sym_addr_table[i].name);
        if(addr == 0){
            printk("Syschk error: Can't find the address of symbol %s\n", rsm_sym_addr_table[i].name);
            return -1;
        }
        *(rsm_sym_addr_table[i].addr) = addr;
        i++;
    }

    return 0;
}
