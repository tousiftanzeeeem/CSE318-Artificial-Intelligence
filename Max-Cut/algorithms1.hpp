#pragma once
#include<bits/stdc++.h>
using namespace std;
#define optimize() ios_base::sync_with_stdio(0);cin.tie(0);cout.tie(0);

pair<int, vector<int>> SemiGreedyMaxCut(int n, vector<pair<int,int>> graph[], double alpha) {
    vector<int> side(n + 1, 0); // 0: unassigned, 1: X, 2: Y
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    // Step 1: Initial edge with max weight
    int max_w = -1e9, u0 = -1, v0 = -1;
    for (int u = 1; u <= n; u++) {
        for (auto it : graph[u]) {
            if (u < it.first && it.second > max_w) {
                max_w = it.second;
                u0 = u;
                v0 = it.first;
            }
        }
    }
    side[u0] = 1;
    side[v0] = 2;

    while (true) {
        vector<int> unassigned;
        for (int v = 1; v <= n; v++) {
            if (side[v] == 0) unassigned.push_back(v);
        }
        if (unassigned.empty()) break;

        vector<pair<int,int>> greedyScores; // {score, vertex}
        int wmin = 1e9, wmax = -1e9;

        for (int v : unassigned) {
            int sigmaX = 0, sigmaY = 0;
            for (auto it : graph[v]) {
                if (side[it.first] == 1) sigmaX += it.second;
                if (side[it.first] == 2) sigmaY += it.second;
            }
            int score = max(sigmaX, sigmaY);
            greedyScores.push_back({score, v});
            wmin = min(wmin, min(sigmaX, sigmaY));
            wmax = max(wmax, max(sigmaX, sigmaY));
        }

        int mu = wmin + alpha * (wmax - wmin);
        vector<int> RCL;
        for (auto it : greedyScores) {
            if (it.first >= mu) RCL.push_back(it.second);
        }

        int selected;
        if (!RCL.empty()) {
            uniform_int_distribution<int> pick(0, RCL.size() - 1);
            selected = RCL[pick(rng)];
        } else {
            // Fallback to most greedy vertex if RCL is empty
            selected = max_element(greedyScores.begin(), greedyScores.end())->second;
        }

        int sigmaX = 0, sigmaY = 0;
        for (auto it : graph[selected]) {
            if (side[it.first] == 1) sigmaX += it.second;
            if (side[it.first] == 2) sigmaY += it.second;
        }

        side[selected] = (sigmaX > sigmaY) ? 2 : 1;
    }

    int cutWeight = 0;
    for (int u = 1; u <= n; u++) {
        for (auto it : graph[u]) {
            if (u < it.first && side[u] != side[it.first]) {
                cutWeight += it.second;
            }
        }
    }

    return {cutWeight, side};
}

int LocalSearchMaxCut(int n, vector<pair<int, int>> graph[], vector<int>& side) {
    bool improved = true;
    int currentCutWeight = 0;

    // Initial cut weight
    for (int u = 1; u <= n; u++) {
        for (auto it : graph[u]) {
            if (u < it.first && side[u] != side[it.first]) {
                currentCutWeight += it.second;
            }
        }
    }

    while (improved) {
        improved = false;
        int bestGain = 0;
        int vertexToMove = -1;

        for (int v = 1; v <= n; v++) {
            int sigmaSame = 0, sigmaOpp = 0;

            for (auto it : graph[v]) {
                if (side[it.first] == side[v]) sigmaSame += it.second;
                else sigmaOpp += it.second;
            }

            int gain = sigmaSame - sigmaOpp;

            if (gain > bestGain) {
                bestGain = gain;
                vertexToMove = v;
                improved = true;
            }
        }

        if (improved && vertexToMove != -1) {
            side[vertexToMove] = 3 - side[vertexToMove]; // flip side (1 -> 2, 2 -> 1)
            currentCutWeight += bestGain;
        }
    }

    return currentCutWeight;
}

pair<int,vector<int>> GreedyMaxCut(int n, vector<pair<int,int>> graph[]) {
    vector<int> side(n + 1, 0); // 0: unassigned, 1: X, 2: Y
    int max_w = -1e5, u0 = -1, v0 = -1;

    // Step 1: Find the edge with max weight
    for (int u = 1; u <= n; u++) {
        for (auto it : graph[u]) {
            if (u < it.first && it.second > max_w) {
                max_w = it.second;
                u0 = u;
                v0 = it.first;
            }
        }
    }

    // Step 2: Assign endpoints to opposite partitions
    side[u0] = 1;
    side[v0] = 2;

    // Step 3: Assign remaining vertices
    for (int v = 1; v <= n; v++) {
        if (side[v] == 0) {
            int x_contrib = 0, y_contrib = 0;
            for (auto it : graph[v]) {
                if (side[it.first] == 1) y_contrib += it.second; // opposite partition
                if (side[it.first] == 2) x_contrib += it.second;
            }
            side[v] = (x_contrib > y_contrib) ? 1 : 2;
        }
    }

    // Step 4: Calculate cut weight
    int cutWeight = 0;
    for (int u = 1; u <= n; u++) {
        for (auto it : graph[u]) {
            if (u < it.first && side[u] != side[it.first]) {
                cutWeight += it.second;
            }
        }
    }

    return {cutWeight,side};
}



double RandomizedMaxCut(int n, int m, vector<pair<int,int>> graph[], int iterations = 10) {
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> prob(0, 1);

    double totalCutWeight = 0;

    for (int i = 0; i < iterations; i++) {
        vector<int> side(n + 1, 0); 

        // Randomly assign each vertex to a partition
        for (int v = 1; v <= n; v++) {
            if (prob(rng) >= 0.5)
                side[v] = 1; // X
            else
                side[v] = 2; // Y
        }
        int cutWeight = 0;

        for (int u = 1; u <= n; u++) {
            for (auto it : graph[u]) {
                if (u < it.first && side[u] != side[it.first]) {
                    cutWeight += it.second;
                }
            }
        }

        totalCutWeight += cutWeight;
    }

    return totalCutWeight / iterations;
}

int grasp(int n, vector<pair<int,int>> graph[],int iterations){
    int bestvalue=0;
    for(int i=0;i<iterations;i++){
    pair<int,vector<int>>semigreedyCut = SemiGreedyMaxCut(n, graph,0.01);
    int grasp=LocalSearchMaxCut(n,graph,semigreedyCut.second);
    bestvalue=max(bestvalue,grasp);
    }

    return bestvalue;
}