#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/seq_file.h>
#include<linux/sched.h>
#include<linux/module.h>
#include<linux/slab.h>
#include<linux/string.h>


#define BUFFER_SIZE 100

struct proc_data {

	int parent_id;
	char *parent_name;
	int numOfChildren;
	int first_child_pid;
	char *first_child_name;
	
};

struct proc_glo_data {
	int runnable;
	int unrunnable;
	int stopped;
};

 void print_proc_report(struct seq_file *m, struct proc_data **pd, struct proc_glo_data *pgd) {
	int i;
	int procLen = pgd->runnable + pgd->unrunnable + pgd->stopped;
seq_printf(m, "PROCESS REPORTER:\nUNRUNNABLE: %d\nRUNNABLE: %d\nSTOPPED: %d\n", pgd->unrunnable, pgd->runnable, pgd->stopped);
	for(i = 0; i < procLen; i++) {
if(pd[i]->numOfChildren == 0) {
		seq_printf(m, "Process ID=%d Name=%s *No Children \n",pd[i]->parent_id, pd[i]->parent_name);
	} else {
		seq_printf(m, "Process ID=%d Name=%s number_of_children=%d first_child_pid=%d first_child_name=%s \n",pd[i]->parent_id, pd[i]->parent_name, pd[i]->numOfChildren, pd[i]->first_child_pid, pd[i]->first_child_name);	
	}
	
	}
	
}



static int hello_proc_show(struct seq_file *m, void *v) {
int buffCount = 1;
int numOfProcs = 0;
long state =0;
struct task_struct *parent_task;
struct proc_data **pd = NULL;
struct proc_glo_data *pgd = kmalloc(sizeof(struct proc_glo_data), GFP_KERNEL);

pgd->runnable = 0;
pgd->unrunnable = 0;
pgd->stopped = 0;
	for_each_process(parent_task){
	
		if(numOfProcs % BUFFER_SIZE == 0) {
			buffCount++;
			pd = krealloc(pd,  BUFFER_SIZE * buffCount * sizeof(struct proc_data *), GFP_KERNEL);
			if(pd == NULL){
				return 0;
			}
		}
	
		pd[numOfProcs] = kmalloc(sizeof(struct proc_data), GFP_KERNEL);
		pd[numOfProcs]->parent_id = parent_task->pid;
		pd[numOfProcs]->parent_name = parent_task->comm;
		pd[numOfProcs]->numOfChildren = 0;
	
	
		if(&parent_task->children != NULL) {
			struct list_head *list; 
			list_for_each(list, &parent_task->children) {
	
			struct task_struct *child_task_struct = list_entry(list, struct task_struct, sibling);
			//seq_printf(m, "child name is: %s\n", child_task_struct->comm);
				if(pd[numOfProcs]->numOfChildren == 0) {
					pd[numOfProcs]->first_child_name = child_task_struct->comm;
					pd[numOfProcs]->first_child_pid = child_task_struct->pid;
				}
			(pd[numOfProcs]->numOfChildren)++;
			}
		}
	
	state = parent_task->state;
	if(state < 0) {
	(pgd->runnable)++;
	} else if(state == 0) {
	(pgd->unrunnable)++;
	} else {
	(pgd->stopped)++;	
	}

numOfProcs++;

}
print_proc_report(m, pd, pgd);
	
	
    	
	if(pd) {
   kfree(pd);
}
if(pgd) {
	kfree(pgd);
}
 
  return 0;
}


static int hello_proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, hello_proc_show, NULL);
}

static const struct file_operations hello_proc_fops = {
  .owner = THIS_MODULE,
  .open = hello_proc_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};



int proc_init (void) {

 	proc_create("proc_report", 0, NULL, &hello_proc_fops);
	
  return 0;
}

void proc_cleanup(void) {

	remove_proc_entry("proc_report", NULL);
}





MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);

