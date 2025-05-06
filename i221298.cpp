#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <chrono>
#include <omp.h>  // OpenMP
using std::vector;
using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::unordered_map;

// Swap function: swaps element x in v with its right neighbor.
vector<int> Swap(const vector<int>& v, int x) {
    vector<int> u = v;
    int n = (int)v.size();
    for (int i = 0; i < n - 1; ++i) {
        if (u[i] == x) {
            std::swap(u[i], u[i + 1]);
            break;
        }
    }
    return u;
}

int find_r(const vector<int>& v) {
    int n = (int)v.size();
    for (int i = n - 1; i >= 0; --i) {
        if (v[i] != i + 1) return i + 1;  // 1-based
    }
    return 0;
}

vector<int> FindPosition(const vector<int>& v, int t) {
    int n = (int)v.size();
    if (t == 2 && Swap(v, t) == vector<int>(v.size(), 0)) {
        return Swap(v, t - 1);
    } else if (v[n - 2] == t || v[n - 2] == n - 1) {
        int j = find_r(v);
        return Swap(v, j);
    } else {
        return Swap(v, t);
    }
}

vector<int> Parent1(const vector<int>& v, int t) {
    int n = (int)v.size();
    if (v[n - 1] == n) {
        if (t != n - 1)
            return FindPosition(v, t);
        else
            return Swap(v, v[n - 2]);
    } else {
        if (v[n - 1] == n - 1 && v[n - 2] == n) {
            return (t == 1) ? Swap(v, n) : Swap(v, t - 1);
        } else {
            return (v[n - 1] == t) ? Swap(v, n) : Swap(v, t);
        }
    }
}

void PrintPermutation(const vector<int>& perm) {
    for (int i = 0; i < perm.size(); ++i) {
        cout << perm[i];
        if (i != perm.size() - 1) cout << " ";
    }
    cout << endl;
}

string PermToString(const vector<int>& perm) {
    string s;
    for (int num : perm)
        s += std::to_string(num) + " ";
    return s;
}

int main() {
    int n;
    cout << "Enter the dimension n of the bubble-sort network: ";
    cin >> n;

    // Step 1: generate all permutations
    vector<vector<int>> permutations;
    vector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i + 1;

    do {
        permutations.push_back(perm);
    } while (std::next_permutation(perm.begin(), perm.end()));

    int perm_count = permutations.size();

    // Prepare parent map for each permutation
    unordered_map<string, vector<vector<int>>> all_parents;

    auto start = std::chrono::high_resolution_clock::now();

    // Step 2: compute parents in parallel
    #pragma omp parallel for
    for (int i = 0; i < perm_count; ++i) {
        vector<int> p = permutations[i];
        vector<vector<int>> parents(n - 1);

        for (int t = 1; t <= n - 1; ++t) {
            parents[t - 1] = Parent1(p, t);
        }

        // Thread-safe insertion using critical section
        #pragma omp critical
        {
            all_parents[PermToString(p)] = parents;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "All permutations processed and parents computed in " << elapsed.count() << " seconds.\n";

    // Step 3: query
    vector<int> query(n);
    cout << "Enter a permutation to query: ";
    for (int i = 0; i < n; ++i) cin >> query[i];

    string key = PermToString(query);
    if (all_parents.find(key) != all_parents.end()) {
        cout << "Parents in each tree:\n";
        for (int t = 1; t <= n - 1; ++t) {
            cout << "Tree T_" << t << ": ";
            PrintPermutation(all_parents[key][t - 1]);
        }
    } else {
        cout << "Permutation not found.\n";
    }

    return 0;
}
