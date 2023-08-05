// SPDX-License-Identifier: MIT

#include <pthread.h>
#include <stdlib.h>

// bounded synchronous multi-producer multi-consumer queue on pthreads
// dependencies: pthread.h, C99 or later.
// somewhat similar to the bounded channels in go.
// queues are created with fixed non-zero capacity
// queues can be marked as "closed" to inform consumers that no more elements will be added to it
// push will block if queue is full, waiting for an item to be poped
// pop retrives the oldest element if there is one, otherwise if queue is closed returns 0,
//     otherwise if queue is empty waits untill an element is inserted or the queue is closed
// close marks queue as closed, no elements can be added into closed queue 

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
#define queue_trypush QCAT3(QUEUEPREF, trypush)
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

int queue_init(queue_t *q, size_t cap) {
	if (cap == 0) {
		return 0;
	}
	q->buf = malloc(sizeof(qitem_t) * cap);
	if (q->buf == NULL) {
		free(q);
		return 0;
	}
	q->cap = cap;
	q->isclosed = q->len = q->begin = 0;
	pthread_cond_init(&q->cond, NULL);
	pthread_mutex_init(&q->mtx, NULL);
	return 1;
}

int queue_push(queue_t *q, qitem_t *i) {
	pthread_mutex_lock(&q->mtx);
	while (q->len == q->cap && q->isclosed == 0) {
		pthread_cond_wait(&q->cond, &q->mtx);
	}
	if (q->isclosed) {
		pthread_mutex_unlock(&q->mtx);
		pthread_cond_broadcast(&q->cond);
		return 0;
	}
	q->buf[(q->begin+q->len) % q->cap] = *i;
	q->len++;
	pthread_mutex_unlock(&q->mtx);
	pthread_cond_broadcast(&q->cond);
	return 1;
}

int queue_trypush(queue_t *q, qitem_t *i) {
	if (pthread_mutex_trylock(&q->mtx) != 0) {
		return -1;
	}
	if (q->isclosed) {
		pthread_mutex_unlock(&q->mtx);
		return 0;
	}
	if (q->len == q->cap) {
		pthread_mutex_unlock(&q->mtx);
		return -1;
	}
	q->buf[(q->begin+q->len) % q->cap] = *i;
	q->len++;
	pthread_mutex_unlock(&q->mtx);
	pthread_cond_broadcast(&q->cond);
	return 1;
}

int queue_pop(queue_t *q, qitem_t *i) {
	pthread_mutex_lock(&q->mtx);
	// at this point queue can be:
	//   non-empty and open
	//   non-empty and closed
	//   empty and open
	//   empty and closed
	// the only state in which it makes sense to wait is "empty and open"
	while (q->len == 0 && q->isclosed == 0) {
		pthread_cond_wait(&q->cond, &q->mtx);
	}
	if (q->len == 0) {
		pthread_mutex_unlock(&q->mtx);
		pthread_cond_broadcast(&q->cond);
		return 0;
	}
	*i = q->buf[q->begin];
	q->len--;
	q->begin = (q->begin+1) % q->cap;
	pthread_mutex_unlock(&q->mtx);
	pthread_cond_broadcast(&q->cond);
	return 1;
}

int queue_trypop(queue_t *q, qitem_t *i) {
	int e;
	if ((e = pthread_mutex_trylock(&q->mtx)) != 0) {
		return -1;
	}
	if (q->len == 0 && q->isclosed == 0) {
		pthread_mutex_unlock(&q->mtx);
		return -1;
	}
	if (q->len == 0) {
		pthread_mutex_unlock(&q->mtx);
		return 0;
	}
	*i = q->buf[q->begin];
	q->len--;
	q->begin = (q->begin+1) % q->cap;
	pthread_mutex_unlock(&q->mtx);
	pthread_cond_broadcast(&q->cond);
	return 1;
}

void queue_close(queue_t *q) {
	pthread_mutex_lock(&q->mtx);
	q->isclosed = 1;
	pthread_mutex_unlock(&q->mtx);
	pthread_cond_broadcast(&q->cond);
}

void queue_drop(queue_t *q) {
	free(q->buf);
	q->buf = NULL;
	pthread_mutex_destroy(&q->mtx);
	pthread_cond_destroy(&q->cond);
}

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

