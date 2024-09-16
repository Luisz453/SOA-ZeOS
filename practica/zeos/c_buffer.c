#include <c_buffer.h>

void init_buffer(circular_buffer *cb){
    cb->read_index = 0;
    cb->write_index = 0;
    cb->count = 0;
}

int is_full(circular_buffer *cb){
    return cb->count == BUFFER_SIZE;
}

int is_empty(circular_buffer *cb){
    return cb->count == 0;
}

//En el buffer encolamos un char
void enqueue(circular_buffer *cb, char element){
    if(!is_full(cb)){
        cb->buffer[cb->write_index] = element;
        cb->write_index = (cb->write_index + 1) % BUFFER_SIZE;
        cb->count++;
    }
}


//Devolvemos un char y NO un int verdad?
char dequeue(circular_buffer *cb){
    char data = "";
    if(!is_empty(cb)){
        data = cb->buffer[cb->read_index];
        cb->read_index = (cb->read_index + 1) % BUFFER_SIZE;
        cb->count--;
    }
    return data;
}


