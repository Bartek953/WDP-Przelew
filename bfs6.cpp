#include <vector>
#include <iostream>
#include <assert.h>
#include <queue>
#include <set>
#include <unordered_set>
using namespace std;

int n;
vector<int> X;
vector<int> Y;

struct State {
    vector<int> vec;
    long long h;
    int dist;
};

inline long long hash_vector(const std::vector<int>& v) {
    long long seed = 0;
    for (int x : v) {
        uint64_t z = (uint64_t)x + 0x9E3779B97F4A7C15ULL;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        seed ^= (z ^ (z >> 31)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

int simple_check(){
    bool simple_case = true;
    int res = 0;
    for(int i = 0; i < n; i++){
        if(Y[i] == 0)continue;
        if(Y[i] == X[i]){
            res++;
            continue;
        }
        simple_case = false;
        break;
    }
    if(simple_case){
        return res;
    }
    return -1;
}

void read_data(){
    cin >> n;
    assert(n >= 0);

    X.reserve(n);
    Y.reserve(n);
    int x, y;
    for(int i = 0; i < n; i++){
        cin >> x >> y;
        assert(0 <= y && y <= x);
        if(x != 0){
            X.push_back(x);
            Y.push_back(y);
        }
    }
    n = (int)X.size();
}

vector<int> new_state;
void add_moves(long long h, int dist, const vector<int>& state, unordered_set<long long>& visited, queue<State>& Q){
    new_state = state;
    for(int i = 0; i < n; i++){
        //wylanie z i-tego kubka
        if(state[i] != 0){
            new_state[i] = 0;
            long long new_h = hash_vector(new_state);
            if(visited.count(new_h) == 0){
                Q.push({new_state, new_h, dist + 1});
                visited.insert(new_h);
            }
            new_state[i] = state[i];
        }
        //nalanie do i-tego kubka
        if(state[i] != X[i]){
            new_state[i] = X[i];
            long long new_h = hash_vector(new_state);
            if(visited.count(new_h) == 0){
                Q.push({new_state, new_h, dist + 1});
                visited.insert(new_h);
            }
            new_state[i] = state[i];
        }
        for(int j = 0; j < n; j++){
            if(i == j || state[i] == 0 || state[j] == X[j])continue;
            //przelanie z i-tego do j-tego kubka
            new_state[j] = min(X[j], state[i] + state[j]);
            new_state[i] = state[i] + state[j] - new_state[j];
            long long new_h = hash_vector(new_state);
            if(visited.count(new_h) == 0){
                Q.push({new_state, new_h, dist + 1});
                visited.insert(new_h);
            }
            new_state[i] = state[i];
            new_state[j] = state[j];
        }
    }
}

int bfs(){
    queue< State > Q;
    unordered_set<long long> visited;
    new_state = vector<int>(n, 0);

    vector<int> start(n, 0);
    Q.push({
        .vec = start, 
        .h = hash_vector(vector<int>(n, 0)),
        .dist = 0
    });

    while(!Q.empty()){
        auto [state, h, dist] = Q.front();
        Q.pop();
        //if(visited.count(h) > 0)continue;
        //visited.insert(h);
        if(state == Y)return dist;

        add_moves(h, dist, state, visited, Q);
    }
    return -1;
}

int nwd(int a, int b){
    if(a < b)swap(a, b);
    while(b > 0){
        a = a % b;
        swap(a, b);
    }
    return a;
}
bool nwd_condition(){
    int curr_nwd = X[0];
    for(int i = 1; i < n; i++){
        curr_nwd = nwd(curr_nwd, X[i]);
    }
    for(int i = 0; i < n; i++){
        if(Y[i] % curr_nwd != 0) return false;
    }
    for(int  i = 0; i < n; i++){
        X[i] /= curr_nwd;
        Y[i] /= curr_nwd;
    }
    return true;
}
bool volume_condition(){
    for(int i = 0; i < n; i++){
        if(Y[i] == 0 || Y[i] == X[i])return true;
    }
    return false;
}
inline bool check_conditions(){
    return nwd_condition() && volume_condition();
}


int calc_res(){
    if(n == 0)return 0;
    if(!check_conditions())return -1;
    if(simple_check() != -1)return simple_check();
    return bfs();
}


int main(){
    ios::sync_with_stdio(0);
    cin.tie(0);

    read_data();

    cout << calc_res() << '\n';

    return 0;
}