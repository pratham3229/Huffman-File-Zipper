#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <bitset>

class HuffmanCoding {
public:
    HuffmanCoding(const std::string& path) : path(path) {}

    // Functions for compression:
    std::string compress() {
        std::string output_path = path.substr(0, path.find_last_of('.')) + ".bin";

        std::ifstream input(path);
        std::ofstream output(output_path, std::ios::binary);

        std::string text;
        std::getline(input, text, '\0');

        auto frequency = make_frequency_dict(text);
        make_heap(frequency);
        merge_nodes();
        make_codes();

        std::string encoded_text = get_encoded_text(text);
        std::string padded_encoded_text = pad_encoded_text(encoded_text);
        auto byte_array = get_byte_array(padded_encoded_text);

        output.write(reinterpret_cast<const char*>(&byte_array[0]), byte_array.size());

        std::cout << "Compressed" << std::endl;
        return output_path;
    }

    // Functions for decompression:
    std::string decompress(const std::string& input_path) {
        std::string output_path = path.substr(0, path.find_last_of('.')) + "_decompressed.txt";

        std::ifstream input(input_path, std::ios::binary);
        std::ofstream output(output_path);

        std::string bit_string;
        char byte;
        while (input.read(&byte, 1)) {
            bit_string += std::bitset<8>(byte).to_string();
        }

        std::string encoded_text = remove_padding(bit_string);
        std::string decompressed_text = decode_text(encoded_text);

        output << decompressed_text;

        std::cout << "Decompressed" << std::endl;
        return output_path;
    }

private:
    struct HeapNode {
        char char;
        int freq;
        HeapNode* left;
        HeapNode* right;

        HeapNode(char c, int f) : char(c), freq(f), left(nullptr), right(nullptr) {}

        bool operator>(const HeapNode& other) const {
            return freq > other.freq;
        }
    };

    std::string path;
    std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> heap;
    std::unordered_map<char, std::string> codes;
    std::unordered_map<std::string, char> reverse_mapping;

    std::unordered_map<char, int> make_frequency_dict(const std::string& text) {
        std::unordered_map<char, int> frequency;
        for (char character : text) {
            frequency[character]++;
        }
        return frequency;
    }

    void make_heap(const std::unordered_map<char, int>& frequency) {
        for (const auto& pair : frequency) {
            heap.emplace(pair.first, pair.second);
        }
    }

    void merge_nodes() {
        while (heap.size() > 1) {
            HeapNode* node1 = new HeapNode(heap.top());
            heap.pop();
            HeapNode* node2 = new HeapNode(heap.top());
            heap.pop();

            HeapNode* merged = new HeapNode('\0', node1->freq + node2->freq);
            merged->left = node1;
            merged->right = node2;

            heap.push(*merged);
        }
    }

    void make_codes_helper(HeapNode* root, const std::string& current_code) {
        if (!root) return;

        if (root->char != '\0') {
            codes[root->char] = current_code;
            reverse_mapping[current_code] = root->char;
        }

        make_codes_helper(root->left, current_code + "0");
        make_codes_helper(root->right, current_code + "1");
    }

    void make_codes() {
        HeapNode root = heap.top();
        heap.pop();
        make_codes_helper(&root, "");
    }

    std::string get_encoded_text(const std::string& text) {
        std::string encoded_text;
        for (char character : text) {
            encoded_text += codes[character];
        }
        return encoded_text;
    }

    std::string pad_encoded_text(const std::string& encoded_text) {
        int extra_padding = 8 - (encoded_text.size() % 8);
        std::string padded_encoded_text = encoded_text + std::string(extra_padding, '0');
        std::bitset<8> padded_info(extra_padding);
        return padded_info.to_string() + padded_encoded_text;
    }

    std::vector<uint8_t> get_byte_array(const std::string& padded_encoded_text) {
        if (padded_encoded_text.size() % 8 != 0) {
            std::cerr << "Encoded text not padded properly" << std::endl;
            exit(1);
        }

        std::vector<uint8_t> byte_array;
        for (size_t i = 0; i < padded_encoded_text.size(); i += 8) {
            byte_array.push_back(static_cast<uint8_t>(std::bitset<8>(padded_encoded_text.substr(i, 8)).to_ulong()));
        }
        return byte_array;
    }

    std::string remove_padding(const std::string& padded_encoded_text) {
        std::bitset<8> padded_info(padded_encoded_text.substr(0, 8));
        int extra_padding = static_cast<int>(padded_info.to_ulong());
        return padded_encoded_text.substr(8, padded_encoded_text.size() - 8 - extra_padding);
    }

    std::string decode_text(const std::string& encoded_text) {
        std::string current_code;
        std::string decoded_text;
        for (char bit : encoded_text) {
            current_code += bit;
            if (reverse_mapping.find(current_code) != reverse_mapping.end()) {
                decoded_text += reverse_mapping[current_code];
                current_code.clear();
            }
        }
        return decoded_text;
    }
};
