#include<bits/stdc++.h>
using namespace std;
#define optimize() ios_base::sync_with_stdio(0);cin.tie(0);cout.tie(0);

#include "algorithms1.hpp"

void solveAllGraphs(int totalGraphs) {
    ofstream fout("final.csv");
    fout << "GraphID,Vertices,Edges,RandomizedMaxCut,GreedyMaxCut,Greedy+LocalSearch,SemiGreedyMaxCut,\n";
fout << ",,,Constructive algorithm,,,Local search,,,,Known best solution,,\n";
    
    // Write the subheader rows
    fout << "Name,|V| or n,|E| or m,Simple Randomized or Ranomized-1,Simple Greedy or Greedy-1,";
    fout << "Semi-greedy-1,Simple local or local-1,,GRASP-1,,\n";
    
    // Write the third header row
    fout << ",,,,,,No. of iterations,Average value,No. of iterations,Best value,\n";

    //    outputFile << "G1,800,19176,a,b,c,d,e,f,g,h" << 12078 << "\n";

    for (int id = 1; id <= totalGraphs; id++) {
        string filename = "set1/g" + to_string(id) + ".rud";
        ifstream fin(filename);
        if (!fin) {
            cerr << "Could not open file " << filename << "\n";
            continue;
        }
        int n, m;
        fin >> n >> m;
        vector<pair<int,int>> graph[n + 1];

        for (int i = 0; i < m; i++) {
            int u, v, w;
            fin >> u >> v >> w;
            graph[u].push_back({v, w});
            graph[v].push_back({u, w});
        }
        int localsearchitenations=20;
        float localsearchavgvalue=0;
        int graspiterations=50;
        int graspbestvalue=0;
        float hyperparmaeter=0.8;
        int tmp=0;
        for(int i=0;i<localsearchitenations;i++){
                pair<int,vector<int>> semi = SemiGreedyMaxCut(n, graph, hyperparmaeter);
                tmp += LocalSearchMaxCut(n, graph,semi.second);
        }

        localsearchavgvalue = tmp*1.0/localsearchitenations;
        double randomized = RandomizedMaxCut(n, m, graph, 10);
        pair<int,vector<int>> greedy = GreedyMaxCut(n, graph);
        int greedyLocal = LocalSearchMaxCut(n, graph, greedy.second);
        pair<int,vector<int>> semi = SemiGreedyMaxCut(n, graph, hyperparmaeter);
        graspbestvalue = grasp(n, graph,graspiterations);

        string graph_id="G"+to_string(id);


        map<string, int> bestKnown = {
    {"G1", 12078}, {"G2", 12084}, {"G3", 12077},
    {"G11", 627}, {"G12", 621}, {"G13", 645},
    {"G14", 3187}, {"G15", 3169}, {"G16", 3172},
    {"G22", 14123}, {"G23", 14129}, {"G24", 14131},
    {"G32", 1560}, {"G33", 1537}, {"G34", 1541},
    {"G35", 8000}, {"G36", 7996}, {"G37", 8009},
    {"G43", 7027}, {"G44", 7022}, {"G45", 7020},
    {"G48", 6000}, {"G49", 6000}, {"G50", 5988}
};

        string bestsolution = bestKnown.count(graph_id) ? to_string(bestKnown[graph_id]) : "n/a";
            fout << graph_id << "," << n << "," << m << ","
            << randomized << "," << greedy.first << "," << semi.first << ","
            << localsearchitenations << "," << localsearchavgvalue << ","
            << graspiterations << "," << graspbestvalue << ","
            << bestsolution << "\n";
        if(id==0) break;
    }

    fout.close();
    cout << "Results saved to final.csv\n";
}


// void solve(){

// }
int main(){
    optimize();
    solveAllGraphs(54);
}