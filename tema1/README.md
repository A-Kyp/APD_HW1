# APD Homework 1
-> Chiper Alexandra-Diana 335CB 2022

## I. General flow
```
N -> nr. of documents to be processed
M -> nr. of mappers
R -> nr. of reducers
E -> R + 1

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
    2. read the names of the N files to be processed and put them in a queue (docPool)  - ! attention to synchronisation in threads
    3. close the file
    4. starts the threads (M + R threads)
    5. wait (join) the threads
```
## II. Implementation details
### II.1 Synchronisation
There are two big synchronisation problem that needs to be addreses:
* how to make sure that the reducers wait for all the mappers to be done
* how to make sure that only one mapper can take a task from the task pool / take an id from the id queue


The first problem is solved by using a barrier initialised with the total number of threads that are to be started. The mappers wait at the barrier once they have no more tasks to do in the task pool, and the reducers wait from the very beginning at the barrier. The second problem is solved with a mutex, more specifically by wrapping the code for the access to the data structure that holds the task pool / id pool in a pair of mutex.lock() / mutex.unlock().

### II.2 Perfect Power (PP) validation

isPP2() -> used

* takes a number n and an exponent e and returns true if there is a number x so that x ^ e == n
* implements a binary search for xin the intervat [1 ... sqrt(n)] 

isPP() -> not used because of restrictions imposed
* the same functionality as isPP2()
* uses p = pow(n, 1/e) to estimate the value of the root, then cheks arround this estimation to search for the "true integer root", meaning it checks wheather pow(p, e) or pow(p + 1, e) are equal to n
* provides correct results faster that isPP2()

### II.3 Global variables
No global variables where used except for the synchronisation elements (one mutex and one barrier).

To simulate the g;obal variables data structures were used and passed as arguments to the threads.