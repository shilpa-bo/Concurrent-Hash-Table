# Hash Hash Hash
In this lab, we make a hash table implementation safe to use concurrently.

## Building
To build, navigate to the lab3 directory and run:
```shell
make
```

## Running
The executable takes two command link arguments: -t changes the number of threads to use (default 4), and -s changes the number of hash table entries to add per thread (default 25,000). Note that you only want to use as many threads as the number of cores on your laptop.
For example, you can run:
```shell
/hash-table-tester -t 8 -s 50000
```
Here is an example of what the output might look like after running the program:
```shell
Generation: 20,316 usec
Hash table base: 53,285 usec
  - 0 missing
Hash table v1: 171,627 usec
  - 0 missing
Hash table v2: 25,352 usec
  - 0 missing
```
The results note how many hash table entries are missing based off of race conditions in the two implementations 

## First Implementation
In the `hash_table_v1_add_entry` function, I added one mutex around the entire critical section. I did this, as we are not too concerned about performance, and want to make sure there are no race conditions. Doing this makes the performance quite slow, as we have no real parallelism- only one thread can insert elements into the hash table at a time. But on the other hand, we don't have to worry about race conditions, as there will never be a case where there are two threads in the critical section attempting to update the same bucket at once. 

### Performance
Version 1 is a little slower than the base version as it is a serialized version of the same program which uses locks- that can create a little more overhead.

## Second Implementation
In the `hash_table_v2_add_entry` function, I created a mutex per hash table entry. I did this so we are able to work truly concurrently, without worrying about race conditions. By having a lock per bucket, we ensure that no two threads can ever attempt to add an entry to the same bucket at once- causing missing buckets; but, multiple threads can add entries to different buckets at the same time- speeding up the overall process.

### Performance
Version 2 is about (n-1) times faster than the base version, n being the number of threads used. This is due to the fact that we are using n threads on the same program, that are all working concurrently on various buckets. Thus, it is intuitive that the program finishes approximately n times faster. 

## Cleaning up
To clean up the executable and any other binary files created during the build process, run:
```shell
make clean
```
