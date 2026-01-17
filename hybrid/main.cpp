#include <iostream>
#include <assert.h>
#include <algorithm>

#include "table_bfs.h"
using namespace std;

const int TABLE_SIZE_LIM = 2e8;

//bucket sort
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

    X.resize(n, 0);
    Y.resize(n, 0);
    int x, y;
    for(int i = 0; i < n; i++){
        cin >> x >> y;
        assert(0 <= y && y <= x);
        X[i] = x;
        Y[i] = y;
    }
    sort_XY(n, X, Y);
}
inline int nwd(int a, int b){
    if(a < b)swap(a, b);
    while(b > 0){
        a = a % b;
        swap(a, b);
    }
    return a;
}
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

    long long states_number = 1;
    bool can_alloc_table = true;
    for(int i = 0; i < n; i++){
        states_number *= (X[i] + 1);
        if(states_number > TABLE_SIZE_LIM){
            can_alloc_table = false;
            break;
        }
    }
    if(can_alloc_table){
        table_bfs bfs(n, X, Y);
        return bfs.get_result();
    }
    else {
        assert(false);
    }
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