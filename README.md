# Shared-Memory
shared memory queue and ring buffer for multiple threads lock-free communication.

# SPSC
spsc - single producer and single consumer

spscq - spsc queue. multiple threads communicate in queue. producer puts one item with machine word size into queue, and consumer gets items from queue.

sqscrb - spsc ring buffer. multitple threads communicate in a continous memory block. producer puts one buffer into ring buffer, and consumer gets data from ring buffer.

# MPSC

mpsc - multiple producers and single consumer

mpscq - mpsc queue. multiple threads communicate in queue. one producer put one item with machine word size into queue, and consumer gets items from queue.

mqscrb - mpsc ring buffer. multitple threads communicate in a continous memory block. one producer puts one buffer into ring buffer, and consumer gets data from ring buffer.

# Note
put means "offer" for producer, and get means "drain" for consumer.

# Verification

make test_cspscq && bin/test

make test_cspscrb && bin/test

make test_cmpscq && bin/test

make test_cmpscrb && bin/test


