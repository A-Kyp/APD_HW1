# APD Homework 1
-> Chiper Alexandra-Diana 335CB 2022

## General flow
```
N -> nr. of documents to be processed
E -> Reducer + 1

mapper -> receives a doc
    1. take id and file
    1. read values line by line
    2. check for each value > 0 if it's a perfect power of {2, 3, 4, ..., E}
    3. place the value in a corresponding list for each exponent
    4. close the doc
    5. release the barrier/semaphore

reducer -> receives a list with the pp of a sole exponent
    1. aquire the semaphore/barrier <=> make sure all the reducers have finished
    2. appending and counting the pp for a sole exponent
    3. write in a file the result of the counting

main -> receives NR_MAPPERS, NR_REDUCERS, and input file
    1. read N from file
    2. read the names of the N files to be processed and put them in a queue (docPool)  - ! attention to syncronasation
    3. close the file
    4. pre-generate the pp for each exponent
    5. starts the threads
    6. wait (join) the threads
```