#include <iostream>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <queue>
#include <iostream>

using namespace std;

//Meet in the middle implementation - parrarel bfs from start and end

const long long back_timeout = 12e6; //48MB
//if one operation on back exceed back_timeout
//then I will switch from mitm to bfs

//By bucket I will understand a block of indexes,
//where for each i, j in it we have X[i] == X[j] and Y[i] == Y[j]
//in case of bigger size buckets we can optimize number of states
//by sorting elements in buckets
//for small buckets we dont want to do that (it increases constant)

const int INF = 2e9;

struct hash_vector {
    size_t operator()(const std::vector<int>& v) const {
        size_t seed = 0;
        for (int i : v) {
            seed ^= std::hash<int>{}(i) + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

struct map_visited {
    unordered_map<vector<int>, int, hash_vector> forward_visited;
    unordered_map<vector<int>, int, hash_vector> backward_visited;

    inline void forward_insert(const vector<int>& state, int dist){
        forward_visited[state] = dist;
    }
    inline int forward_get_dist(const vector<int>& state) const {
        auto res = forward_visited.find(state);
        if(res == forward_visited.end())
            return -1;
        else
            return res->second;
    }
    inline void backward_insert(const vector<int>& state, int dist){
        backward_visited[state] = dist;
    }
    inline int backward_get_dist(const vector<int>& state) const {
        auto res = backward_visited.find(state);
        if(res == backward_visited.end())
            return -1;
        else
            return res->second;
    }
    inline void insert(const vector<int>& state, int dist, bool forward){
        if(forward)forward_insert(state, dist);
        else backward_insert(state, dist);
    }
    inline int get_dist(const vector<int>& state, bool forward){
        if(forward)return forward_get_dist(state);
        else return backward_get_dist(state);
    }
};

//Im using meet in the middle algorithm
//that means Im doing bfs's from start and end at the same time
//This becomes much more efficient for large depths
//even considering fact, that going backwards is more expensive than forwards
//I can optimize it by going from the "smaller side"

class map_mtm {
    map_visited visited;
    queue<pair<vector<int>, int>> forward_Q;
    queue<pair<vector<int>, int>> backward_Q;
    vector<int> L, R;
    vector<int>& X, Y;
    int n;
    bool do_mtm;
public:
    map_mtm(int nn, vector<int>& XX, vector<int>& YY) : X(XX), Y(YY), n(nn), do_mtm(true) {
        //calculating left and right ends of each bucket
        L.resize(n, 0);
        R.resize(n, 0);
        L[0] = 0;
        R[n - 1] = n - 1;

        //calculation of L, R - left and right end of each bucket
        for(int i = 1; i < n; i++){
            if(X[i] == X[i - 1])
                L[i] = L[i - 1];
            else
                L[i] = i;
        }
        for(int i = n - 2; i >= 0; i--){
            if(X[i] == X[i + 1])
                R[i] = R[i + 1];
            else
                R[i] = i;
        }
    }
    int ANS = INF;

    //for bucket sort
    //im using bubble sort, because i only need to do 2 to 4 forloops
    //(only one or two elements are not sorted)
    //and it will be more efficient than sort() function
    inline void sort_state(int l, int r, vector<int>& state, int repeat){
        while(repeat--){
            for(int i = l; i < r; i++){
                if(state[i] > state[i + 1])swap(state[i], state[i + 1]);
            }
            for(int i = r; i > l; i--){
                if(state[i - 1] > state[i])swap(state[i - 1], state[i]);
            }
        }
    }
    
    inline void restore_old_state(int l, int r, vector<int>& new_state, const vector<int>& old_state){
        for(int i = l; i <= r; i++){
            new_state[i] = old_state[i];
        }
    }
    inline void push_to_queue(const vector<int>& state, vector<int>& new_state, int dist, int i, bool forward){
        if(L[i] < R[i])
            sort_state(L[i], R[i], new_state, 1);
        int back_dist = visited.get_dist(new_state, !forward);
        if(back_dist != -1 && back_dist < INF){
            ANS = min(ANS, back_dist + dist);
            //Im checking whetever connection exist
        }
        if(visited.get_dist(new_state, forward) == -1){
            if(forward)forward_Q.push({new_state, dist});
            else backward_Q.push({new_state, dist});

            visited.insert(new_state, dist, forward);
        }
        restore_old_state(L[i], R[i], new_state, state);
    }
    inline void push_to_queue(const vector<int>& state, vector<int>& new_state, int dist, int i, int j, bool forward){
        if(L[i] == L[j]){
            sort_state(L[i], R[i], new_state, 2);
        }
        else {
            if(L[i] < R[i])
                sort_state(L[i], R[i], new_state, 1);
            if(L[j] < R[j])
                sort_state(L[j], R[j], new_state, 1);
        }
        int back_dist = visited.get_dist(new_state, !forward);
        if(back_dist != -1 && back_dist < INF){
            ANS = min(ANS, back_dist + dist);
            //Checking whetever connection exist
        }
        if(visited.get_dist(new_state, forward) == -1){
            if(forward)forward_Q.push({new_state, dist});
            else backward_Q.push({new_state, dist});

            visited.insert(new_state, dist, forward);
        }
        if(L[i] == L[j]){
            restore_old_state(L[i], R[i], new_state, state);
        }
        else {
            restore_old_state(L[i], R[i], new_state, state);
            restore_old_state(L[j], R[j], new_state, state);
        }
    }

    inline void propagate_forwards(vector<int>& new_state, const vector<int>& state, const int dist){
        new_state = state;
        
        for(int i = 0; i < n; i++){
            //1. Filling the glass
            if(state[i] != X[i]){
                new_state[i] = X[i];
                push_to_queue(state, new_state, dist + 1, i, true);
            }
            //2. Emptying the glass
            if(state[i] != 0){
                new_state[i] = 0;
                push_to_queue(state, new_state, dist + 1, i, true);
            }
            //3. Pouring from i-th glass to j-th glass
            for(int j = 0; j < n; j++){
                if(i == j || state[i] == 0 || state[j] == X[j])continue;
                new_state[j] = min(X[j], state[i] + state[j]);
                new_state[i] = state[i] + state[j] - new_state[j];
                push_to_queue(state, new_state, dist + 1, i, j, true);
            }
        }
    }
    long long operations_counter = 0;
    inline void propagate_backwards(vector<int>& new_state, const vector<int>& state, const int dist){
        
        new_state = state;
        for(int i = 0; i < n; i++){
            if(state[i] == 0){
                //Refilling the glass
                for(int prev_y = 1; prev_y <= X[i]; prev_y++){
                    new_state[i] = prev_y;
                    push_to_queue(state, new_state, dist + 1, i, false);
                    operations_counter += n;
                }
            }
            else if(state[i] == X[i]){
                //Reemptying the i-th glass
                for(int prev_y = 0; prev_y < X[i]; prev_y++){
                    new_state[i] = prev_y;
                    push_to_queue(state, new_state, dist + 1, i, false);
                    operations_counter += n;
                }
            }
        }
        for(int i = 0; i < n; i++){
            for(int j = 0; j < n; j++){
                if(i == j || (state[i] != 0 && state[j] != X[j]))continue;
                //Reversing pour i -> j
                int max_v = min(X[i] - state[i], state[j]);
                for(int v = 1; v <= max_v; v++) {
                    new_state[i] = state[i] + v; 
                    new_state[j] = state[j] - v;
                    push_to_queue(state, new_state, dist + 1, i, j, false);
                    operations_counter += n;
                    if(operations_counter > back_timeout){
                        do_mtm = false;
                        //cerr << "BREAK!";
                        return;
                    }
                }
            }
        }
    }


    inline int run_mtm(){
        forward_Q.push({vector<int>(n, 0), 0});
        backward_Q.push({Y, 0});
        visited.forward_insert(vector<int>(n, 0), 0);
        visited.backward_insert(Y, 0);

        vector<int> state(n, 0), new_state(n, 0);
        int dist;

        int old_dist1 = 0, old_dist2 = 0;

        long long front_size = 0, back_size = 0;


        //with Q sizes or not?
        //Im using heuristics to go from the smaller side
        //In bad scenario it should automatically switch to bfs
        while(!forward_Q.empty()){ 
            //after forward_Q is empty, we have been it all important states
            //if ANS is known, so there exist connection, so we only need to process current forward layer
            //if operations on the back exceeds backward_timeout, Im switching to standard bfs (do_mtm condition)
            if(do_mtm && ANS >= INF && back_size + backward_Q.size() < front_size + forward_Q.size() && !backward_Q.empty()){
                back_size++;
                state = backward_Q.front().first;
                dist = backward_Q.front().second;
                if(dist != old_dist1 && ANS < INF){
                    return ANS;
                }
                old_dist1 = dist;
                backward_Q.pop();
                
                int forward_dist = visited.forward_get_dist(state);
                if(forward_dist != -1 && forward_dist < INF){
                    ANS = min(ANS, dist + forward_dist);
                }
                propagate_backwards(new_state, state, dist);
            }
            else {
                front_size++;
                state = forward_Q.front().first;
                dist = forward_Q.front().second;
                if(dist != old_dist2 && ANS < INF){
                    return ANS;
                }
                old_dist2 = dist;
                forward_Q.pop();

                if(state == Y)return dist;

                int backward_dist = visited.backward_get_dist(state);
                if(backward_dist != -1 && backward_dist < INF){
                    ANS = min(ANS, dist + backward_dist);
                }
                propagate_forwards(new_state, state, dist);
            }
        }
        return ANS >= INF ? -1 : ANS;
    }
    inline int get_result(){
        return run_mtm();
    }
};


//Im checking cases, where each yi is full or empty
//so the result is just number of non-empty glasses
int simple_check(int n, vector<int>& X, vector<int>& Y){
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

//bucket sort at the beginning
inline void sort_XY(int &n, vector<int>& X, vector<int>& Y){
    vector<pair<int, int>> XY(n, {0, 0});
    for(int i = 0; i < n; i++)
        XY[i] = {X[i], Y[i]};
    sort(XY.begin(), XY.end());
    for(int i = 0; i < n; i++){
        X[i] = XY[i].first;
        Y[i] = XY[i].second;
    }
}

inline void read_data(int &n, vector<int>& X, vector<int>& Y){
    cin >> n;
    assert(0 <= n);

    int x, y;
    for(int i = 0; i < n; i++){
        cin >> x >> y;
        assert(0 <= y && y <= x);
        //if x = 0 the glass is not interesting - it doesnt change anything
        if(x != 0){
            X.push_back(x);
            Y.push_back(y);
        }
    }
    n = (int)X.size();
    sort_XY(n, X, Y);
    //I can sort X, Y in buckets
}
inline int nwd(int a, int b){
    if(a < b)swap(a, b);
    while(b > 0){
        a = a % b;
        swap(a, b);
    }
    return a;
}
//Observation - in each operation we have that level of water in each glass
//is divisible by nwd{xi}, so if any yi is not divisible by nwd{xi}
//the answer must be -1
//to compress the data i will also divide all variables by nwd{xi}
//because it doesnt change the result
inline bool nwd_check(int n, vector<int>& X, vector<int>& Y){
    assert(n > 0);
    int x_nwd = 0;
    for(int i = 0; i < n; i++){
        x_nwd = nwd(X[i], x_nwd);
    }
    for(int i = 0; i < n; i++){
        if(Y[i] % x_nwd != 0) return false;
        Y[i] /= x_nwd;
        X[i] /= x_nwd;
    }
    return true;
}
//Another thing that doesnt change while performing operations is that
//always there's at least one full or empty glass
//so if for all yi we have 0 < yi < xi, then answer is -1
inline bool full_empty_check(int n, vector<int>& X, vector<int>& Y){
    for(int i = 0; i < n; i++){
        if(Y[i] == 0 || Y[i] == X[i])return true;
    }
    return false;
}
inline bool check_conditions(int n, vector<int>& X, vector<int>& Y){
    return nwd_check(n, X, Y) && full_empty_check(n, X, Y);
}
inline int solve(int n, vector<int>& X, vector<int>& Y){
    if(n == 0)return 0;
    if(!check_conditions(n, X, Y))return -1;

    int simple_res = simple_check(n, X, Y);
    if(simple_res != -1)return simple_res;

    map_mtm mtm(n, X, Y);
    return mtm.get_result();
}
int main(){
    ios::sync_with_stdio(0);
    cin.tie(0);

    int n;
    vector<int> X, Y;

    read_data(n, X, Y);

    cout << solve(n, X, Y) << '\n';

    return 0;
}