#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "hung_task.h"

#define RSM_SET_ADDR_ERR    (-2)
#define RSM_GET_ADDR_ERR    (-3)
#define RSM_INIT_OK         0


int rsm_sym_addr_init(void);


static int hung_task_ok = 0;
int rsm_init_module (void)
{

    /* get the the address of symbol */
    if(0 != rsm_sym_addr_init()){
        return RSM_GET_ADDR_ERR;
    }
    
    hung_task_init();	
    hung_task_ok = 1;	
    	
    return RSM_INIT_OK;
}

void rsm_cleanup_module(void)
{
    if(hung_task_ok == 1)
    {
	    hung_task_exit();
        hung_task_ok = 0;
    }
}



MODULE_LICENSE("GPL");
module_init(rsm_init_module);
module_exit(rsm_cleanup_module);
