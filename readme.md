Synchronous multi-producer multi-consumer bounded queue on pthreads

## About
This is a generic C implementation of a synchronous bounded queue, similar in semantics to Go's bounded channels.
Dependencies: C99, <pthread.h>

## Usage
### Setting up
Just drop pthread_queue.c and pthread_queue.h if needed in your include path.
These files contain generic or template code to instantiate which you will need to define `QUEUE`
macro, which will contain parameters to the template and include these files. Like this:

```c
#define QUEUE (my_queue_t, data_t, queue_)
#include <pthread_queue.c>
```
where `(my_queue_t, data_t, queue_)` act as template parameters: first defines the name that queue type will use
, second defines the name of a type you want to pass through the queue, third defines the prefixes for the
function names that will be defined/decalred by pthread_queue.c/h . So you can instantiate multiple queues
as long as they have distinct first and third template parameters.

### API
struct and function definitions are given here as if `#define QUEUE (queue_t, item_t, queue_)` was used

```c
typedef struct {
        // all fields are private
} queue_t;
```
Moving `queue_t` after it has been initialized will result in "undefined behavior" because it contains mutex which are illegal to move as per POSIX

- `int queue_init(queue_t *q, size_t cap)`
  * initializes queue pointed to by `q` with capacity to hold `cap` elements.
    note that `queue_t` should not be moved in memory after initialization, as it is UB to move POSIX mutex that `queue_t` contains.
    returns 1 if initialization was successful, 0 otherwise.
- `int queue_push(queue_t *q, item_t *i)`
  * pushes a copy of `*i` into the queue's buffer. if queue is full, blocks until some other thread pops an item or closes the queue.
    if queue is closed does not push an item and returns 0, otherwise 1.
- `int queue_trypush(queue_t *q, item_t *i)`
  * like `queue_push` but returns `-1` when push couldn't be done without blocking current thread.
- `int queue_pop(queue_t *q, item_t *i)`
  * pops an oldest item from the queue into `*i` if there is one in queue. blocks until an element is pushed into this queue or the queue is closed.
    if queue is empty and closed returns 0, otherwise 1.
- `int queue_trypop(queue_t *q, item_t *i)`
  * like `queue_pop` but returns `-1` when pop couldn't be done without blocking current thread.
- `void queue_close(queue_t *q)`
  * closes the queue.
- `void queue_drop(queue_t *q)`
  * frees resources allocated to the queue.

### Example
```c
#include <stdio.h>
#include <pthread.h>
#define QUEUE (ichan, int, ichan_)
#include "pthread_queue.c"

void reader(ichan *c) {
	int s = 0;
	int x;
	while (ichan_pop(c, &x)) {
		s += x;
	}
	printf("sum: %d\n", s);
}

void writer(ichan *c) {
	for (int n = 10000; n; n--) {
		ichan_push(c, &n);
	}
	ichan_close(c);
}

int main(void) {
	pthread_t rd, wr;
	ichan c;
	ichan_init(&c, 10);
	
	pthread_create(&rd, NULL, (void*(*)(void*))reader, (void*)&c);
	pthread_create(&wr, NULL, (void*(*)(void*))writer, (void*)&c);
	
	pthread_join(wr, NULL);
	pthread_join(rd, NULL);
	
	ichan_drop(&c);
	return 0;	
}
```

