#pragma once
#include <vector>
#include <algorithm>
#include <queue>
#include <utility>
using namespace std;

//By bucket I will understand a block of indexes,
//where for each i, j in it we have X[i] == X[j] and Y[i] == Y[j]
//in case of bigger size buckets we can optimize number of states
//by sorting elements in buckets
//for small buckets we dont want to do that (it increases constant)

const int bucket_lim = 1;


//because x product is small, we can calculate sth like "deterministic hash"
//which is just a typical way of converting vector of numbers to one number,
//if numbers in vector have upper limit
//in this case:
//{z(1), z(2), z(3), ... z(n)} -> z(1) + (x(1)+1)*z(2) + (x(1)+1)*(x(2)+1)*z(3)+...+(x(1)+1)***(x(n-1)+1)*z(n)
//<(x(1)+1)***(x(n)+1)
struct table_visited {
    int n;
    vector<int>& X;
    vector<int>& Y;
    vector<char> visited;
    vector<long long> products;
    table_visited(int nn, vector<int>& XX, vector<int>& YY) : n(nn), X(XX), Y(YY){
        products.resize(n, 1);
        for(int i = 1; i < n; i++){
            products[i] = products[i-1] * (X[i - 1] + 1);
        }
        int size = 1;
        for(int i = 0; i < n; i++){
            size *= (X[i] + 1);
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


//I use three-state visited:
//0 - not added to queue or visited
//1 - added to queue
//2 - visited
//it is to prevent multiple node adding to queue, while keeping same visited memory
//(sizes of bool and char are equal)
struct table_bfs {
    int n;
    vector<int>& X;
    vector<int>& Y;
    table_visited visited;
    struct node {
        long long hash1;
        int dist;
    };
    queue<node> Q;
    struct state {
        vector<int> vec;
        long long hash1;
        int dist;
    };
    

    //left and right end of each bucket for each index
    vector<int> L, R;
    bool need_sort;

    table_bfs(int nn, vector<int>& XX, vector<int>& YY)
    : n(nn), X(XX), Y(YY), need_sort(false), visited(nn, XX, YY) {
        L.resize(n, 0);
        R.resize(n, 0);
        L[0] = 0;
        for(int i = 1; i < n; i++){
            if(X[i - 1] == X[i]){ //  && Y[i - 1] == Y[i]
                L[i] = L[i - 1];
            }
            else L[i] = i;
        }
        R[n - 1] = n - 1;
        for(int i = n - 2; i >= 0; i--){
            if(X[i] == X[i + 1]){ // && Y[i] == Y[i + 1]
                R[i] = R[i + 1];
            }
            else R[i] = i;
        }
        int bucket_size = 0;
        for(int i = 0; i < n; i++){
            bucket_size += R[i] - L[i];
        }
        if(bucket_size > bucket_lim){
            need_sort = true;
        }
    }

    inline void swap_in_order(int i, int j, state& new_state){
        new_state.hash1 = visited.get_h(new_state.hash1, i, j, new_state.vec[i], new_state.vec[j], new_state.vec[j], new_state.vec[i]);
        swap(new_state.vec[i], new_state.vec[j]);
    }
    
    inline pair<int, int> restore_order(int i, state& new_state){
        int start = i;
        while(true){
            if(i < n - 1 && X[i] == X[i + 1] && Y[i] == Y[i + 1] && new_state.vec[i] > new_state.vec[i + 1]){
                swap_in_order(i, i + 1, new_state);
                i++;
                continue;
            }
            if(i > 0 && X[i - 1] == X[i] && Y[i - 1] == Y[i] && new_state.vec[i - 1] > new_state.vec[i]){
                swap_in_order(i - 1, i, new_state);
                i--;
                continue;
            }
            break;
        }
        return {min(start, i), max(start, i)};
    }
    inline void restore_old_state(int i, int j, const state& old_state, state& new_state){
        for(int k = i; k <=j; k++){
            new_state.vec[k] = old_state.vec[k];
        }
        new_state.hash1 = old_state.hash1;
    }
    
    inline pair<int, int> restore_order(int ind1, int ind2, state& new_state){
        int first = L[ind1];
        int last = R[ind2];
        for(int i = first; i < last; i++){
            if(X[i] == X[i + 1] && Y[i] == Y[i + 1] && new_state.vec[i] > new_state.vec[i + 1]){
                swap_in_order(i, i + 1, new_state);
            }
        }
        for(int i = last; i > first; i--){
            if(X[i] == X[i - 1] && Y[i] == Y[i - 1] && new_state.vec[i - 1] > new_state.vec[i]){
                swap_in_order(i - 1, i, new_state);
            }
        }
    
        for(int i = first; i < last - 1; i++){
            if(X[i] == X[i + 1] && Y[i] == Y[i + 1] && new_state.vec[i] > new_state.vec[i + 1]){
                swap_in_order(i, i + 1, new_state);
            }
        }
        for(int i = last; i > first; i--){
            if(X[i] == X[i - 1] && Y[i] == Y[i - 1] && new_state.vec[i - 1] > new_state.vec[i]){
                swap_in_order(i - 1, i, new_state);
            }
        }
        return {first, last};
    }

    template<bool should_sort>
    inline void add_moves(const state& curr_state){
        state new_state = curr_state;
        new_state.dist++;
    
        for(int i = 0; i < n; i++){
            //1. Wylanie wody
            if(curr_state.vec[i] != 0){
                new_state.vec[i] = 0;
                new_state.hash1 = visited.get_h(curr_state.hash1, i, curr_state.vec, new_state.vec);
                int first = i, last = i;
                if constexpr(should_sort){
                    if(L[i] < R[i]){
                        std::tie(first, last) = restore_order(i, new_state);
                    }
                }
                if(visited.count(new_state.hash1) == 0){
                    Q.push({new_state.hash1, new_state.dist});
                    visited.insert(new_state.hash1);
                }
                if constexpr(should_sort){
                    restore_old_state(first, last, curr_state, new_state);
                } else {
                    new_state.vec[i] = curr_state.vec[i];
                    new_state.hash1 = curr_state.hash1;
                }
            }
            //2. Wylanie wody
            if(curr_state.vec[i] != X[i]){
                new_state.vec[i] = X[i];
                new_state.hash1 = visited.get_h(curr_state.hash1, i, curr_state.vec, new_state.vec);
                int first = i, last = i;
                if constexpr(should_sort){
                    if(L[i] < R[i]){
                        std::tie(first, last) = restore_order(i, new_state);
                    }
                }
                if(visited.count(new_state.hash1) == 0){
                    Q.push({new_state.hash1, new_state.dist});
                    visited.insert(new_state.hash1);
                }
                if constexpr(should_sort){
                    restore_old_state(first, last, curr_state, new_state);
                } else {
                    new_state.vec[i] = curr_state.vec[i];
                    new_state.hash1 = curr_state.hash1;
                }
            }
            //3. Przelewanki z i do j
            for(int j = 0; j < n; j++){
                if(i == j)continue;
                new_state.vec[j] = min(X[j], curr_state.vec[i] + curr_state.vec[j]);
                new_state.vec[i] = curr_state.vec[i] + curr_state.vec[j] - new_state.vec[j];
                new_state.hash1 = visited.get_h(curr_state.hash1, i, j, curr_state.vec, new_state.vec);
                bool one_bucket = false;
                int first1 = i, last1 = i, first2 = j, last2 = j;
                if constexpr(should_sort){
                    if(L[i] < R[i] || L[j] < R[j]){
                        if(X[i] == X[j] && Y[i] == Y[j]){
                            one_bucket = true;
                            std::tie(first1, last1) = restore_order(i, j, new_state);
                        }
                        else {
                            std::tie(first1, last1) = restore_order(i, new_state);
                            std::tie(first2, last2) = restore_order(j, new_state);
                        }
                    }
                }
                if(visited.count(new_state.hash1) == 0){
                    Q.push({new_state.hash1, new_state.dist});
                    visited.insert(new_state.hash1);
                }
                if constexpr(should_sort){
                    if(!one_bucket){
                        restore_old_state(first1, last1, curr_state, new_state);
                        restore_old_state(first2, last2, curr_state, new_state);
                    }
                    else {
                        restore_old_state(first1, last1, curr_state, new_state);
                    }
                } else {
                    new_state.vec[i] = curr_state.vec[i];
                    new_state.vec[j] = curr_state.vec[j];
                    new_state.hash1 = curr_state.hash1;
                }
            }
        }
    }

    template<bool should_sort>
    inline int run_bfs(){
        node start_node = {0, 0};
        state curr_state;
        curr_state.vec.resize(n, 0);

        long long Y_hash = 0;
        for(int i = 0; i < n; i++){
            Y_hash += Y[i] * visited.products[i];
        }

        Q.push(start_node);
        visited.insert(0);

        while(!Q.empty()){
            auto [curr_hash, curr_dist] = Q.front();
            Q.pop();
            if(visited.count(curr_hash) > 1)continue;
            visited.insert(curr_hash);

            if(curr_hash == Y_hash)
                return curr_dist;
            curr_state.hash1 = curr_hash;
            curr_state.dist = curr_dist;
            for(int i = 0; i < n; i++){
                curr_state.vec[i] = (curr_hash / visited.products[i]) % (X[i] + 1);
            }
            add_moves<should_sort>(curr_state);
        }

        return -1;
    }
    int get_result(){
        if(need_sort){
            return run_bfs<true>();
        }
        else return run_bfs<false>();
    }



};