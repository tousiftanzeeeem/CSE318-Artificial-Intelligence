#include<bits/stdc++.h>

using namespace std;
#define optimize() ios_base::sync_with_stdio(0);cin.tie(0);cout.tie(0);


map<int, float (*)(vector<vector<int>>)> mp;



float manhattan(vector<vector<int>> grid) {
    int n = grid.size();         
    float dist = 0;                 

    for (int i = 0; i < n; ++i) {          
        for (int j = 0; j < n; ++j) {       
            int val = grid[i][j];          
            if (val != 0) {                
                int goal_i = (val - 1) / n; 
                int goal_j = (val - 1) % n; 

                dist += abs(i - goal_i) + abs(j - goal_j);
            }
        }
    }

    return dist;
}

float hamming(vector<vector<int>> grid) {
    int n = grid.size();
    int expected = 1;
    float count = 0;
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (grid[i][j] != 0 && grid[i][j] != expected)
                count++;
            expected++;
            if (expected == n * n) expected = 0;  // blank tile is 0
        }
    }
    return count;
}

float eucledian(vector<vector<int>> grid) {
    int n = grid.size();
    float dist = 0.0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int val = grid[i][j];
            if (val != 0) {
                int goal_i = (val - 1) / n;
                int goal_j = (val - 1) % n;
                dist += sqrt(pow(i - goal_i, 2) + pow(j - goal_j, 2));
            }
        }
    }
    return round(dist);
}

float Linear_conflict(vector<vector<int>> grid) {
    int n = grid.size();
    float manh = manhattan(grid);
    float conflicts = 0;

    // Row conflicts
    for (int row = 0; row < n; ++row) {
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                int tile1 = grid[row][i];
                int tile2 = grid[row][j];
                if (tile1 != 0 && tile2 != 0) {
                    int goal_row1 = (tile1 - 1) / n;
                    int goal_col1 = (tile1 - 1) % n;
                    int goal_row2 = (tile2 - 1) / n;
                    int goal_col2 = (tile2 - 1) % n;

                    if (goal_row1 == row && goal_row2 == row && goal_col1 > goal_col2) {
                        conflicts++;
                    }
                }
            }
        }
    }

    // Column conflicts
    for (int col = 0; col < n; ++col) {
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                int tile1 = grid[i][col];
                int tile2 = grid[j][col];
                if (tile1 != 0 && tile2 != 0) {
                    int goal_row1 = (tile1 - 1) / n;
                    int goal_col1 = (tile1 - 1) % n;
                    int goal_row2 = (tile2 - 1) / n;
                    int goal_col2 = (tile2 - 1) % n;

                    if (goal_col1 == col && goal_col2 == col && goal_row1 > goal_row2) {
                        conflicts++;
                    }
                }
            }
        }
    }

    return manh + 2 * conflicts;
}

void setup(){
    mp[0] = manhattan;
    mp[1] = hamming;
    mp[2] = eucledian;
    mp[3] = Linear_conflict;
}

class AStarSearch{
protected:
        static float (*heuristic)(vector<vector<int>>);
public:
    void set_heuristic(float (*func_pointer)(vector<vector<int>>)){
          heuristic = func_pointer;
    }
};

class Node : public AStarSearch{
    vector<vector<int>>grid;
public:
    Node *parentnode = NULL;
    int g_n;
    int h_n;
    Node(vector<vector<int>>graph,Node* parent,int gn){
        grid = graph;
        parentnode = parent;
        g_n = gn;
        h_n = heuristic(grid);
    }
    void print_grid(){
        for(auto it:grid) {
            for(auto x:it) cout<<"----";
                cout<<endl;
            for(auto x:it) cout<<"| "<<x<<" ";
            cout<<"|"<<endl;
        }
    }
bool is_goal() {
    int n = grid.size();
    int expected = 1;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == n - 1 && j == n - 1) {
                if (grid[i][j] != 0) return false; // last cell must be 0
            } else {
                if (grid[i][j] != expected++) return false;
            }
        }
    }
    return true;
}

    vector<vector<int>> board(){
        return grid;
    }



};

float (*AStarSearch::heuristic)(vector<vector<int>>) = nullptr;

