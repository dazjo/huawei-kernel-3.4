

#include"phudiagfwd.h"
#include"phudiagchar.h"
struct phudiag_data

{
	int length;
	uint8_t data[0];
};



struct phudiag_ring_buf * phudiag_ring_buf_malloc(int buf_size)

{
	struct phudiag_ring_buf * ring_buf;

	ring_buf =  kzalloc(sizeof(struct phudiag_ring_buf) + buf_size, GFP_KERNEL);
	if(ring_buf)
	{
		ring_buf->buf_size = buf_size;
		ring_buf->start = ring_buf->end = ring_buf->buf;
		ring_buf->edge = ring_buf->buf + buf_size;
		#ifdef PHUDIAG_DEBUG
		printk("phudiag_ring_buf_malloc  success! malloc mem size = 0x%x\n",buf_size);
		#endif
	}
	else
	{
		printk("phudiag_ring_buf_malloc  fail! malloc mem size = 0x%x\n",buf_size);
	}
	return ring_buf;
}



void phudiag_ring_buf_free(struct phudiag_ring_buf *ring_buf)

{
	if(ring_buf)
	{
		kfree(ring_buf);
	}
}

void phudiag_ring_buf_clear(struct phudiag_ring_buf *ring_buf)
{
	if(ring_buf)
	{
		ring_buf->start = ring_buf->end = ring_buf->buf;
		ring_buf->edge = ring_buf->buf + ring_buf->buf_size;
	}
}

int phudiagfwd_ring_buf_is_empty(struct phudiag_ring_buf *ring_buf)
{
	if(!ring_buf)
	{
		printk(KERN_INFO "phudiagfwd_ring_buf_is_empty ring_buf== NULL  \n");
		return -1;
	}
	
	return !(ring_buf->start - ring_buf->end);
}




int phudiagfwd_ring_buf_set_data_before_process(struct phudiag_ring_buf *ring_buf ,int data_length)
{
	uint8_t* buf_end;
	
	if(0 == ring_buf || data_length <= 0)
	{
		printk(KERN_INFO "phudiagfwd_ring_buf_set_data_before_process  parameter invalid! \n");
		return -1;
	}
	
	buf_end = ring_buf->buf + ring_buf->buf_size;
	if(ring_buf->start <= ring_buf->end)
	{
		if(ring_buf->start == ring_buf->end && ring_buf->start != ring_buf->buf)
		{
			printk(KERN_INFO "phudiagfwd_ring_buf_set_data_before_process  fatal error! \n");
			return -1;
		}
		
		if(buf_end - ring_buf->end < data_length)
		{
			if(ring_buf->start - ring_buf->buf <= data_length)
			{
				return 0;	
			}
			ring_buf->edge = ring_buf->end;
			ring_buf->end = ring_buf->buf;		
		}
	}
	else
	{
		if(ring_buf->start - ring_buf->end <= data_length)
		{
			return 0;
		}
	}
	
	return data_length;
}



int phudiagfwd_ring_buf_get_data_before_process(struct phudiag_ring_buf *ring_buf,int buf_size)
{
	int data_length = 0;
	
	if(0 == ring_buf || buf_size <= 0)
	{
		printk(KERN_INFO "phudiagfwd_ring_buf_get_data_after_process  parameter invalid! \n");
		return -1;	
	}
	
	if(ring_buf->start <= ring_buf->end)
	{
		if(ring_buf->start == ring_buf->end)
		{
			if(ring_buf->start == ring_buf->buf)
			{
				return 0;
			}
			else
			{
				printk(KERN_INFO "phudiagfwd_ring_buf_get_data_before_process  fatal error: start == buf! \n");
				return -1;
			}
		}
		
		data_length = ring_buf->end - ring_buf->start;
		if(data_length > buf_size)
		{
			data_length = buf_size;
		}	
	}
	else
	{
		data_length = ring_buf->edge - ring_buf->start;
		if(data_length <= 0)
		{
			printk(KERN_INFO "phudiagfwd_ring_buf_get_data_before_process  fatal error: edge <= start! \n");
			return -1;
		}
		
		if(data_length > buf_size)
		{
			data_length = buf_size;
		}
	}

	return data_length;

}




int phudiagfwd_ring_buf_get_data_after_process(struct phudiag_ring_buf *ring_buf,int read_length)
{
	if(0 == ring_buf || read_length <= 0)
	{
		printk(KERN_INFO "phudiagfwd_ring_buf_get_data_after_process  parameter invalid! \n");
		return -1;	
	}

	ring_buf->start += read_length;
	if(ring_buf->start == ring_buf->end)
	{
		ring_buf->start = ring_buf->end = ring_buf->buf;
		ring_buf->edge =  ring_buf->buf + ring_buf->buf_size;
	}
	else if(ring_buf->start == ring_buf->edge)
	{
		ring_buf->start = ring_buf->buf;
		ring_buf->edge = ring_buf->buf + ring_buf->buf_size;
	}
	
	return 0;
}


