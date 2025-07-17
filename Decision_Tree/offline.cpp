#include <bits/stdc++.h>
using namespace std;

class DataPoint {
public:
    vector<string> features;
    string label;

    DataPoint(vector<string> features_, string label_) : features(features_), label(label_) {}
};

class Node {
public:
    bool isLeaf;
    string label;
    int splitAttr;
    map<string, Node*> children;

    Node() : isLeaf(false), splitAttr(-1) {}
};

class DecisionTree {
private:
    vector<string> attributeNames;
    vector<DataPoint> dataset;
    map<int, set<string>> attributeValues;
    set<string> classLabels;
    int numAttributes = 0;
    mt19937 rng;
    int nodeCount = 0;

public:
    DecisionTree() {
        rng.seed(time(0));
    }

    void loadCSV(const string &filename) {
        ifstream file(filename);
        string line;
        getline(file, line);
        attributeNames = splitCSVLine(line);
        int idIndex = -1;
        vector<string> newAttributeNames;
        if (!attributeNames.empty() && (attributeNames[0] == "Id" || attributeNames[0] == "id")) {
            idIndex = 0;
        }
        for (int i = 0; i < attributeNames.size() - 1; ++i) { 
            if (i != idIndex)
                newAttributeNames.push_back(attributeNames[i]);
        }
        numAttributes = newAttributeNames.size();
        attributeNames = newAttributeNames;

        while (getline(file, line)) {
            vector<string> tokens = splitCSVLine(line);
            vector<string> features;
            for (int i = 0; i < tokens.size() - 1; ++i) {
                if (i == idIndex) continue; 
                features.push_back(tokens[i]);
                attributeValues[features.size() - 1].insert(tokens[i]);
            }
            string label = tokens.back();
            classLabels.insert(label);
            dataset.emplace_back(features, label);
        }

        // cout << "Loaded " << dataset.size() << " data points.\n";
        // cout << "Attributes: ";
        // for (const auto& name : attributeNames) cout << name << ", ";
        // cout << endl;
        // if (!dataset.empty()) {
        //     cout << "First data point: ";
        //     for (const auto& f : dataset[0].features) cout << f << ", ";
        //     cout << "Label: " << dataset[0].label << endl;
        // }
    }

    double run(string method, int maxDepth) {
        double totalAcc = 0;
        int totalNodeCount = 0;
        for (int i = 0; i < 20; ++i) {
            shuffle(dataset.begin(), dataset.end(), rng);
            int splitPoint = dataset.size() * 0.8;
            vector<DataPoint> train(dataset.begin(), dataset.begin() + splitPoint);
            vector<DataPoint> test(dataset.begin() + splitPoint, dataset.end());
            nodeCount= 0;
            Node* root = buildTree(train, 0, maxDepth, method);
            totalNodeCount += nodeCount;
            double acc = evaluate(root, test);
            // cout<<"Run " << i + 1 << ": Node Count = " << nodeCount << "\n";
            // cout << "Run " << i + 1 << ": Accuracy = " << acc * 100 << "%\n";
            totalAcc += acc;
        }
        // cout<< "Average node count: " << (totalNodeCount / 20.0) << "\n";
        double avg_acc = (totalAcc / 20.0) * 100;
        return avg_acc;
    }

private:
    vector<string> splitCSVLine(const string &line) {
        stringstream ss(line);
        string token;
        vector<string> tokens;
        while (getline(ss, token, ',')) tokens.push_back(token);
        return tokens;
    }

    double log2safe(double x) {
        return (x <= 0) ? 0 : log2(x);
    }

    double entropy(const vector<DataPoint> &data) {
        map<string, int> freq;
        for (auto &d : data) freq[d.label]++;
        double e = 0;
        for (auto it : freq) {
            double p = (double)it.second / data.size();
            e -= p * log2safe(p);
        }
        return e;
    }

    double infoGain(const vector<DataPoint> &data, int attr) {
        double baseEntropy = entropy(data);
        map<string, vector<DataPoint>> splits;
        for (auto &d : data)
            splits[d.features[attr]].push_back(d);
        double weighted = 0;
        for (auto it : splits) {
            double p = (double)it.second.size() / data.size();
            weighted += p * entropy(it.second);
        }
        return baseEntropy - weighted;
    }

    double intrinsicValue(const vector<DataPoint> &data, int attr) {
        map<string, int> freq;
        for (auto &d : data) freq[d.features[attr]]++;
        double iv = 0;
        for (auto it : freq) {
            double p = (double)it.second / data.size();
            iv -= p * log2safe(p);
        }
        return iv;
    }

    double gainRatio(const vector<DataPoint> &data, int attr) {
        double ig = infoGain(data, attr);
        double iv = intrinsicValue(data, attr);
        return (iv == 0) ? 0 : ig / iv;
    }

