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
vector<int> L, R;

const int vec_alloc_lim = 6e8;

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

    L.resize(n);
    R.resize(n);
    L[0] = 0;
    for(int i = 1; i < n; i++){
        if(X[i] == X[i - 1] && Y[i] == Y[i - 1]){
            L[i] = L[i - 1];
        }
        else L[i] = i;
    }
    R[n - 1] = n - 1;
    for(int i = n - 2; i >= 0; i--){
        if(X[i] == X[i + 1] && Y[i] == Y[i + 1]){
            R[i] = R[i + 1];
        }
        else R[i] = i;
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
            assert(size < vec_alloc_lim);
        }
        visited.resize(size + 1, 0);
    }
    //hash = z1 + (x1+1)*z2 + ... + (x1+1)****(x(n-1)+1)zn
    inline long long get_h(long long old_h, int i, int old_val, int new_val) const {
        return old_h + (new_val - old_val) * products[i];
    }
    inline long long get_h(long long old_h, int i, int j, int old_i_val, int new_i_val, int old_j_val, int new_j_val) const {
        old_h = get_h(old_h, i, old_i_val, new_i_val);
        old_h = get_h(old_h, j, old_j_val, new_j_val);
        return old_h;
    }
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

inline void swap_in_order(int i, int j, vector<int>& state, long long& h, const Table_Visited& visited){
    h = visited.get_h(h, i, j, state[i], state[j], state[j], state[i]);
    swap(state[i], state[j]);
}

inline pair<int, int> restore_order(int i, State& state,const Table_Visited& visited){
    int start = i;
    while(true){
        if(i < n - 1 && X[i] == X[i + 1] && Y[i] == Y[i + 1] && state.vec[i] > state.vec[i + 1]){
            swap_in_order(i, i + 1, state.vec, state.hash1, visited);
            i++;
            continue;
        }
        if(i > 0 && X[i - 1] == X[i] && Y[i - 1] == Y[i] && state.vec[i - 1] > state.vec[i]){
            swap_in_order(i - 1, i, state.vec, state.hash1, visited);
            i--;
            continue;
        }
        break;
    }
    return {min(start, i), max(start, i)};
}
inline void restore_old_state(int i, int j, const State& old_state, State& new_state){
    for(int k = i; k <=j; k++){
        new_state.vec[k] = old_state.vec[k];
    }
    new_state.hash1 = old_state.hash1;
}

inline pair<int, int> restore_order(int ind1, int ind2, State& state,const Table_Visited& visited){
    int first = L[ind1];
    int last = R[ind2];
    for(int i = first; i < last; i++){
        if(X[i] == X[i + 1] && Y[i] == Y[i + 1] && state.vec[i] > state.vec[i + 1]){
            swap_in_order(i, i + 1, state.vec, state.hash1, visited);
        }
    }
    for(int i = last; i > first; i--){
        if(X[i] == X[i - 1] && Y[i] == Y[i - 1] && state.vec[i - 1] > state.vec[i]){
            swap_in_order(i - 1, i, state.vec, state.hash1, visited);
        }
    }

    for(int i = first; i < last - 1; i++){
        if(X[i] == X[i + 1] && Y[i] == Y[i + 1] && state.vec[i] > state.vec[i + 1]){
            swap_in_order(i, i + 1, state.vec, state.hash1, visited);
        }
    }
    for(int i = last; i > first; i--){
        if(X[i] == X[i - 1] && Y[i] == Y[i - 1] && state.vec[i - 1] > state.vec[i]){
            swap_in_order(i - 1, i, state.vec, state.hash1, visited);
        }
    }
    return {first, last};
}

inline void add_moves(const State& state, queue<pair<long long, int>>& Q, Table_Visited& visited){
    State new_state = state;
    new_state.dist++;

    for(int i = 0; i < n; i++){
        //1. Wylanie wody
        if(state.vec[i] != 0){
            new_state.vec[i] = 0;
            new_state.hash1 = visited.get_h(state.hash1, i, state.vec, new_state.vec);
            auto [first, last] = restore_order(i, new_state, visited);
            if(visited.count(new_state.hash1) == 0){
                Q.push({new_state.hash1, new_state.dist});
                visited.insert(new_state.hash1);
            }
            restore_old_state(first, last, state, new_state);
            //new_state.vec[i] = state.vec[i];
            //new_state.hash1 = state.hash1;
        }
        //2. Wylanie wody
        if(state.vec[i] != X[i]){
            new_state.vec[i] = X[i];
            new_state.hash1 = visited.get_h(state.hash1, i, state.vec, new_state.vec);
            auto [first, last] = restore_order(i, new_state, visited);
            if(visited.count(new_state.hash1) == 0){
                Q.push({new_state.hash1, new_state.dist});
                visited.insert(new_state.hash1);
            }
            restore_old_state(first, last, state, new_state);
            //new_state.vec[i] = state.vec[i];
            //new_state.hash1 = state.hash1;
        }
        //3. Przelewanki z i do j
        for(int j = 0; j < n; j++){
            if(i == j)continue;
            new_state.vec[j] = min(X[j], state.vec[i] + state.vec[j]);
            new_state.vec[i] = state.vec[i] + state.vec[j] - new_state.vec[j];
            new_state.hash1 = visited.get_h(state.hash1, i, j, state.vec, new_state.vec);
            int first1, last1, first2, last2;
            if(X[i] != X[j] || Y[i] != Y[j]){
                std::tie(first1, last1) = restore_order(i, new_state, visited);
                std::tie(first2, last2) = restore_order(j, new_state, visited);
            }
            else {
                std::tie(first1, last1) = restore_order(i, j, new_state, visited);
                std::tie(first2, last2) = {first1, last1};
            }
            if(visited.count(new_state.hash1) == 0){
                Q.push({new_state.hash1, new_state.dist});
                visited.insert(new_state.hash1);
            }
            restore_old_state(first1, last1, state, new_state);
            restore_old_state(first2, last2, state, new_state);
            // new_state.vec[i] = state.vec[i];
            // new_state.vec[j] = state.vec[j];
            // new_state.hash1 = state.hash1;
        }
    }
}

int bfs(){
    queue<pair<long long, int> > Q;
    Table_Visited visited;

    pair<long long, int> start = {0, 0};
    Q.push(start);

    State state;
    state.vec.resize(n);

    long long Y_h = 0;
    for(int i = 0; i < n; i++){
        Y_h += Y[i] * visited.products[i];
    }

    while(!Q.empty()){
        auto [h, d] = Q.front();
        Q.pop();
        if(visited.count(h) > 1)continue;
        visited.insert(h);

        for(int i = 0; i < n; i++){
            state.vec[i] = (h / visited.products[i]) % (X[i] + 1);
        }
        state.hash1 = h;
        state.dist = d;

        if(state.hash1 == Y_h)return state.dist;

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