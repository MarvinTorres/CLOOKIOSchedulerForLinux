/*
 * elevator clook
 *
 * The CLOOK I/O scheduler minimizes backtracking by satisfying requests 
 * in one direction. When it satisfies the last request in its path, it 
 * backtracks to the request that lets it hit the most requests possible
 * in its subsequent path. 
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

//Position of the read+write head
sector_t head = 0L;

struct clook_data {
	struct list_head queue;
};

/*
 * Reports the instruction (read or write) and sector of the added request.
 */
static void show_added_request(struct request *rq) {
	unsigned long curr_instruction = rq_data_dir(rq);
	sector_t curr_sector = blk_rq_pos(rq);
	char curr_instruction_str[10]; 	

	if (curr_instruction == READ) {	
		strcpy(curr_instruction_str, "R");
	} else if (curr_instruction == WRITE) {
		strcpy(curr_instruction_str, "W");
	}
	printk("[CLOOK] add %s %20lu\n", curr_instruction_str, curr_sector);	
}

/*
 * Reports the instruction (read or write) and sector of the dispatched request.
 */
static void show_dispatched_request(struct request *rq) {	
	unsigned long curr_instruction = rq_data_dir(rq);
	sector_t curr_sector = blk_rq_pos(rq);
	char curr_instruction_str[10]; 	

	if (curr_instruction == READ) {	
		strcpy(curr_instruction_str, "R");
	} else if (curr_instruction == WRITE) {
		strcpy(curr_instruction_str, "W");
	}
	printk("[CLOOK] dsp %s %20lu\n", curr_instruction_str, curr_sector);
}

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *cd = q->elevator->elevator_data;

	if (!list_empty(&cd->queue)) {
		struct request *rq;
		rq = list_entry(cd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		//Show direction and sector of dispatched request
		show_dispatched_request(rq);
		//Sector of dispatched request is visited by the head. So
		//save this sector since it is also the head's sector position
		head = blk_rq_pos(rq);
		elv_dispatch_sort(q, rq);
		return 1;
	}
	return 0;
}

static void clook_add_request(struct request_queue *q, struct request *rq)
{
	/*
	 * clook_data	The request list
	 * curr_request The current request in the request list. Used
	 *		to place the incoming request in the right spot
	 *		in the request list
	 * sector	The sector of the incoming request.
	 * curr_sector	The sector of the current request in request list.
	 *		Used to place the incoming request in the right
	 *		spot in the request list.
	 */
	struct clook_data *cd = q->elevator->elevator_data;
 	struct request *curr_request = NULL;		
	sector_t sector = blk_rq_pos(rq);		
	sector_t curr_sector = 0L;			
	struct list_head *itr = NULL;			
	
	//Show direction and sector of added request
	show_added_request(rq);	
	
	if (list_empty(&cd->queue)) {
		//Request is working with an empty list, so add it
		list_add_tail(&rq->queuelist, &cd->queue);	
		return;
	}
	
	/*
	 * The incoming request is either high or low priority. High
	 * priority requests go in front of the queue and low priority
	 * requests go in back of the queue.
	 *
	 * A request is high priority if it can be satisfied in the
	 * head's path. (recall that the head satisfies requests in one
	 * direction.) It is low priority if the head must backtrack and 
	 * start a new path to satisfy it.
	 *
	 * Note the possibility of starvation if high priority requests
	 * are added before a low priority request can be satisfied. 
	 * This function does not address this issue.
	 */
	if (sector >= head) { //high priority
		list_for_each(itr, &cd->queue) {
			curr_request = list_entry(itr, struct request, queuelist);
			curr_sector = blk_rq_pos(curr_request);
			if (curr_sector < head || sector <= curr_sector) {
				/*
				 * This request has reached the start of the 
				 * low priority area or found the right 
				 * insertion spot, so append it to the end 
				 * of the high priority area or add it to 
				 * that spot.
				 */
				list_add_tail(&rq->queuelist, itr);
				return;		
			} else if (list_is_last(itr, &cd->queue)) {
				/*
				 * This request has not found the right
				 * insertion spot nor reached the start
				 * of the low priority area but reached
				 * the end of the list. This means that
				 * the only requests in the list are
				 * high priority so it add it to 
				 * the end of the list.
				 */
				list_add(&rq->queuelist, itr);
				return;
			} else {
				/*
				 * This request is in the high priority area 
				 * but is not in the right insertion spot. 
				 * In addition it has not reached the 
				 * end of the list. So move to 
				 * the next request in the list.
				 */
				;
			}
		}
	} else { //low priority
		list_for_each(itr, &cd->queue) {
			curr_request = list_entry(itr, struct request, queuelist);
			curr_sector = blk_rq_pos(curr_request);
			if (curr_sector < head && sector <= curr_sector) {
				/*
				 * The request is in the low priority area and
				 * found the right insertion spot, so add it.
				 */
				list_add_tail(&rq->queuelist, itr);
				return;				
			} else if (list_is_last(itr, &cd->queue)) { 
				/* 
				 * If the request reached the end of the list 
				 * before finding the low priority area, then 
				 * it must be the first low priority request. 
				 * So add it right after the 
				 * last high priority request.
				 */
				list_add(&rq->queuelist, itr);
				return;
			} else {
				/* This request is in the low priority area but
				 * is not in the right insertion spot. 
				 * In addition, it has not reached 
				 * the end of the list. So move to 
				 * the next request in the list.
				 */
				;
			}
		}
	}
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *cd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &cd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *cd = q->elevator->elevator_data;

	if (rq->queuelist.next == &cd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int clook_init_queue(struct request_queue *q)
{
	struct clook_data *cd;

	cd = kmalloc_node(sizeof(*cd), GFP_KERNEL, q->node);
	if (!cd)
		return -ENOMEM;

	INIT_LIST_HEAD(&cd->queue);
	q->elevator->elevator_data = cd;
	return 0;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *cd = e->elevator_data;

	BUG_ON(!list_empty(&cd->queue));
	kfree(cd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("Jens Axboe, Marvin Torres");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("C-LOOK IO scheduler");
