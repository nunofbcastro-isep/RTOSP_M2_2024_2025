#include "rm_rq.h"

void init_rm_rq(struct rm_rq *rq){
    rq->tasks = RB_ROOT;
    raw_spin_lock_init(&rq->lock);
    rq->nr_running = 0;
}