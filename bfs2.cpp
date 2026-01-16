#include <vector>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <unordered_set>
#include <queue>
using namespace std;

int n;
vector<int> X;
vector<int> Y;

const int vec_alloc_lim = 6e6;

void read_data(){
    cin >> n;
    assert(n >= 0);

    vector<pair<int, int>> XY(n, {0, 0});
    int x, y;
    for(int i = 0; i < n; i++){
        cin >> x >> y;
        assert(0 <= y && y <= x);
        XY[i] = {x, y};
    }
    sort(XY.begin(), XY.end());

    X.resize(n);
    Y.resize(n);
    for(int i = 0; i < n; i++){
        X[i] = XY[i].first;
        Y[i] = XY[i].second;
    }
}
inline int nwd(int a, int b){
    if(a < b)swap(a, b);
    while(b != 0){
        a = a % b;
        swap(a, b);
    }
    return a;
}
bool nwd_check(){
    assert(n > 0);
    int x_nwd = 0;
    for(int i = 0; i < n; i++){
        x_nwd = nwd(x_nwd, X[i]);
    }
    for(int i = 0; i < n; i++){
        if(Y[i] % x_nwd != 0)return false;
    }
    for(int i = 0; i < n; i++){
        X[i] /= x_nwd;
        Y[i] /= x_nwd;
    }
    return true;
}
bool full_empty_check(){
    for(int i = 0; i < n; i++){
        if(Y[i] == 0 || Y[i] == X[i])return true;
    }
    return false;
}

struct Table_Visited {
    vector<char> visited;
    vector<long long> products;
    Table_Visited(){
        products.resize(n, 1);
        for(int i = 1; i < n; i++){
            products[i] = products[i-1] * (X[i - 1] + 1);
        }
        int size = 1;
        for(int i = 0; i < n; i++){
            size *= (X[i] + 1);
        }
        assert(size < vec_alloc_lim);
        visited.resize(size + 1, 0);
    }
    //hash = z1 + (x1+1)*z2 + ... + (x1+1)****(x(n-1)+1)zn
    inline long long get_h(long long old_h, int i, const vector<int>& old_state, const vector<int>& new_state) const {
        return old_h + (new_state[i] - old_state[i]) * products[i];
    }
    inline long long get_h(long long old_h, int i, int j, const vector<int>& old_state, const vector<int>& new_state) const {
        old_h = get_h(old_h, i, old_state, new_state);
        old_h = get_h(old_h, j, old_state, new_state);
        return old_h;
    }
    inline void insert(long long h){
        visited[h]++;
    }
    inline int count(long long h) const {
        return visited[h];
    }
};

struct State {
    vector<int> vec;
    long long hash1;
    int dist;
};

inline void add_moves(const State& state, queue<State>& Q, Table_Visited& visited){
    State new_state = state;
    new_state.dist++;

    for(int i = 0; i < n; i++){
        //1. Wylanie wody
        if(state.vec[i] != 0){
            new_state.vec[i] = 0;
            new_state.hash1 = visited.get_h(state.hash1, i, state.vec, new_state.vec);
            if(visited.count(new_state.hash1) == 0){
                Q.push(new_state);
                visited.insert(new_state.hash1);
            }
            new_state.vec[i] = state.vec[i];
            new_state.hash1 = state.hash1;
        }
        //2. Wylanie wody
        if(state.vec[i] != X[i]){
            new_state.vec[i] = X[i];
            new_state.hash1 = visited.get_h(state.hash1, i, state.vec, new_state.vec);
            if(visited.count(new_state.hash1) == 0){
                Q.push(new_state);
                visited.insert(new_state.hash1);
            }
            new_state.vec[i] = state.vec[i];
            new_state.hash1 = state.hash1;
        }
        //3. Przelewanki z i do j
        for(int j = 0; j < n; j++){
            if(i == j)continue;
            new_state.vec[j] = min(X[j], state.vec[i] + state.vec[j]);
            new_state.vec[i] = state.vec[i] + state.vec[j] - new_state.vec[j];
            new_state.hash1 = visited.get_h(state.hash1, i, j, state.vec, new_state.vec);
            if(visited.count(new_state.hash1) == 0){
                Q.push(new_state);
                visited.insert(new_state.hash1);
            }
            new_state.vec[i] = state.vec[i];
            new_state.vec[j] = state.vec[j];
            new_state.hash1 = state.hash1;
        }
    }
}

int bfs(){
    queue<State> Q;
    Table_Visited visited;

    State start = {vector<int>(n, 0), 0, 0};
    Q.push(start);

    while(!Q.empty()){
        State state = Q.front();
        Q.pop();
        if(visited.count(state.hash1) > 1)continue;
        visited.insert(state.hash1);

        if(state.vec == Y)return state.dist;

        add_moves(state, Q, visited);
    }
    return -1;
}



int solve(){
    if(n == 0)return 0;
    if(!nwd_check() || !full_empty_check())return -1;
    return bfs();
}

int main(){
    ios::sync_with_stdio(0);
    cin.tie(0);

    read_data();

    //cout << solve() << '\n';
    cout << solve() << '\n';

    return 0;
}