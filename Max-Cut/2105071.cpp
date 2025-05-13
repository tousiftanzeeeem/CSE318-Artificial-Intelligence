#include <iostream>
#include <string>
#include <algorithm>

#include "algorithms1.hpp"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <algorithm> <graphID>\n";
        std::cerr << "Example: " << argv[0] << " greedy G1\n";
        return 1;
    }

    std::string algorithm = argv[1];
    std::string graphID = argv[2];

    // Convert algorithm to lowercase

    // Construct file path
    std::string filename = "set1/" + graphID + ".rud";
    ifstream fin(filename);
        int result = -1;
        int n, m;
        fin >> n >> m;
        vector<pair<int,int>> graph[n + 1];

        for (int i = 0; i < m; i++) {
            int u, v, w;
            fin >> u >> v >> w;
            graph[u].push_back({v, w});
            graph[v].push_back({u, w});
        }
    if (algorithm == "greedy") {
            pair<int,vector<int>>greedyCut = GreedyMaxCut(n, graph);
            cout<<"GreedyMxCut Weight "<<greedyCut.first<<endl;
    } else if (algorithm == "semigreedy") {
        float hyperparmaeter=0.9;
        pair<int,vector<int>> semi = SemiGreedyMaxCut(n, graph, hyperparmaeter);
        cout<<"SemiGreedymxcut Weight "<<semi.first<<endl;

    } else if (algorithm == "randomized") {
                int iterations=0;
                cout<<"No of iterations ";
                cin>>iterations;
                double randomized = RandomizedMaxCut(n, m, graph, iterations);
                cout<<"Randomizedmx cut "<<randomized<<endl;
    } else if (algorithm == "grasp") {
        int iterations;
                        cout<<"No of iterations ";
                cin>>iterations;
        int graspbestvalue = grasp(n, graph,iterations);
        cout<<"Grasp mx cut weight "<<graspbestvalue<<endl;
    } 


    return 0;
}
