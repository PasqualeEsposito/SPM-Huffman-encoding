//
// Pasquale Esposito, no. 649153
// Thread version for the Huffman Coding
//
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <thread>
#include <vector>
#include <bitset>
#include <cstring>
#include <mutex>
#include "utimer.hpp"
#include "huffman_alg.hpp"

using namespace std;

long total, totalNoIO, usecs;
bool flag;
mutex m;

// Print the time taken by the program
void print_time() {
    cout << endl;
    cout << "Total time: " << setw(15) << total << " usec" << endl;
    cout << "Total time (no IO): " << setw(15) << totalNoIO << " usec" << endl;
}

// Worker that counts the symbols' frequencies
void countSymbols(const string& chunk, unordered_map<char, int>& symbolCount) {
    unordered_map<char, int> localSymbolCount;

    for (char c: chunk)
        localSymbolCount[c]++;

    m.lock();
    for (auto &pair: localSymbolCount)
        symbolCount[pair.first] += pair.second;
    m.unlock();
}

// Split the input string among the workers
void mapFrequencies(string input, int nw, unordered_map<char, int>& freqTable) {
    int length = input.size();
    int chunkSize = length / nw;
    vector<thread> threads;

    for(int i = 0; i < nw; i++) {
        int start = i * chunkSize;
        int end = (i == nw - 1 ? length : (i + 1) * chunkSize);

        string chunk = input.substr(start, end - start);
        threads.emplace_back(countSymbols, chunk, ref(freqTable));
    }

    for(auto& tids: threads)
        tids.join();
}

// Worker that transforms the input string to a binary string
void encodeStringWorker(string toEncode, unordered_map<char, string> huffmanCode, string& tempEncodedString) {
    string binary;

    for (auto &c: toEncode)
        binary += huffmanCode[c];

    tempEncodedString = binary;
}

// Split the input string among the workers
string encodeString(string source, int nw, unordered_map<char, string> huffmanCode) {
    string encodedString;
    vector<string> tempEncodedString(nw);

    int length = source.size();
    int chunkSize = length / nw;
    vector<thread> threads;

    for(int i = 0; i < nw; i++) {
        int start = i * chunkSize;
        int end = (i == nw - 1 ? length : (i + 1) * chunkSize);

        threads.emplace_back(encodeStringWorker, source.substr(start, end - start), ref(huffmanCode), ref(tempEncodedString[i]));
    }

    for(auto& tid: threads)
        tid.join();

    for(auto& s: tempEncodedString)
        encodedString += s;

    if(encodedString.size() % 8 != 0) {
        int padding = 8 - encodedString.size() % 8;
        encodedString += string(padding, '0');
    }

    return encodedString;
}

// Worker that transforms the binary string to an ASCII string
void encodeToASCIIWorker(string encodedString, string& tempEncodedString) {
    string encodedASCII;

    for (int i = 0; i < encodedString.size(); i += 8) {
        string byte = encodedString.substr(i, 8);
        encodedASCII += bitset<8>(byte).to_ulong();
    }

    tempEncodedString = encodedASCII;
}


// Split the binary string among the workers
string mapEncodeASCII(string encodedString, int nw) {
    string encodedASCII;
    vector<thread> threads;
    vector<string> tempEncodedString(nw);

    int length = encodedString.size();
    int chunkSize = length / nw;

    if(chunkSize % 8 != 0)
        chunkSize += 8 - chunkSize % 8;

    for(int i = 0; i < nw; i++) {
        int start = i * chunkSize;
        int end = (i == nw - 1 ? length : (i + 1) * chunkSize);

        threads.emplace_back(encodeToASCIIWorker, encodedString.substr(start, end - start), ref(tempEncodedString[i]));
    }

    for(auto& tid: threads)
        tid.join();

    for(auto& s: tempEncodedString)
        encodedASCII += s;

    return encodedASCII;
}

int main(int argc, char** argv) {
    if(argc < 3) {
        cout << "Usage: <input_file> <number_threads> -v" << endl;
        return -1;
    }

    string inputFileName = argv[1];
    int nw = (argc > 1 ? stoi(argv[2]) : 4);
    if(argc == 4 && !strcmp(argv[3], "-v"))
        flag = true;
    else
        flag = false;

    string content, encodedString;
    unordered_map<char, int> frequencyTable;
    unordered_map<char, string> huffmanCode;
    Node* root;

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
                mapFrequencies(content, nw, frequencyTable);
            }
            {
                utimer timer2("Building the Huffman tree and the codes", &usecs);
                root = buildHuffmanTree(frequencyTable);
                buildHuffmanCodes(root, "", huffmanCode);
            }
            {
                utimer timer2("Compressing the file", &usecs);
                {
                    utimer timer4("Binary encoding phase");
                    encodedString = encodeString(content, nw, huffmanCode);
                }
                {
                    utimer timer4("ASCII encoding phase");
                    encodedString = mapEncodeASCII(encodedString, nw);
                }
            }
            {
                utimer timer2("Writing the compressed file", &usecs);
                ofstream output("output/compressed_thread_" + inputFileName);
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