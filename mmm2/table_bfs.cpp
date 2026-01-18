#pragma once
#include <vector>
#include <algorithm>
#include <queue>
#include <utility>

using namespace std;

const unsigned short UNVISITED = 65535;

struct table_visited {
    int n;
    vector<int>& X;
    vector<long long> products;
    // Tablice odległości dla MitM
    vector<unsigned short> distF;
    vector<unsigned short> distB;

    table_visited(int nn, vector<int>& XX) : n(nn), X(XX) {
        products.resize(n, 1);
        long long size = 1;
        for(int i = 0; i < n; i++) {
            if (i > 0) products[i] = products[i-1] * (X[i-1] + 1);
            size *= (X[i] + 1);
        }
        distF.assign(size, UNVISITED);
        distB.assign(size, UNVISITED);
    }

    inline long long get_h(long long old_h, int i, int old_val, int new_val) const {
        return old_h + (long long)(new_val - old_val) * products[i];
    }

    inline long long calculate_full_hash(const vector<int>& vec) const {
        long long h = 0;
        for(int i = 0; i < n; i++) h += (long long)vec[i] * products[i];
        return h;
    }
};

struct table_bfs {
    int n;
    vector<int>& X;
    vector<int>& Y;
    table_visited visited;

    // Kolejki przechowują teraz tylko hasze
    queue<long long> QF, QB;
    vector<int> L, R;
    bool need_sort;

    table_bfs(int nn, vector<int>& XX, vector<int>& YY)
        : n(nn), X(XX), Y(YY), visited(nn, XX), need_sort(false) {
        L.resize(n); R.resize(n);
        for(int i = 0; i < n; i++) {
            L[i] = (i > 0 && X[i-1] == X[i] && Y[i-1] == Y[i]) ? L[i-1] : i;
        }
        for(int i = n-1; i >= 0; i--) {
            R[i] = (i < n-1 && X[i+1] == X[i] && Y[i+1] == Y[i]) ? R[i+1] : i;
        }
        for(int i = 0; i < n; i++) if(R[i] > L[i]) need_sort = true;
    }

    inline void canonicalize(vector<int>& vec, long long& h, int idx) {
        if (!need_sort) return;
        int l = L[idx], r = R[idx];
        for (int i = l + 1; i <= r; i++) {
            int j = i;
            while (j > l && vec[j-1] > vec[j]) {
                h += (long long)(vec[j] - vec[j-1]) * visited.products[j-1];
                h += (long long)(vec[j-1] - vec[j]) * visited.products[j];
                swap(vec[j-1], vec[j]);
                j--;
            }
        }
    }

    int expand(bool forward) {
        queue<long long>& Q = forward ? QF : QB;
        vector<unsigned short>& myDist = forward ? visited.distF : visited.distB;
        
        int levelSize = Q.size();
        while (levelSize--) {
            long long currHash = Q.front(); Q.pop();
            
            // Odtwarzanie wektora stanu z hasza
            vector<int> currVec(n);
            for (int i = 0; i < n; i++) {
                currVec[i] = (currHash / visited.products[i]) % (X[i] + 1);
            }

            for (int i = 0; i < n; i++) {
                if (forward) {
                    process_move(currVec, currHash, i, 0, forward);         // Wylej
                    process_move(currVec, currHash, i, X[i], forward);      // Nalej
                    for (int j = 0; j < n; j++) {
                        if (i == j) continue;
                        int diff = min(currVec[i], X[j] - currVec[j]);
                        if (diff > 0) process_move_pour(currVec, currHash, i, j, diff, forward);
                    }
                } 
                else {
                    // Ruchy wstecz
                    if (currVec[i] == 0) {
                        for (int val = 1; val <= X[i]; val++) process_move(currVec, currHash, i, val, forward);
                    }
                    if (currVec[i] == X[i]) {
                        for (int val = 0; val < X[i]; val++) process_move(currVec, currHash, i, val, forward);
                    }
                    for (int j = 0; j < n; j++) {
                        if (i == j) continue;
                        if (currVec[j] == X[j] || currVec[i] == 0) {
                            for (int diff = 1; diff <= X[j]; diff++) {
                                if (currVec[j] - diff < 0 || currVec[i] + diff > X[i]) break;
                                process_move_pour(currVec, currHash, i, j, -diff, forward);
                            }
                        }
                    }
                }
                if (found) return result;
            }
        }
        return -1; 
    }

    void process_move(const vector<int>& currVec, long long currHash, int i, int newValI, bool forward) {
        vector<int> nextVec = currVec;
        nextVec[i] = newValI;
        long long nextHash = visited.get_h(currHash, i, currVec[i], newValI);
        canonicalize(nextVec, nextHash, i);
        check_and_push(nextHash, currHash, forward);
    }

    void process_move_pour(const vector<int>& currVec, long long currHash, int i, int j, int diff, bool forward) {
        vector<int> nextVec = currVec;
        nextVec[i] -= diff;
        nextVec[j] += diff;
        long long nextHash = currHash;
        nextHash = visited.get_h(nextHash, i, currVec[i], nextVec[i]);
        nextHash = visited.get_h(nextHash, j, currVec[j], nextVec[j]);
        canonicalize(nextVec, nextHash, i);
        canonicalize(nextVec, nextHash, j);
        check_and_push(nextHash, currHash, forward);
    }

    void check_and_push(long long nextHash, long long parentHash, bool forward) {
        vector<unsigned short>& myDist = forward ? visited.distF : visited.distB;
        vector<unsigned short>& otherDist = forward ? visited.distB : visited.distF;

        if (myDist[nextHash] == UNVISITED) {
            myDist[nextHash] = myDist[parentHash] + 1;
            if (otherDist[nextHash] != UNVISITED) {
                result = (int)myDist[nextHash] + (int)otherDist[nextHash];
                found = true;
            }
            if (!found) (forward ? QF : QB).push(nextHash);
        }
    }

    bool found = false;
    int result = -1;

    int solve_mitm() {
        vector<int> startV(n, 0);
        long long startH = visited.calculate_full_hash(startV);
        
        vector<int> targetV = Y;
        long long targetH = visited.calculate_full_hash(targetV);
        for(int i=0; i<n; i++) canonicalize(targetV, targetH, i);

        if (startH == targetH) return 0;

        visited.distF[startH] = 0;
        QF.push(startH);

        visited.distB[targetH] = 0;
        QB.push(targetH);

        while (!QF.empty() && !QB.empty()) {
            if (QF.size() <= QB.size()) expand(true);
            else expand(false);
            if (found) return result;
        }
        return -1;
    }

    // Funkcja zachowująca kompatybilność z main.cpp
    int get_result() {
        return solve_mitm();
    }
};