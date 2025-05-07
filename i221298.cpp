#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace std::chrono;

// Function to check if a permutation is the root (1, 2, ..., n)
bool is_root(const vector<int>& v) {
    for (int i = 0; i < v.size(); i++) {
        if (v[i] != i + 1) return false;
    }
    return true;
}

// Function to swap elements at positions pos and pos+1 in the permutation
void swap_elements(vector<int>& parent, const vector<int>& v, int pos) {
    parent = v;
    if (pos < parent.size() - 1) {
        swap(parent[pos], parent[pos + 1]);
    }
}

// Function to compute the inverse permutation
void compute_inverse(vector<int>& inv, const vector<int>& v) {
    inv.resize(v.size() + 1);
    for (int i = 0; i < v.size(); i++) {
        inv[v[i]] = i + 1;
    }
}

// Function to find the rightmost position where the element is not in its correct position
int compute_r(const vector<int>& v) {
    for (int i = v.size() - 1; i >= 0; i--) {
        if (v[i] != i + 1) return i + 1;
    }
    return 0;
}

// Function to find the parent of a permutation for a given tree t
void find_parent(vector<int>& parent, const vector<int>& v, int t) {
    if (is_root(v)) {
        parent.assign(v.size(), 0);
        return;
    }

    vector<int> inv;
    compute_inverse(inv, v);
    int r_v = compute_r(v);
    int v_n = v.back();

    if (v_n == v.size()) {
        if (t != v.size() - 1) {
            if (t == 2) {
                vector<int> tmp;
                swap_elements(tmp, v, inv[2] - 1);
                if (is_root(tmp)) {
                    swap_elements(parent, v, inv[1] - 1);
                } else {
                    if (v[v.size() - 2] == t || v[v.size() - 2] == v.size() - 1) {
                        swap_elements(parent, v, r_v - 1);
                    } else {
                        swap_elements(parent, v, inv[t] - 1);
                    }
                }
            } else {
                if (v[v.size() - 2] == t || v[v.size() - 2] == v.size() - 1) {
                    swap_elements(parent, v, r_v - 1);
                } else {
                    swap_elements(parent, v, inv[t] - 1);
                }
            }
        } else {
            swap_elements(parent, v, v.size() - 2);
        }
    } else if (v_n == v.size() - 1) {
        if (v[v.size() - 2] != v.size()) {
            if (v_n == t) {
                swap_elements(parent, v, v.size() - 1);
            } else {
                swap_elements(parent, v, inv[t] - 1);
            }
        } else {
            vector<int> tmp;
            swap_elements(tmp, v, v.size() - 1);
            if (is_root(tmp)) {
                if (t != 1) {
                    swap_elements(parent, v, inv[t - 1] - 1);
                } else {
                    swap_elements(parent, v, v.size() - 1);
                }
            } else {
                if (t != 1) {
                    swap_elements(parent, v, inv[t - 1] - 1);
                } else {
                    swap_elements(parent, v, v.size() - 1);
                }
            }
        }
    } else {
        if (v_n == t) {
            swap_elements(parent, v, v.size() - 1);
        } else {
            swap_elements(parent, v, inv[t] - 1);
        }
    }
}

// Function to generate all permutations of 1..n
void generate_permutations(vector<vector<int>>& permutations, vector<int>& current, int n) {
    if (current.size() == n) {
        permutations.push_back(current);
        return;
    }
    for (int i = 1; i <= n; i++) {
        if (find(current.begin(), current.end(), i) == current.end()) {
            current.push_back(i);
            generate_permutations(permutations, current, n);
            current.pop_back();
        }
    }
}

int main() {
    int n;
    cout << "Enter dimension n: ";
    cin >> n;
    cin.ignore(); // To ignore the newline character after reading n

    // Start timing for generating permutations
    auto start_gen = high_resolution_clock::now();

    // Generate all permutations of 1..n
    vector<vector<int>> permutations;
    vector<int> current;
    generate_permutations(permutations, current, n);

    // Map to store parents for each permutation
    map<vector<int>, vector<vector<int>>> parent_map;

    // Compute parents for each permutation and each tree in parallel
    #pragma omp parallel for
    for (size_t i = 0; i < permutations.size(); i++) {
        const auto& perm = permutations[i];
        vector<vector<int>> parents(n - 1, vector<int>(n));
        for (int t = 1; t <= n - 1; t++) {
            find_parent(parents[t - 1], perm, t);
        }
        #pragma omp critical
        {
            parent_map[perm] = parents;
        }
    }

    // Stop timing for generating permutations and computing parents
    auto stop_gen = high_resolution_clock::now();
    duration<double> duration_gen = stop_gen - start_gen;
    cout << "Time taken to generate all permutations and compute parents: " << duration_gen.count() << " seconds" << endl;

    // User query
    while (true) {
        cout << "Enter a permutation (space-separated, e.g., 4 2 303 1), or 'exit' to quit: ";
        string input;
        getline(cin, input);
        if (input == "exit") break;

        // Start timing for searching parents
        auto start_search = high_resolution_clock::now();

        istringstream iss(input);
        vector<int> query_perm;
        int num;
        while (iss >> num) {
            query_perm.push_back(num);
        }

        if (query_perm.size() != n) {
            cout << "Invalid permutation length. Please enter a permutation of length " << n << "." << endl;
            continue;
        }

        if (parent_map.find(query_perm) == parent_map.end()) {
            cout << "Permutation not found. Please enter a valid permutation of 1.." << n << "." << endl;
            continue;
        }

        cout << "Parents for the permutation:" << endl;
        const auto& parents = parent_map[query_perm];
        for (int t = 0; t < n - 1; t++) {
            cout << "T" << t + 1 << " parent: ";
            for (int num : parents[t]) {
                cout << num << " ";
            }
            cout << endl;
        }

        // Stop timing for searching parents
        auto stop_search = high_resolution_clock::now();
        duration<double> duration_search = stop_search - start_search;
        cout << "Time taken to search and display parents: " << duration_search.count() << " seconds" << endl;
    }

    return 0;
}