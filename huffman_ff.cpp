//
// Pasquale Esposito, no. 649153
// Fastflow version for the Huffman Coding
//
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <bitset>
#include <cstring>
#include <mutex>
#include "huffman_alg.hpp"
#include "utimer.hpp"
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>

using namespace std;
using namespace ff;

long total, totalNoIO, usecs;
bool flag;

/*
typedef struct Work {
    int pos;
    string toEncode;

    Work(string str, int pos): toEncode(str), pos(pos) {}
} Work;

class emitter : public ff::ff_monode_t<Work> {
private:
    int nw;
    long int size;
    long int chunk;
public:
    emitter(int nw, long int size):nw(nw), size(size) {
        this->chunk = (size / nw) - round((size / nw) % 8);
        binToASCII.resize(nw);
    }

    Work * svc(Work *) {
        for(int i = 0; i < nw; i++) {
            int from = i * chunk;
            int to = (i == (nw - 1)) ? size : (i + 1) * chunk;  // (poor load balancing)
            Work *t;
            t = new Work(binary.substr(from, to), i);
            ff_send_out(t);
        }
        return(EOS);
    }
};

class collector : public ff::ff_node_t<Work> {
private:
    Work * tt;

public:
    Work * svc(Work * t) {
        free(t);
        return(GO_ON);
    }

};

Work *  worker(Work * t, ff::ff_node* nn) {
    string toEncode = t -> toEncode;
    auto pos = t-> pos;

    bitset<8> bits;
    string res = "";
    for (long int i = 0; i < toEncode.size(); i += 8) {
        bits = bitset<8>(binary.substr(i, 8));
        res += static_cast<char>(bits.to_ulong() & 0xFF);
    }

    binToASCII[pos] = res;

    return t;
}
*/

// Print the time taken by the program
void print_time() {
    cout << endl;
    cout << "Total time: " << setw(15) << total << " usec" << endl;
    cout << "Total time (no IO): " << setw(15) << totalNoIO << " usec" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: <input_file> <number_threads> -v" << endl;
        return -1;
    }

    string inputFileName = argv[1];
    int nw = (argc > 1 ? stoi(argv[2]) : 4);
    if (argc == 4 && !strcmp(argv[3], "-v"))
        flag = true;
    else
        flag = false;

    int chunkSize;
    int length;
    string encodedString;
    string content = "", binary = "";
    vector<string> encode, binToASCII;
    mutex m;
    unordered_map<char, int> frequencyTable;
    unordered_map<char, string> huffmanCode;
    Node *root;

    // number of workers nw, use spinwaits instead of locks -> to use with small grain computations
    ParallelFor pf(nw);

    {
        utimer timer("Total time", &total);
        {
            utimer timer1("Reading the file", &usecs);
            ifstream input("input/" + inputFileName);
            if (!input) {
                cerr << "Error opening input file." << endl;
                return -1;
            } else {
                string temp;
                while (getline(input, temp)) {
                    content += temp;
                }
                input.close();
            }
        }
        {
            utimer timer1("Total time (no IO)", &totalNoIO);
            {
                utimer timer2("Counting the characters", &usecs);

                length = content.size();
                chunkSize = length / nw;
                pf.parallel_for(0, nw, 1, 0, [&](const int i) {
                    unordered_map<char, int> localSymbolCount;
                    int start = i * chunkSize;
                    int end = (i == nw - 1) ? length : start + chunkSize;
                    for (int j = start; j < end; ++j)
                        localSymbolCount[ASCIIToDec(content[j])]++;

                    m.lock();
                    for (auto &pair: localSymbolCount)
                        frequencyTable[pair.first] += pair.second;
                    m.unlock();
                }, nw);
            }
            {
                utimer timer2("Building the Huffman tree and the codes", &usecs);
                root = buildHuffmanTree(frequencyTable);
                buildHuffmanCodes(root, "", huffmanCode);
            }
            {
                utimer timer2("Compressing the file", &usecs);

                vector<string> tempEncodedString(nw);
                length = content.size();
                chunkSize = length / nw;

                pf.parallel_for(0, nw, 1, 0, [&](const int i) {
                    string binary;
                    int start = i * chunkSize;
                    int end = (i == nw - 1 ? length : (i + 1) * chunkSize);

                    for(int j = start; j < end; j++)
                        binary += huffmanCode[content[j]];

                    tempEncodedString[i] = binary;
                }, nw);

                for(auto& s : tempEncodedString)
                    encodedString += s;

                tempEncodedString.clear();
                tempEncodedString.resize(nw);

                if(encodedString.size() % 8 != 0) {
                    int padding = 8 - encodedString.size() % 8;
                    encodedString += string(padding, '0');
                }

                length = encodedString.size();
                chunkSize = length / nw;

                if(chunkSize % 8 != 0)
                    chunkSize += 8 - chunkSize % 8;

                pf.parallel_for(0, nw, 1, 0, [&](const int i) {
                    string tmpResult;
                    int start = i * chunkSize;
                    int end = (i == nw - 1 ? length : (i + 1) * chunkSize);

                    for(int j = start; j < end; j += 8) {
                        string byte = encodedString.substr(j, 8);
                        tmpResult += bitset<8>(byte).to_ulong();
                    }

                    tempEncodedString[i] = tmpResult;
                }, nw);

                encodedString = "";

                for(auto& s : tempEncodedString)
                    encodedString += s;
            }
            {
                utimer timer2("Writing the compressed file", &usecs);
                ofstream output("output/compressed_ff_" + inputFileName);
                if (!output) {
                    cerr << "Error opening output file." << endl;
                    return -1;
                } else {
                    output << encodedString;
                    output.close();
                }
            }
            freeTree(root);
        }
    }

    if(flag)
        print_time();
    //cout << argv[1] << ", " << numThreads << endl;

    ofstream output;

    if(!strcmp(argv[1], "bible1MB.txt"))
        output.open("results/ff_results_1MB.csv", ios::app);
    else if(!strcmp(argv[1], "bible10MB.txt"))
        output.open("results/ff_results_10MB.csv", ios::app);
    else
    if(!strcmp(argv[1], "bible100MB.txt"))
        output.open("results/ff_results_100MB.csv", ios::app);

    string str1 = "Completion time," + to_string(nw) + "," + to_string(total) + "\n";
    string str2 = "Completion time (no IO)," + to_string(nw) + "," + to_string(totalNoIO) + "\n";
    output << str1;
    output << str2;
    output.close();

    return 0;
}






























