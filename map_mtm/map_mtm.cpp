#pragma once
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <queue>
#include <iostream>
using namespace std;

//By bucket I will understand a block of indexes,
//where for each i, j in it we have X[i] == X[j] and Y[i] == Y[j]
//in case of bigger size buckets we can optimize number of states
//by sorting elements in buckets
//for small buckets we dont want to do that (it increases constant)

//I will use meet in the middle for smaller products of y
//for big products of y it might be better to switch to normal bfs

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

class map_mtm {
    map_visited visited;
    queue<pair<vector<int>, int>> forward_Q;
    queue<pair<vector<int>, int>> backward_Q;
    vector<int> L, R;
    vector<int>& X, Y;
    int n;
public:
    map_mtm(int nn, vector<int>& XX, vector<int>& YY) : X(XX), Y(YY), n(nn) {
        //calculating left and right ends of each bucket
        L.resize(n, 0);
        R.resize(n, 0);
        L[0] = 0;
        R[n - 1] = n - 1;
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
            //sort(new_state.begin() + L[i], new_state.begin() + R[i] + 1);
        int back_dist = visited.get_dist(new_state, !forward);
        if(back_dist != -1 && back_dist < INF){
            ANS = min(ANS, back_dist + dist);
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
            //sort(new_state.begin() + L[i], new_state.begin() + R[i] + 1);
        }
        else {
            if(L[i] < R[i])
                sort_state(L[i], R[i], new_state, 1);
                //sort(new_state.begin() + L[i], new_state.begin() + R[i] + 1);
            if(L[j] < R[j])
                sort_state(L[j], R[j], new_state, 1);
                //sort(new_state.begin() + L[j], new_state.begin() + R[j] + 1);
        }
        int back_dist = visited.get_dist(new_state, !forward);
        if(back_dist != -1 && back_dist < INF){
            ANS = min(ANS, back_dist + dist);
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
    inline void propagate_backwards(vector<int>& new_state, const vector<int>& state, const int dist){
        new_state = state;
        for(int i = 0; i < n; i++){
            if(state[i] == 0){
                //Refilling the glass
                for(int prev_y = 1; prev_y <= X[i]; prev_y++){
                    new_state[i] = prev_y;
                    push_to_queue(state, new_state, dist + 1, i, false);
                }
            }
            else if(state[i] == X[i]){
                //Reemptying the i-th glass
                for(int prev_y = 0; prev_y < X[i]; prev_y++){
                    new_state[i] = prev_y;
                    push_to_queue(state, new_state, dist + 1, i, false);
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

        while(!forward_Q.empty()){ 
            //after forward_Q is empty, we have been it all important states
            if(ANS >= INF && back_size < front_size && !backward_Q.empty()){
                back_size++;
                //if(back_size > 1000)front_size = 0;
                state = backward_Q.front().first;
                dist = backward_Q.front().second;
                if(dist != old_dist1 && ANS < INF){
                    return ANS;
                }
                old_dist1 = dist;
                backward_Q.pop();

                //visited.backward_insert(state, dist);

                // cout << "BACKWARD: " << dist << '\n';
                // for(int i = 0; i < n; i++)cout << state[i] << ' ';
                // cout << '\n';
                
                int forward_dist = visited.forward_get_dist(state);
                if(forward_dist != -1 && forward_dist < INF){
                    ANS = min(ANS, dist + forward_dist);
                }
                propagate_backwards(new_state, state, dist);
            }
            else {
                front_size++;
                //if(front_size > 2000)back_size = 0;
                state = forward_Q.front().first;
                dist = forward_Q.front().second;
                if(dist != old_dist2 && ANS < INF){
                    return ANS;
                }
                old_dist2 = dist;
                forward_Q.pop();

                //visited.forward_insert(state, dist);

                // cout << "FORWARD: " << dist << '\n';
                // for(int i = 0; i < n; i++)cout << state[i] << ' ';
                // cout << '\n';

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
