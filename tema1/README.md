# APD Homework 1
-> Chiper Alexandra-Diana 335CB 2022

## General flow

N -> nr. of documents to be processed
E -> Reducer + 1

mapper -> receives a doc
    1. take its own id 
    2. verify if there are tasks (= files) to be done in the docPool 
    3. read from files values line by line
    4. check for each value > 0 if it's a perfect power of {2, 3, 4, ..., E}
    5. place the value in a corresponding list for each exponent
    6. close the file
    7. (docPool != empty) ? take one more file : wait at the barrier (= signal that the mapper has finished)

reducer -> receives a list with the pp of a sole exponent
    1. wait at the barrier <=> make sure all the mappers have finished
    2. appending and counting the pp for a sole exponent
    3. write in a file the result of the counting
    4. close the file

main -> receives NR_MAPPERS, NR_REDUCERS, and input file
    1. read N from file
    2. read the names of the N files to be processed and put them in a queue (docPool)  - ! attention to syncronasation in threads
    3. close the file
    4. starts the threads
    5. wait (join) the threads
    