    double NWIG(const vector<DataPoint> &data, int attr) {
        double ig = infoGain(data, attr);
        int k = attributeValues[attr].size();
        double penalty = log2(k + 1);
        double sizeAdjust = 1.0 - (double)(k - 1) / data.size();
        return (penalty == 0) ? 0 : (ig / penalty) * sizeAdjust;
    }

    string majorityLabel(const vector<DataPoint> &data) {
        map<string, int> freq;
        for (auto &d : data) freq[d.label]++;
        string majority;
        int maxCount = 0;
        for (auto &it : freq) { 
            if (it.second > maxCount) {
                maxCount = it.second;
                majority = it.first;
            }
        }
        return majority;
    }

    Node* buildTree(const vector<DataPoint> &data, int depth, int maxDepth, string method) {
        cout<<"current depth: " << depth << "\n";
        string majority = majorityLabel(data);
        bool flag = true;
        for (const auto& d : data) {
            if (d.label != majority) {
                flag = false;
                break;
            }
        }

        if (data.empty() || flag || (maxDepth > 0 && depth == maxDepth)) {
            Node *leaf = new Node();
            nodeCount++;
            leaf->isLeaf = true;
            leaf->label = majority;
            return leaf;
        }

        double bestScore = -1e9;
        int bestAttr = -1;
        for (int i = 0; i < numAttributes; ++i) {
            double score = 0;
            if (method == "IG") score = infoGain(data, i);
            else if (method == "IGR") score = gainRatio(data, i);
            else if (method == "NWIG") score = NWIG(data, i);
            if (score > bestScore) {
                bestScore = score;
                bestAttr = i;
            }
        }
        if (bestAttr == -1) {
            Node *leaf = new Node();
            nodeCount++;
            leaf->isLeaf = true;
            leaf->label = majority;
            return leaf;
        }

        Node *node = new Node();
        nodeCount++;
        node->splitAttr = bestAttr;

        map<string, vector<DataPoint>> splits;
        for (auto &d : data)
            splits[d.features[bestAttr]].push_back(d);
        if (splits.size() <= 1)
        {
            Node *leaf = new Node();
            nodeCount++;
            leaf->isLeaf = true;
            leaf->label = majority;
            return leaf;
        }

        for (auto it = splits.begin(); it != splits.end(); ++it)
            node->children[it->first] = buildTree(it->second, depth + 1, maxDepth, method);

        return node;
    }

    string classify(Node *node, const DataPoint &d) {
        while (!node->isLeaf) {
            string val = d.features[node->splitAttr];
            if (node->children.count(val))
                node = node->children[val];
            else
                return "Invalid";
        }
        return node->label;
    }

    double evaluate(Node *root, const vector<DataPoint> &test) {
        int correct = 0;
        for (auto &d : test)
            if (classify(root, d) == d.label)
                correct++;
        return (double)correct / test.size();
    }
};
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <method: IG|IGR|NWIG> <maxDepth> \n";
        return 1;
    }
    string datasetName ="iris_categorical.csv";
    string method = argv[1];
    int maxDepth = stoi(argv[2]);
    DecisionTree tree;
    tree.loadCSV(datasetName);
    // tree.loadCSV("Preprocessed_adult.csv");

    double avg_acc = tree.run(method, maxDepth);
    cout<<"Average accuracy: " << avg_acc << "%\n";
    // int arr[5] = {5,10,13,15,20};
    // for(int i = 0; i < 5; i++) {
    //     double avg_acc = tree.run("IG", arr[i]);
    //     cout << "Iris" << "," << "IG" << "," << arr[i] << "," << avg_acc << "\n";
    // }
    // for(int i = 0; i < 5; i++) {
    //     double avg_acc = tree.run("IGR", arr[i]);
    //     cout << "Iris" << "," << "IGR" << "," << arr[i] << "," << avg_acc << "\n";
    // }
    // for(int i = 0; i < 5; i++) {
    //     double avg_acc = tree.run("NWIG", arr[i]);
    //     cout << "Iris" << "," << "NWIG" << "," << arr[i] << "," << avg_acc << "\n";
    // }
    //     for(int i = 0; i <5; i++) {
    //     double avg_acc = tree.run("IG", arr[i]);
    //     // cout << "Adult" << "," << "IG" << "," << arr[i] << "," << avg_acc << "\n";
    // }
    // for(int i = 0; i < 5; i++) {
    //     double avg_acc = tree.run("IGR", arr[i]);
    //     // cout << "Adult" << "," << "IGR" << "," << arr[i] << "," << avg_acc << "\n";
    // }
    // for(int i = 0; i < 5; i++) {
    //     double avg_acc = tree.run("NWIG", arr[i]);
    //     // cout << "Adult" << "," << "NWIG" << "," << arr[i] << "," << avg_acc << "\n";
    // }
    return 0;
}
