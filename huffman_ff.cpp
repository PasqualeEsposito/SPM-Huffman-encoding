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
                        localSymbolCount[content[j]]++;

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

    return 0;
}