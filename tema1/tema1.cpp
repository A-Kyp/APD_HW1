#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <limits.h>
// #include <bits/stdc++.h>
#include<unordered_set>
#include<sstream>
#include<cmath>
#include <pthread.h>
using namespace std;

typedef vector<unordered_set<int>> buffer, *Buffer;

typedef struct gl_var {
    queue<string> pool;
    queue<int> ids;
    buffer pp;
    vector<buffer> *buf;
    int R;
}global,  *Global;

typedef struct gl_r{
    queue<int> ids;
    vector<buffer> *buf;
    int M;
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
    
    for(int i = 0; i < M; ++i) {
        m_ids.push(i);
    }

    for(int i = 0; i <  R; ++i) {
        r_ids.push(i);
    }


    ifstream file(inFile);

    //1. read N from file
    int nrDoc;
    string x;

    if(file.is_open()) {
        file >> nrDoc;  
    }

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

    // cout << isPP(4096, 6, pp) << endl;

    //initialise "global var"
    arg.pool = docPool;
    arg.pp = pp;
    arg.buf = r_arg.buf = &buffers;
    arg.R = R;
    arg.ids = m_ids;

    r_arg.ids = r_ids;
    r_arg.M = M;

    // printq(arg.pool);

    // cout << M << R << inFile << endl;

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

    // cout << "am pornit threadurile" << endl; 

    // cout << r_arg.buf.at(0).at(0).size()<<endl;

    // 6. wait (join) the threads
    for(int i = 0; i < NUM_THREADS; ++i) {
        r = pthread_join(threads[i], &status);

        if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
    }
    cout << "am facut join la threaduri" << endl; 
    cout << "(main) buff size: " << (*arg.buf).at(0).at(0).size();

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

    cout << "am pornit ~ !" << endl;

    while(1) {
        //lock the mutex -> only one thread should read/modify the pool at a time
        pthread_mutex_lock(&m);
        id = shared.ids.front();
        shared.ids.pop();

        cont = shared.pool.empty() ? 0 : 1;
        if(cont) {
            str = shared.pool.front();
            shared.pool.pop();
        } else {
            pthread_mutex_unlock(&m);
            break;
        }

        pthread_mutex_unlock(&m);
        //unlock

        // cout << "map id: " << id << endl;
        cout << "map task: " << str << endl;


        ifstream inf(str);

        int n, aux; // the numbers to be verified

        inf >> n;
        // cout << n << endl;

        for(int i = 0; i < n; i++) {
            inf >> aux;
            // cout << "citit: " << aux << endl;
            if(aux > 0){
                for(int e = 2; e <= shared.R + 1; ++e) {
                    cout << "pp buffer size: " << (*shared.buf).at(id).at(e-2).size() << endl;
                    if (isPP(aux, e, shared.pp)) {
                        // cout << "pp found: " << aux << " ";
                        (*shared.buf).at(id).at(e-2).insert(aux);
                        cout << "pp new buffer size: " << (*shared.buf).at(id).at(e-2).size() << endl;
                    }
                }
                // cout << endl;
            }

        }

        inf.close();

        pthread_mutex_lock(&m);
        cont = shared.pool.empty() ? 0 : 1;
        pthread_mutex_unlock(&m);
        if(cont == 0) {
            break;
        }
    }

    //release the barrier
    pthread_barrier_wait(&b);

    pthread_exit(NULL);
}

void *reducer(void *arg) {
    R_global args = (R_global) arg;
    r_global shared = (*args);
    string name;
    int id;

    pthread_mutex_lock(&m);
    id = shared.ids.front();
    shared.ids.pop();
    pthread_mutex_unlock(&m);

    // cout << "reduce id: " << id << endl;

    stringstream ss;
    ss << id + 2;
    name = ss.str();


    name = "out" + name + ".txt";
    ofstream out(name);

    cout << name << endl;
    // out << name << endl;

    pthread_barrier_wait(&b);

    for(int e = 1; e < shared.M; ++e) {
        (*shared.buf).at(0).at(id).insert((*shared.buf).at(e).at(id).begin(), (*shared.buf).at(e).at(id).end());
    }

    out << (*shared.buf).at(0).at(id).size();

    out.close();
    pthread_exit(NULL);
}

void printq(queue<string> gq) {
    queue<string> g = gq;
    while (!g.empty()) {
        cout << '\t' << g.front();
        g.pop();
    }
    cout << '\n';
}

bool isPP(int n, int e, buffer pp) {
    if(pp.at(e-2).find(n) == pp.at(e-2).end()) {
        return false;
    }

    return true;
}