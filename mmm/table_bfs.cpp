#pragma once
#include <vector>
#include <algorithm>
#include <queue>
#include <utility>

using namespace std;

const int bucket_lim = 1;
const unsigned short UNVISITED = 65535;

struct table_visited {
    int n;
    vector<int>& X;
    vector<long long> products;
    // Dwie tablice odległości dla MitM
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

    struct node {
        long long hash;
        vector<int> vec;
    };

    queue<node> QF, QB;
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

    // Utrzymuje porządek wewnątrz kubełka identycznych szklanek (kanonizacja stanu)
    inline void canonicalize(vector<int>& vec, long long& h, int idx) {
        if (!need_sort) return;
        int l = L[idx], r = R[idx];
        // Sortowanie przez wstawianie dla małych kubełków jest szybkie
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

    // Wykonuje jeden poziom BFS dla wybranej kolejki
    int expand(bool forward) {
        queue<node>& Q = forward ? QF : QB;
        vector<unsigned short>& myDist = forward ? visited.distF : visited.distB;
        vector<unsigned short>& otherDist = forward ? visited.distB : visited.distF;
        
        int levelSize = Q.size();
        while (levelSize--) {
            node curr = Q.front(); Q.pop();
            int d = myDist[curr.hash];

            // Generowanie ruchów
            for (int i = 0; i < n; i++) {
                // Ruchy w przód: standardowe
                if (forward) {
                    process_move(curr, i, -1, 0, forward);         // Wylej
                    process_move(curr, i, -1, X[i], forward);      // Nalej
                    for (int j = 0; j < n; j++) {
                        if (i == j) continue;
                        int diff = min(curr.vec[i], X[j] - curr.vec[j]);
                        if (diff > 0) process_move_pour(curr, i, j, diff, forward);
                    }
                } 
                // Ruchy wstecz: odwracanie operacji
                else {
                    // Odwróć wylanie: szklanka i jest teraz 0, wcześniej mogła mieć 1..X[i]
                    if (curr.vec[i] == 0) {
                        for (int val = 1; val <= X[i]; val++) process_move(curr, i, -1, val, forward);
                    }
                    // Odwróć nalanie: szklanka i jest pełna, wcześniej mogła mieć 0..X[i]-1
                    if (curr.vec[i] == X[i]) {
                        for (int val = 0; val < X[i]; val++) process_move(curr, i, -1, val, forward);
                    }
                    // Odwróć przelanie i -> j: 
                    // Mogło się zdarzyć tylko jeśli j jest pełne lub i jest puste
                    for (int j = 0; j < n; j++) {
                        if (i == j) continue;
                        if (curr.vec[j] == X[j] || curr.vec[i] == 0) {
                            // diff to ile przelaliśmy. Próbujemy wszystkie możliwe diff
                            for (int diff = 1; diff <= X[j]; diff++) {
                                if (curr.vec[j] - diff < 0 || curr.vec[i] + diff > X[i]) break;
                                process_move_pour(curr, i, j, -diff, forward);
                            }
                        }
                    }
                }
            }
        }
        return -1; 
    }

    void process_move(const node& curr, int i, int j, int newValI, bool forward) {
        node next = curr;
        long long oldH = next.hash;
        next.vec[i] = newValI;
        next.hash = visited.get_h(next.hash, i, curr.vec[i], newValI);
        canonicalize(next.vec, next.hash, i);
        check_and_push(next, oldH, forward);
    }

    void process_move_pour(const node& curr, int i, int j, int diff, bool forward) {
        node next = curr;
        long long oldH = next.hash;
        next.vec[i] -= diff;
        next.vec[j] += diff;
        next.hash = visited.get_h(next.hash, i, curr.vec[i], next.vec[i]);
        next.hash = visited.get_h(next.hash, j, curr.vec[j], next.vec[j]);
        canonicalize(next.vec, next.hash, i);
        canonicalize(next.vec, next.hash, j);
        check_and_push(next, oldH, forward);
    }

    void check_and_push(const node& next, long long parentHash, bool forward) {
        vector<unsigned short>& myDist = forward ? visited.distF : visited.distB;
        vector<unsigned short>& otherDist = forward ? visited.distB : visited.distF;

        if (myDist[next.hash] == UNVISITED) {
            myDist[next.hash] = myDist[parentHash] + 1;
            if (otherDist[next.hash] != UNVISITED) {
                // Spotkanie! Obliczamy wynik końcowy
                result = (int)myDist[next.hash] + (int)otherDist[next.hash];
                found = true;
            }
            if (!found) Q(forward).push(next);
        }
    }

    queue<node>& Q(bool f) { return f ? QF : QB; }
    bool found = false;
    int result = -1;

    int solve_mitm() {
        vector<int> startV(n, 0);
        long long startH = visited.calculate_full_hash(startV);
        
        vector<int> targetV = Y;
        long long targetH = visited.calculate_full_hash(targetV);
        // Kanonizujemy stan docelowy na starcie
        for(int i=0; i<n; i++) canonicalize(targetV, targetH, i);

        if (startH == targetH) return 0;

        visited.distF[startH] = 0;
        QF.push({startH, startV});

        visited.distB[targetH] = 0;
        QB.push({targetH, targetV});

        while (!QF.empty() && !QB.empty()) {
            // Optymalizacja: ruszamy się z mniejszej kolejki
            if (QF.size() <= QB.size()) {
                expand(true);
            } else {
                expand(false);
            }
            if (found) return result;
        }
        return -1;
    }
};