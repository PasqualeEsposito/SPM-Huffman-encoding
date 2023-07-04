//
// Created by espos on 09/06/2023.
//
#ifndef SPM_HUFFMAN_NODE_H
#define SPM_HUFFMAN_NODE_H

#endif //SPM_HUFFMAN_NODE_H

#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

using namespace std;

char decToASCII(int decimalValue) {
    return static_cast<char>(decimalValue);
}

// Structure for representing a Huffman tree node
struct Node {
    char symbol;
    int frequency;
    Node* left;
    Node* right;

    Node(char symbol, int frequency): symbol(symbol), frequency(frequency), left(nullptr), right(nullptr) {}

    ~Node() {
        delete left;
        delete right;
    }
};

// Function to compare two nodes for priority queue ordering
struct CompareNodes {
    bool operator()(const Node* lhs, const Node* rhs) const {
        return lhs->frequency > rhs->frequency;
    }
};

// Function to delete the Huffman tree
void freeTree(Node* root) {
    if (root == nullptr)
        return;

    freeTree(root->left);
    freeTree(root->right);

    free(root);
}

// Function to build the Huffman tree
Node* buildHuffmanTree(const unordered_map<char, int>& frequencyTable) {
    //in the angle bracket, the first parameter is used to tell the type of the single element, the second parameter to
    //say the type of the container and the third parameter is the comparator used for the priority
    priority_queue<Node*, vector<Node*>, CompareNodes> pq;

    // Create a leaf node for each symbol and add it to the priority queue
    for (const auto& entry : frequencyTable) {
        pq.push(new Node(decToASCII(entry.first), entry.second));
    }

    // Build the Huffman tree by combining the least frequent nodes
    //there cannot be the case there is only one element in the queue, because the last element is the root of the tree
    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();

        Node* parent = new Node('\0', left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;

        pq.push(parent);
    }

    return pq.top();
}

// Function to traverse the Huffman tree and build the Huffman codes
void buildHuffmanCodes(Node* root, const string& code, unordered_map<char, string>& huffmanCode) {
    if (root->left == nullptr && root->right == nullptr) {
        huffmanCode[root->symbol] = code;
        return;
    }

    buildHuffmanCodes(root->left, code + "0", huffmanCode);
    buildHuffmanCodes(root->right, code + "1", huffmanCode);
}
