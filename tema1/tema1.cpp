#include <iostream>
#include <bits/stdc++.h>
using namespace std;

typedef vector<unordered_set<int>> buffer, *Buffer;

typedef struct gl_var {
    queue<string> *pool;
    queue<int> *ids;
    buffer pp;
    vector<buffer> *buf;
    int R;
    string path;
}global,  *Global;

typedef struct gl_r{
    queue<int> *ids;
    vector<buffer> *buf;
    int M;
    string path;
}r_global, *R_global;

pthread_mutex_t m;
pthread_barrier_t b;

void printq(queue<string> gq);
void *mapper(void *arg);
void *reducer(void *arg);
bool isPP(int n, int e, buffer pp);

int main(int argc, char *argv[]) {
    if(argc < 4) {
        perror("Not enough arguments");
        exit(-1);
    }

    stringstream ss;
    ss << argv[1];
    int M = stoi(ss.str());

    stringstream ss1;
    ss1 << argv[2];
    int R = stoi(ss1.str());

    int NUM_THREADS = M + R;
    string inFile = argv[3];
    string path = inFile.substr(0, inFile.find("/") + 1);
    global arg;
    r_global r_arg;
    buffer pp(R);
    queue<int> m_ids;
    queue<int> r_ids;
    queue<string> docPool;
    vector<buffer> buffers(M, buffer(R));
    pthread_t threads[NUM_THREADS];
    void* status;
    int r;
    pthread_mutex_init(&m, NULL);
    pthread_barrier_init(&b, NULL, NUM_THREADS);
    int E = R + 1;
    
    for(int i = 0; i < M; ++i) { //init mapper ids
        m_ids.push(i);
    }

    for(int i = 0; i <  R; ++i) { //init reducers ids
        r_ids.push(i);
    }


    ifstream file(inFile);

    //1. read N from file
    int nrDoc;
    string x;

    file >> nrDoc;  

    //2. read the names of the N files to be processed and put them in a queue (docPool)
    for(int i = 0; i < nrDoc && file.is_open(); i++) {
        file >> x;
        docPool.push(x);  
    }

    //3. close the file
    file.close();

    //4. pre-generate the pp for each exponent
    for(int e = 2; e <= E; ++e) {
        int limit = pow(INT_MAX, 1.0/e);
        for(int k = 1; k <= limit; k++) {
            pp.at(e-2).insert((int)pow(k,e));
        }
    }

    //initialise "global var"
    arg.pool = &docPool;
    arg.pp = pp;
    arg.buf = r_arg.buf = &buffers;
    arg.R = R;
    arg.ids = &m_ids;
    arg.path = r_arg.path = path;

    r_arg.ids = &r_ids;
    r_arg.M = M;

    // 5. starts the threads
    for(int i = 0; i < NUM_THREADS; ++i) {
        if(i < M){
            r = pthread_create(&threads[i], NULL, mapper, &arg);
        } else {
            r = pthread_create(&threads[i], NULL, reducer, &r_arg);
        }

        if (r) {
	  		printf("Eroare la crearea thread-ului %d\n", i);
	  		exit(-1);
		}
    }

    // 6. wait (join) the threads
    for(int i = 0; i < NUM_THREADS; ++i) {
        r = pthread_join(threads[i], &status);

        if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
    }

    pthread_mutex_destroy(&m);
    pthread_barrier_destroy(&b);

    return 0;
}

void *mapper(void *arg) {
    Global args = (Global) arg;
    global shared = (*args);
    string str;
    int id;
    int cont;

    pthread_mutex_lock(&m);
    id = (*shared.ids).front();
    (*shared.ids).pop();
    pthread_mutex_unlock(&m);


    while(1) {
        //lock the mutex -> only one thread should read/modify the pool at a time
        pthread_mutex_lock(&m);

        cont = (*shared.pool).empty() ? 0 : 1; //go on if there are still docs to be processed
        if(cont == 1) {
            str = (*shared.pool).front();
            (*shared.pool).pop();
            pthread_mutex_unlock(&m);
        } else {
            pthread_mutex_unlock(&m);
            break;
        }

        ifstream inf(str);

        int n, aux; // the numbers to be verified
        inf >> n;

        for(int i = 0; i < n; i++) {
            inf >> aux;
            if(aux > 0){
                for(int e = 2; e <= shared.R + 1; ++e) {
                    if (isPP(aux, e, shared.pp)) {
                        (*shared.buf).at(id).at(e-2).insert(aux);
                    }
                }
            }
        }
        inf.close();
    }

    //wait at the barrier
    pthread_barrier_wait(&b);
    pthread_exit(NULL);
}

void *reducer(void *arg) {
    R_global args = (R_global) arg;
    r_global shared = (*args);
    string name;
    int id;

    pthread_mutex_lock(&m);
    id = (*shared.ids).front();
    (*shared.ids).pop();
    pthread_mutex_unlock(&m);

    stringstream ss;
    ss << id + 2;
    name = ss.str();

    name = "myOut" + name + ".txt";
    name = shared.path + name;
    ofstream out(name);

    pthread_barrier_wait(&b); // wait for all the mapers to finish

    for(int e = 1; e < shared.M; ++e) { //combine all the partial results from mappers
        (*shared.buf).at(0).at(id).insert((*shared.buf).at(e).at(id).begin(), (*shared.buf).at(e).at(id).end());
    }

    out << (*shared.buf).at(0).at(id).size();

    out.close();
    pthread_exit(NULL);
}

bool isPP(int n, int e, buffer pp) {
    if(pp.at(e-2).find(n) == pp.at(e-2).end()) {
        return false;
    }

    return true;
}
