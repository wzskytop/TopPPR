#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "ppr.h"
#include <unordered_set>    
#include <cstdlib>
#include <cstring>


void usage() {
    cerr << "AdaptivePPR [-d <dataset>] [-e epsilon] [-w walknum] [-k topk (default 500)] [-n node_count (default 20)] [-r error_rate (default 1)] [-err error_eps (default 0)] [-c bound_err (default 0.5)]" << endl;
}

int check_inc(int i, int max) {
    if (i == max) {
        usage();
        exit(1);
    }
    return i + 1;
}

bool maxCmp(const pair<int, double>& a, const pair<int, double>& b){
    return a.second > b.second;
}

vector<int> getRealTopK(int s, int k, string target_filename, int vert){
    stringstream ss;
    ss << "ppr-answer/" << target_filename << "/" << s << ".txt";
    string infile = ss.str();
    ifstream real(infile);
    vector<int> realList;
    vector<double> simList;
    for(int i = 0; i < vert; i++){
        int tempId; 
        double tempSim;
        real >> tempId >> tempSim;
        if(i >= k && tempSim < simList[k-1]){
           break; 
        } 
        realList.push_back(tempId);
        simList.push_back(tempSim);
    }
    real.close();
    return realList;
}

unordered_map<int, double> getRealTopKMap(int s, int k, string target_filename, int vert){
    unordered_map<int, double> answer_map;
    stringstream ss;
    ss << "ppr-answer/" << target_filename << "/" << s << ".txt";
    string infile = ss.str();
    ifstream real(infile);
    double k_Sim = 0;
    for(int i = 0; i < vert; i++){
        int tempId;
        double tempSim;
        real >> tempId >> tempSim;
        if(i == k - 1){
            k_Sim = tempSim;
        }
        if(i >= k && tempSim < k_Sim){
            break;
        }
        answer_map[tempId] = tempSim;
    }
    real.close();
    return answer_map;
}

int main(int argc, char *argv[]){
    int i = 1;
    char *endptr;
    string filename;
    double eps;                    //forward adaptive parameter
    int k = 500;                   //parameter k
    double alpha = 0.2;            //decay factor
    int node_count = 20;           //query node size
    double error_rate = 1;         //precision parameter
    double error_eps = 0;          //min error
    double bound_c = 0.5;          //backward search shrink parameter
    double forward_c = 1;          //forward search shrink parameter
    string algo = "TopPPR";
    if(argc < 7){
        usage();
        exit(1);
    }
    while (i < argc) {
        if (!strcmp(argv[i], "-d")) {
            i = check_inc(i, argc);
            filename = argv[i];
        } 
        else if (!strcmp(argv[i], "-algo")) {
            i = check_inc(i, argc);
            algo = argv[i];
        }
        else if (!strcmp(argv[i], "-e")) {
            i = check_inc(i, argc);
            eps = strtod(argv[i], &endptr);
            if ((eps == 0 || eps > 1) && endptr) {
                cerr << "Invalid eps argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-k")) {
            i = check_inc(i, argc);
            k = strtod(argv[i], &endptr);
            if ((k < 0) && endptr) {
                cerr << "Invalid k argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-n")) {
            i = check_inc(i, argc);
            node_count = strtod(argv[i], &endptr);
            if ((node_count < 0) && endptr) {
                cerr << "Invalid node_count argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-r")) {
            i = check_inc(i, argc);
            error_rate = strtod(argv[i], &endptr);
            if (((error_rate < 0) || (error_rate > 1)) && endptr) {
                cerr << "Invalid error_rate argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-err")) {
            i = check_inc(i, argc);
            error_eps = strtod(argv[i], &endptr);
            if (((error_eps < 0) || (error_eps > 1)) && endptr) {
                cerr << "Invalid error_eps argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-c")) {
            i = check_inc(i, argc);
            bound_c = strtod(argv[i], &endptr);
            if ((bound_c < 0) && endptr) {
                cerr << "Invalid bound_c argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-fc")) {
            i = check_inc(i, argc);
            forward_c = strtod(argv[i], &endptr);
            if (((forward_c < 0) || (forward_c > 1)) && endptr) {
                cerr << "Invalid forward_c argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-a")) {
            i = check_inc(i, argc);
            alpha = strtod(argv[i], &endptr);
            if (((alpha < 0) || (alpha > 1)) && endptr) {
                cerr << "Invalid alpha argument" << endl;
                exit(1);
            }
        }
        else {
            usage();
            exit(1);
        }
        i++;
    }
    
    PPR ppr = PPR(filename, error_rate, error_eps, bound_c, forward_c, k, alpha);
    if(algo == "GEN_QUERY"){
        ofstream outFile("dataset/" + filename + ".query");
        ppr.generateQueryNode(node_count, outFile);
        outFile.close(); 
    }
    else if(algo == "GEN_GROUND_TRUTH"){
        ppr.PowerMethodMulti(100, node_count, 10);/*  多线程PowerMethparameter: iteration loops, node size, thread num */
    }
    else{
        ifstream nodes_file("dataset/" + filename + ".query");
        vector<int> test_nodes;
        while(!nodes_file.eof()){
            int temp_node;
            nodes_file >> temp_node;
            test_nodes.push_back(temp_node);
        }
        cout << "read done!" << endl;
        for(int para = 0; para < 1; para++){
            if(k > ppr.g.n){
                break;
            }
            int realCount = 0;
            for(int t = 0; t < node_count; t++){
                int test_node = test_nodes[realCount++];
                cout << "node: " << test_node << " " << eps << " " << k << endl;
                vector<int> realList = ppr.getRealTopK(test_node, k+1);
                unordered_map<int, double> realMap = ppr.getRealTopKMap(test_node, k+1);
                if(ppr.g.getOutSize(test_node) == 0){
                    t--;
                    continue;
                }
                if(realMap[realList[k - 1]] < 0.0000000001){
                    t--;
                    continue;
                }
                if(realMap[realList[k - 1]] > 0.0000000001 && realMap[realList[k - 1]] - realMap[realList[k]] < 0.0000000001){
                    t--;
                    cout << "gap too small!" << endl;
                    continue;
                }
                double real_eps = eps * 80 * sqrt(1/ (double) ppr.g.m / (double) ppr.g.n / log(ppr.g.n)) / (double) log(k);
                //TopPPR算法
                double* resultList = ppr.TopPPR(test_node, real_eps, k, outputData, outputData2);     
            }
            cout << "avg precision: " << ppr.avg_pre / (double) node_count << endl;
            cout << "avg time: " << ppr.avg_time / (double) node_count << endl;
            ppr.avg_time = 0;   
            ppr.avg_pre = 0; 
        }
        for(int i = 0; i < argc; i++){
            cout << argv[i] << " ";
        }
        cout << endl;
    }
    return 0;
};
