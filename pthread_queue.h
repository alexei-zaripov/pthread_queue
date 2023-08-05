// SPDX-License-Identifier: MIT

#ifndef QUEUE
# error "this file needs QUEUE to be defined"
#endif
#define QU1(a, b, c) a
#define QU2(a, b, c) b
#define QU3(a, b, c) c
#define QCAT1(a, b) a ## b
#define QCAT2(a, b) QCAT1(a, b)
#define QCAT3(a, b) QCAT2(a, b)
#define QUEUENAME(a) QU1 a
#define QUEUEITEM(a) QU2 a
#define QUEUEPREF_(a) QU3 a
#define QUEUEPREF QUEUEPREF_(QUEUE)
#define queue_t QUEUENAME(QUEUE)
#define qitem_t QUEUEITEM(QUEUE)
#define queue_init QCAT3(QUEUEPREF, init)
#define queue_push QCAT3(QUEUEPREF, push)
#define queue_trypush QCAT3(QUEUEPREF, queue_trypush)
#define queue_pop QCAT3(QUEUEPREF, pop)
#define queue_trypop QCAT3(QUEUEPREF, trypop)
#define queue_close QCAT3(QUEUEPREF, close)
#define queue_drop QCAT3(QUEUEPREF, drop)

typedef struct {
	qitem_t *buf;
	size_t cap;
	size_t len;
	size_t begin;
	int isclosed;
	pthread_mutex_t mtx;
	pthread_cond_t cond;
} queue_t;

int queue_init(queue_t *q, size_t cap);
int queue_push(queue_t *q, qitem_t *i);
int queue_pop(queue_t *q, qitem_t *i);
void queue_close(queue_t *q);
void queue_drop(queue_t *q);

#undef QUEUE
#undef QU1
#undef QU2
#undef QU3
#undef QCAT1
#undef QCAT2
#undef QCAT3
#undef QUEUENAME
#undef QUEUEITEM
#undef QUEUEPREF_
#undef QUEUEPREF
#undef queue_t
#undef qitem_t
#undef queue_init
#undef queue_push
#undef queue_trypush
#undef queue_pop
#undef queue_trypop
#undef queue_close
#undef queue_drop
