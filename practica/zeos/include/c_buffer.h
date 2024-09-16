#ifndef C_BUFFER_H__
#define C_BUFFER_H__

#define BUFFER_SIZE 2048

typedef struct {
    char buffer[BUFFER_SIZE];
    int read_index;
    int write_index;
    int count;
} circular_buffer;



void init_buffer(circular_buffer *cb);

int is_full(circular_buffer *cb);

int is_empty(circular_buffer *cb);

void enqueue(circular_buffer *cb, char element);

char dequeue(circular_buffer *cb);

#endif /* C_BUFFER_H__*/