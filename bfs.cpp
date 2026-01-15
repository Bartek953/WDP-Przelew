#include <vector>
#include <iostream>
#include <assert.h>
#include <queue>
#include <set>
using namespace std;

int n;
vector<int> X;
vector<int> Y;

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

inline bool equal(const vector<int>& x, const vector<int>& y){
    assert(x.size() == n && y.size() == n);
    for(int i = 0; i < n; i++){
        if(x[i] != y[i]) return false;
    }
    return true;
}

int bfs(){
    queue< pair<vector<int>, int> > Q;
    set< vector<int> > visited;

    vector<int> start(n, 0);
    Q.push({start, 0});

    while(!Q.empty()){
        auto [state, dist] = Q.front();
        Q.pop();
        if(visited.count(state) > 0)continue;
        visited.insert(state);
        if(equal(state, Y))return dist;

        vector<int> new_state(n, 0);
        for(int i = 0; i < n; i++){
            new_state[i] = state[i];
        }
        for(int i = 0; i < n; i++){
            //wylanie z i-tego kubka
            if(state[i] != 0){
                new_state[i] = 0;
                if(visited.count(new_state) == 0){
                    Q.push({new_state, dist + 1});
                }
                new_state[i] = state[i];
            }
            //nalanie do i-tego kubka
            if(state[i] != X[i]){
                new_state[i] = X[i];
                if(visited.count(new_state) == 0){
                    Q.push({new_state, dist + 1});
                }
                new_state[i] = state[i];
            }
            for(int j = 0; j < n; j++){
                if(i == j)continue;
                //przelanie z i-tego do j-tego kubka
                new_state[j] = min(X[j], state[i] + state[j]);
                new_state[i] = state[i] + state[j] - new_state[j];
                if(visited.count(new_state) == 0){
                    Q.push({new_state, dist + 1});
                }
                new_state[i] = state[i];
                new_state[j] = state[j];
            }
        }
    }
    return -1;
}


int main(){
    ios::sync_with_stdio(0);
    cin.tie(0);

    read_data();

    cout << bfs() << '\n';


    return 0;
}