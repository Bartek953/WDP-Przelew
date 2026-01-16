#include <vector>
#include <iostream>
#include <assert.h>
#include <queue>
#include <set>
#include <unordered_map>
using namespace std;

int n;
vector<int> X;
vector<int> Y;

using hash1 = pair<long long, long long>;

struct pair_hash {
    inline size_t operator()(const hash1 &v) const {
        // Miksowanie bitów dla lepszej dystrybucji w mapie
        return v.first ^ (v.second + 0x9e3779b9 + (v.first << 6) + (v.first >> 2));
    }
};

struct Visited {
    const long long p1 = 1000003;
    const long long m1 = 1e18 + 3;
    const long long p2 = 1000037;
    const long long m2 = 1e18 + 37;

    unordered_map<hash1, int, pair_hash> m;
    vector<long long> tab_p1;
    vector<long long> tab_p2;

    Visited() {
        tab_p1.resize(n + 1);
        tab_p2.resize(n + 1);
        tab_p1[0] = 1; tab_p2[0] = 1;
        for (int i = 1; i <= n; i++) {
            tab_p1[i] = (long long)((__int128_t)tab_p1[i - 1] * p1 % m1);
            tab_p2[i] = (long long)((__int128_t)tab_p2[i - 1] * p2 % m2);
        }
    }

    bool count(hash1 h) {
        return m.find(h) != m.end();
    }

    inline hash1 get_h(hash1 h, int i, int j, const vector<int>& state, const vector<int>& new_state){
        return calc_h(h, i, state[i], new_state[i], j, state[j], new_state[j]);
    }
    hash1 calc_h(hash1 h, int i, int old_xi, int new_xi, int j, int old_xj, int new_xj) {
        // Obliczamy zmianę dla pozycji i
        long long d1_i = (long long)((__int128_t)(new_xi - old_xi + m1) % m1 * tab_p1[i] % m1);
        long long d2_i = (long long)((__int128_t)(new_xi - old_xi + m2) % m2 * tab_p2[i] % m2);

        // Obliczamy zmianę dla pozycji j
        long long d1_j = (long long)((__int128_t)(new_xj - old_xj + m1) % m1 * tab_p1[j] % m1);
        long long d2_j = (long long)((__int128_t)(new_xj - old_xj + m2) % m2 * tab_p2[j] % m2);

        // Nowy hasz to stary + delta_i + delta_j
        hash1 new_h = {
            (h.first + d1_i + d1_j) % m1,
            (h.second + d2_i + d2_j) % m2
        };

        return new_h;
    }
    void insert(hash1 h){
        m.insert({h, 1});
    }
};

void read_data(){
    cin >> n;
    assert(n >= 0);

    X.reserve(n);
    Y.reserve(n);
    int x, y;
    for(int i = 0; i < n; i++){
        cin >> x >> y;
        assert(0 <= y && y <= x);
        X.push_back(x);
        Y.push_back(y);
    }
}

void add_moves(hash1 h, int dist, const vector<int>& state, Visited& visited, queue<pair<vector<int>, pair<int, hash1>>>& Q){
    vector<int> new_state(n, 0);
    for(int i = 0; i < n; i++){
        new_state[i] = state[i];
    }
    for(int i = 0; i < n; i++){
        //wylanie z i-tego kubka
        if(state[i] != 0){
            new_state[i] = 0;
            hash1 new_h = visited.calc_h(h, i, state[i], 0, i, state[i], state[i]);
            if(visited.count(new_h) == 0){
                Q.push({new_state, {dist + 1, new_h}});
            }
            new_state[i] = state[i];
        }
        //nalanie do i-tego kubka
        if(state[i] != X[i]){
            new_state[i] = X[i];
            hash1 new_h = visited.calc_h(h, i, state[i], X[i], i, state[i], state[i]);
            if(visited.count(new_h) == 0){
                Q.push({new_state, {dist + 1, new_h}});
            }
            new_state[i] = state[i];
        }
        for(int j = 0; j < n; j++){
            if(i == j)continue;
            //przelanie z i-tego do j-tego kubka
            new_state[j] = min(X[j], state[i] + state[j]);
            new_state[i] = state[i] + state[j] - new_state[j];
            hash1 new_h = visited.get_h(h, i, j, state, new_state);
            if(visited.count(new_h) == 0){
                Q.push({new_state, {dist + 1, new_h}});
            }
            new_state[i] = state[i];
            new_state[j] = state[j];
        }
    }
}

int bfs(){
    queue< pair<vector<int>, pair<int, hash1> > > Q;
    Visited visited;

    vector<int> start(n, 0);
    Q.push({start, {0, {0, 0}}});

    while(!Q.empty()){
        auto [state, sth] = Q.front();
        auto [dist, h] = sth;
        Q.pop();
        if(visited.count(h) > 0)continue;
        visited.insert(h);
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
    return bfs();
}


int main(){
    ios::sync_with_stdio(0);
    cin.tie(0);

    read_data();

    cout << calc_res() << '\n';

    return 0;
}