#ifndef __RSM_FIND_FUNC_ADDR_H
#define __RSM_FIND_FUNC_ADDR_H

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>

#define UNUSED(x)       ( (void)(x) )
#define MIN(x, y)       ( (x) < (y)? (x) : (y) )
#define MAX(x, y)       ( (x) > (y)? (x) : (y) )

/* MAX num of function */
#define SYSCHK_MAX_SYM_UNFOUND (16)

/*define  name and address struct */
typedef struct  rsm_sym_addr{
    char * name;
    unsigned long * addr;
}RSM_SYM_ADDR;

/* define function pointer */
typedef void (*DEF_SCHED_SHOW_TASK)(struct task_struct *p);
typedef void (*DEF_PUT_TASK_STRUCT)(struct task_struct *tsk);

extern DEF_SCHED_SHOW_TASK DEF_SCHED_SHOW_TASK_TMP;
extern DEF_PUT_TASK_STRUCT DEF_PUT_TASK_STRUCT_TMP;


/* define samename function */
#define sched_show_task_macro(p) DEF_SCHED_SHOW_TASK_TMP(p)
#define __put_task_struct_macro(tsk) DEF_PUT_TASK_STRUCT_TMP(tsk)


#endif /*__SYSCHK_FIND_FUNC_ADDR_H*/
