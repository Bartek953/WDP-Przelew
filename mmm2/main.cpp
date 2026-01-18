#include <iostream>
#include <vector>
#include <assert.h>
#include <algorithm>
#include "table_bfs.cpp"

using namespace std;

inline int __gcd(int a, int b){
    if(a < b)swap(a, b);
    while(b > 0){
        a = a % b;
        swap(a, b);
    }
    return a;
}

const int TABLE_SIZE_LIM = 3e8;

// ... (funkcje sort_XY, nwd, nwd_check, full_empty_check bez zmian) ...

inline int solve(int n, vector<int>& X, vector<int>& Y){
    if(n == 0) return 0;
    
    // Warunki konieczne (GCD i stan brzegowy)
    int x_nwd = 0;
    for(int i=0; i<n; i++) x_nwd = __gcd(X[i], x_nwd);
    for(int i=0; i<n; i++) if(Y[i] % x_nwd != 0) return -1;
    
    bool ok = false;
    for(int i=0; i<n; i++) if(Y[i] == 0 || Y[i] == X[i]) ok = true;
    if(!ok) return -1;

    long long states = 1;
    for(int i=0; i<n; i++) {
        states *= (X[i] + 1);
        if(states > TABLE_SIZE_LIM) break;
    }

    if(states <= TABLE_SIZE_LIM && n <= 6000) {
        table_bfs bfs(n, X, Y);
        return bfs.solve_mitm();
    }
    assert(false);
    return -1; // Albo inna strategia dla bardzo dużych szklanek
}

int main(){
    ios::sync_with_stdio(0); cin.tie(0);
    int n;
    if (!(cin >> n)) return 0;
    vector<int> X(n), Y(n);
    vector<pair<int, int>> xy(n);
    for(int i=0; i<n; i++) cin >> xy[i].first >> xy[i].second;
    
    // Ważne: sortujemy wejście, aby kubełki (X, Y) były obok siebie dla kanonizacji
    sort(xy.begin(), xy.end());
    for(int i=0; i<n; i++) {
        X[i] = xy[i].first;
        Y[i] = xy[i].second;
    }

    cout << solve(n, X, Y) << endl;
    return 0;
}