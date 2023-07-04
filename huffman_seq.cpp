//
// Pasquale Esposito, no. 649153
// Sequential code for the Huffman Coding
//
#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <bitset>
#include <cstring>
#include "huffman_alg.hpp"
#include "utimer.hpp"

using namespace std;

long total, totalNoIO, usecs;
bool flag;

// Print the time taken by the program
void print_time() {
    cout << endl;
    cout << "Total time: " << setw(15) << total << " usec" << endl;
    cout << "Total time (no IO): " << setw(15) << totalNoIO << " usec" << endl;
}

// Count the characters in the buffer and update the frequency table
void countCharacters(string buffer, unordered_map<char, int>& frequencyTable) {
    for (int i = 0; i < buffer.size(); i++)
        ++frequencyTable[buffer[i]];
}

// Compress the file
string encodeString(string source, unordered_map<char, string> huffmanCode) {
    string binary, encodedString;

    //Map the input string from ASCII to Huffman code
    for(char c: source)
        binary += huffmanCode[c];

    // Add padding 0s
    if(binary.size() % 8 != 0) {
        int padding = 8 - binary.size() % 8;
        binary += string(padding, '0');
    }

    // Transform the binary string to an ASCII string
    for (size_t i = 0; i < binary.size(); i += 8) {
        bitset<8> bits(binary.substr(i, 8));
        char c = static_cast<char>(bits.to_ulong());
        encodedString += c;
    }

    return encodedString;
}

int main(int argc, char* argv[]) {

    unordered_map<char, int> frequencyTable;
    unordered_map<char, string> huffmanCode;
    Node* root;
    string content, encodedString;

    if(argc < 2) {
        cout << "Usage: <input_file> -v" << endl;
        return -1;
    }

    string inputFileName = argv[1];
    if(argc == 3 && !strcmp(argv[2], "-v"))
        flag = true;
    else
        flag = false;

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
            utimer timer1("Total time no IO", &totalNoIO);
            {
                utimer timer2("Counting the characters", &usecs);
                countCharacters(content, frequencyTable);
            }
            {
                utimer timer2("Building the Huffman tree and the codes", &usecs);
                root = buildHuffmanTree(frequencyTable);
                buildHuffmanCodes(root, "", huffmanCode);
            }
            {
                utimer timer2("Compressing the file", &usecs);
                encodedString = encodeString(content, huffmanCode);
            }
            {
                utimer timer2("Writing the compressed file", &usecs);
                ofstream output("output/compressed_seq_" + inputFileName);
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