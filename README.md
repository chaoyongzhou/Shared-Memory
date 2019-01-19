# Shared-Memory
shared memory queue and ring buffer for multiple threads lock-free communication.


spsc - single producer and single consumer

spscq - spsc queue. multiple threads communicate in queue. producer puts one item with machine word size into queue, and consumer gets items from queue.

sqscrb - spsc ring buffer. multitple threads communicate in a continous memory block. producer puts one buffer into ring buffer, and consumer gets data from ring buffer.

put means "offer" for producer, and get means "drain" for consumer.