/*
    if (!input) {
        cout << "Error opening input file." << endl;
        return 1;
    }

    {
        utimer timer("Time spent reading and counting the symbols", &time1);
        content.assign((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

        len = content.size();
        partition = len / numThreads;
        pf.parallel_for(0, numThreads, 1, 0, [&](const int i) {
            int start = i * partition;
            int end = (i == numThreads - 1) ? len : start + partition;
            for (int j = start; j < end; ++j)
                frequencyTable[i][content[j]]++;
        }, numThreads);

        for (auto &map : frequencyTable)
            for (auto &pair : map)
                frequencyMap[pair.first] += pair.second;
    }

    {
        utimer timer("Time spent creating the Huffman Code", &time2);
        //Generates the Huffman Tree and returns its root pointer
        root = buildHuffmanTree(frequencyMap);

        //Encodes each symbol and insert the pair <symbol, code> in the huffmanCode map
        buildHuffmanCodes(root, "", huffmanCode);
    }

    {
        utimer timer("Time spent compressing the file", &time3);
        //Compresses the file
        pf.parallel_for(0, numThreads, 1, 0, [&](const int i) {
            string temp = "";
            int start = i * partition;
            int end = (i == numThreads - 1) ? len : start + partition;
            for (int j = start; j < end; ++j)
                temp += huffmanCode[content[j]];

            encode[i] = temp;
        }, numThreads);

        for (int i = 0; i < numThreads; ++i)
            binary += encode[i];

        encode.clear();

        auto e = emitter(numThreads, binary.size());
        auto c = collector();
        ff_Farm<Work> mf(worker, numThreads);
        mf.add_emitter(e);
        mf.add_collector(c);

        mf.run_and_wait_end();

        for (int i = 0; i < numThreads; ++i)
            output << binToASCII[i];

        output.close();
    }

    delete root;

    print_time();
    //cout << argv[1] << ", " << numThreads << endl;

    ofstream output;

    if(!strcmp(argv[1], "bible1MB.txt"))
        output.open("results/thread_results_1MB.csv", ios::app);
    else if(!strcmp(argv[1], "bible10MB.txt"))
        output.open("results/thread_results_10MB.csv", ios::app);
    else
    if(!strcmp(argv[1], "bible100MB.txt"))
        output.open("results/thread_results_100MB.csv", ios::app);

    string str1 = "Completion time," + to_string(nw) + "," + to_string(total) + "\n";
    string str2 = "Completion time (no IO)," + to_string(nw) + "," + to_string(totalNoIO) + "\n";
    output << str1;
    output << str2;
    output.close();

    return 0;
}

*/