vector<Node*> generate_successors(Node* current_node) {
    vector<Node*> successors;
    vector<vector<int>> grid = current_node->board();  

    int n = grid.size();
    int x, y;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (grid[i][j] == 0) {
                x = i;
                y = j;
                break;
            }
        }
    }

    // 2. Define 4 possible moves
    vector<pair<int, int>> directions = {{-1,0}, {1,0}, {0,-1}, {0,1}};

    for (auto dir : directions) {
        int new_x = x + dir.first;
        int new_y = y + dir.second;

        // 3. Check bounds
        if (new_x >= 0 && new_x < n && new_y >= 0 && new_y < n) {
            vector<vector<int>> new_grid = grid;
            swap(new_grid[x][y], new_grid[new_x][new_y]);  // 4. swap 0 with new position

            Node* successor = new Node(new_grid, current_node,current_node->g_n+1);  // 5. Create new Node
            successors.push_back(successor);
        }
    }

    return successors;
}

bool is_solvable(vector<vector<int>> grid) {
    int n = grid.size(); // grid size
    vector<int> flat;
    int blank_row;

    // 1. Flatten the grid into row-major order and ignore the blank (0)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (grid[i][j] == 0)
                blank_row = i;  // store the row of the blank (0), 0-indexed from top
            else
                flat.push_back(grid[i][j]);
        }
    }

    // 2. Count inversions
    int inversions = 0;
    for (int i = 0; i < flat.size(); ++i) {
        for (int j = i + 1; j < flat.size(); ++j) {
            if (flat[i] > flat[j])
                ++inversions;
        }
    }

    // 3. Apply rules
    if (n % 2 == 1) {
        // Odd grid: solvable if inversions is even
        return inversions % 2 == 0;
    } else {
        // Even grid: blank_row is from top, convert it to bottom counting
        int blank_row_from_bottom = n - blank_row;
        if ((blank_row_from_bottom % 2 == 0 && inversions % 2 == 1) ||
            (blank_row_from_bottom % 2 == 1 && inversions % 2 == 0)) {
            return true;
        }
        return false;
    }
}

string flatten(vector<vector<int>> grid) {
    string s;
    for (auto& row : grid)
        for (int val : row)
            s += to_string(val);
    return s;
}

int main() {
    cout << "Enter Grid size" << endl;
    int k; 
    cin >> k;

    cout << "Enter Heuristic Function " << endl;
    cout << "0. Manhattan" << endl;
    cout << "1. Hamming" << endl;
    cout << "2. Eucledian" << endl;
    cout << "3. Linear_conflict" << endl;
    int hue; 
    cin >> hue;

    cout << "Enter initial Grid Configuration" << endl;
    vector<vector<int>> graph(k, vector<int>(k));
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) cin >> graph[i][j];
    }

    setup();
    AStarSearch search;
    search.set_heuristic(mp[hue]);

    Node *start_node = new Node(graph, nullptr, 0);

    if (!is_solvable(start_node->board())) {
        cout << "Unsolvable puzzle" << endl;
        return 0;
    }

priority_queue<pair<pair<float, float>, Node*>, vector<pair<pair<float, float>, Node*>>, greater<pair<pair<float, float>, Node*>>> pq;
pq.push({{start_node->g_n + start_node->h_n, start_node->h_n}, start_node});


    unordered_set<string> closed_set;
     // map<string,int>closed_set;
    int explored = 0;
    int expanded = 0;

    Node* goal_node = nullptr;
    explored++;
    while (!pq.empty()) {
        auto current = pq.top();
        pq.pop();
        Node* node = current.second;
       
        string current_state = flatten(node->board());

        if (closed_set.count(current_state)) continue;
        closed_set.insert(current_state);
        // if (closed_set[current_state]) continue;
        // closed_set[current_state] = 1;

        expanded++;

        if (node->is_goal()) {
            goal_node = node;
            break;
        }

        vector<Node*> successors = generate_successors(node);
        for (auto successor : successors) {
            string successor_state = flatten(successor->board());
            if (!closed_set.count(successor_state)) {
                explored++;
                pq.push({{successor->g_n + successor->h_n, successor->h_n}, successor});
            }
        }
    }

    if (goal_node == nullptr) {
        cout << "No solution found" << endl;
        return 0;
    }

    // Reconstruct and print the path
    vector<Node*> path;
    Node* node = goal_node;
    while (node != nullptr) {
        path.push_back(node);
        node = node->parentnode; // traverse using parent pointers
    }

    reverse(path.begin(), path.end());

    cout << "Minimum number of moves = " << path.size() - 1 << endl;
    for (auto it : path) {
        it->print_grid();
        cout << endl << endl;
    }

    cout << "Explored node count is " << explored << endl;
    cout << "Expanded node count is " << expanded << endl;

    return 0;
}