int phudiagfwd_user_get_data(char __user *buf, int buf_size)
{

	int count = 0;


	mutex_lock(&phudriver->diagchar_mutex);
	count = phudiagfwd_ring_buf_get_data_before_process(phudriver->in_buf,buf_size);
	if(count > 0)
	{
		if(copy_to_user(buf,(void*)phudriver->in_buf->start,count))
		{
			printk(KERN_INFO "phudiagfwd_ring_buf_user_get_data copy_to_user failed !\n");
			mutex_unlock(&phudriver->diagchar_mutex);
			return  -1;
		}
		phudiagfwd_ring_buf_get_data_after_process(phudriver->in_buf, count);
	}

	mutex_unlock(&phudriver->diagchar_mutex);

	return count;

}



void phudiagfwd_read_data_from_smd(void)
{
	int r = smd_read_avail(phudriver->ch);

	
	if (r > PHU_MAX_BUF_SIZE)
	{
			printk(KERN_ALERT "\n diag: SMD sending in "
			"packets more than %d bytes", PHU_MAX_BUF_SIZE);
			return;
	}

	if(r > 0)
	{
		
		if(r == phudiagfwd_ring_buf_set_data_before_process(phudriver->in_buf, r))
		{
			smd_read(phudriver->ch, phudriver->in_buf->end, r);
			phudriver->in_buf->end += r;
		}
		else
		{
			printk(KERN_INFO "phudiagfwd_read_data_from_smd write out of memory !\n");
		}
	}

}


int phudiagfwd_user_set_data(const char __user * data,int data_length)
{
	struct phudiag_data *pdata;
	int total_data_length = 0;
	int ret = 0;

	if(NULL == phudriver->out_buf || NULL == data ||data_length <= 0 )
	{
		printk(KERN_INFO "phudiagfwd_copy_from_user_data  fatal error: parameter invalid! \n");
		return -1;
	}

	mutex_lock(&phudriver->diagchar_mutex);
	
	total_data_length = data_length +  sizeof(int);
	ret = phudiagfwd_ring_buf_set_data_before_process(phudriver->out_buf,total_data_length);
	if(ret <= 0)
	{
		printk(KERN_INFO "phudiagfwd_copy_from_user_data  fatal error! \n");
		mutex_unlock(&phudriver->diagchar_mutex);
		return ret;	
	}

	
	pdata = (struct phudiag_data *)phudriver->out_buf->end;
	pdata->length = data_length;
	if(copy_from_user(pdata->data,data,data_length))
	{
		printk(KERN_INFO "phudiagfwd_user_set_data copy_from_user failed!\n");
		mutex_unlock(&phudriver->diagchar_mutex);
		return -1;
	}
	phudriver->out_buf->end  += total_data_length;
	mutex_unlock(&phudriver->diagchar_mutex);

	return data_length;
}


void phudiagfwd_write_to_smd_work_fn(struct work_struct *work)

{
	int data_length = 0;
	int count = 0;

	mutex_lock(&phudriver->diagchar_mutex);
	
	if(0 == phudiagfwd_ring_buf_is_empty(phudriver->out_buf))
	{	
		count = phudiagfwd_ring_buf_get_data_before_process(phudriver->out_buf,sizeof(int));
		if(count != sizeof(int))
		{
			printk(KERN_INFO "phudiagfwd_write_to_smd_work_fn  fatal error: count != sizeof(int) !");
			mutex_unlock(&phudriver->diagchar_mutex);
			return;
		}
		
		memcpy(&data_length,(void*)phudriver->out_buf->start,count);
		phudiagfwd_ring_buf_get_data_after_process(phudriver->out_buf, count);
		
		count = phudiagfwd_ring_buf_get_data_before_process(phudriver->out_buf,data_length);
		if(count != data_length)
		{
			printk(KERN_INFO "phudiagfwd_write_to_smd_work_fn  fatal error: count != data_length !");
			mutex_unlock(&phudriver->diagchar_mutex);
			return;
		}

		if(count > PHU_DIAG_SMD_BUF_MAX)
		{
			printk(KERN_INFO "phudiagfwd_write_to_smd_work_fn  fatal error: count > PHU_DIAG_SMD_BUF_MAX !");
			mutex_unlock(&phudriver->diagchar_mutex);
			return;
		}
		
		memcpy(phudriver->smd_buf,(void*)phudriver->out_buf->start,count);
		phudiagfwd_ring_buf_get_data_after_process(phudriver->out_buf, count);
	
		if (phudriver->ch && data_length > 0)
		{
			smd_write(phudriver->ch, phudriver->smd_buf,data_length);
			//#ifdef PHUDIAG_DEBUG
			{
				int i = 0;
				printk(KERN_INFO "phudiagfwd_write_to_smd_work_fn  data_length = %4d ,data:",data_length);

				for(i=0;i< data_length;i++)
				{
					printk(" %x",phudriver->smd_buf[i]);
				}
				printk(" \n");
			}
			//#endif
		}
	}
	
	if(0 == phudiagfwd_ring_buf_is_empty(phudriver->out_buf))
	{
		queue_work(phudriver->diag_wq , &(phudriver->phudiag_write_work));
	}
	
	mutex_unlock(&phudriver->diagchar_mutex);
	
}
