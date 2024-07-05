#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <sstream>


const std::map<int, std::string> flags = {{-1, "none"}, {0, "-c"}, {1, "-l"}, {2, "-w"}, {3, "-m"}};

void count_characters(std::istream &input, std::vector<unsigned long>& size_outputs) {
    size_t count = std::distance(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    input.clear();
    input.seekg(0, std::ios::beg);
    size_outputs.push_back(count);
}

void count_lines(std::istream &input, std::vector<unsigned long>& size_outputs) {
    std::string line;
    int number_of_lines = 0;
    while (std::getline(input, line)) {
        number_of_lines++;
    }
    input.clear();
    input.seekg(0, std::ios::beg);
    size_outputs.push_back(number_of_lines);
}

void count_words(std::istream &input, std::vector<unsigned long>& size_outputs) {
    std::istream_iterator<std::string> in{ input }, end;
    long count = std::distance(in, end);
    input.clear();
    input.seekg(0, std::ios::beg);
    size_outputs.push_back(count);
}

void count_utf8_characters(std::istream &input, std::vector<unsigned long>& size_outputs) {
    std::string file_content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    std::u8string u8_file_content(file_content.begin(), file_content.end());

    size_t char_count = 0;
    for (auto it = u8_file_content.begin(); it != u8_file_content.end(); ++char_count) {
        char8_t c = *it;
        if ((c & 0b10000000) == 0) {
            ++it; // 1-byte character (ASCII)
        } else if ((c & 0b11100000) == 0b11000000) {
            std::advance(it, 2); // 2-byte character
        } else if ((c & 0b11110000) == 0b11100000) {
            std::advance(it, 3); // 3-byte character
        } else if ((c & 0b11111000) == 0b11110000) {
            std::advance(it, 4); // 4-byte character
        } else {
            std::cerr << "Invalid UTF-8 encoding" << std::endl;
            return;
        }
    }
    input.clear();
    input.seekg(0, std::ios::beg);
    size_outputs.push_back(char_count);
}

int main(int argc, char *argv[]) {
    std::string flag {"none"};
    std::string file_name;
    std::vector<unsigned long> size_outputs;

    bool we_have_file {false};

    std::istream *input_stream = &std::cin;
    std::ifstream file_input;
    std::stringstream buffer;

    if (argc == 2) {
        if (strlen(argv[1]) == 2) {
            flag = argv[1];
        } else {
            file_name = argv[1];
            if (std::filesystem::exists(file_name)) {
                we_have_file = true;
            }
        }
    }
    if (argc == 3) {
        we_have_file = true;
        flag = argv[1];
        file_name = argv[2];
    }

    if (we_have_file) {
        file_input.open(file_name, std::ios::binary);
        if (!file_input.is_open()) {
            std::cerr << "Could not open file: " << file_name << std::endl;
            return 1;
        }
        input_stream = &file_input;
    } else {
        buffer << std::cin.rdbuf();
        input_stream = &buffer;
    }

    for (const auto& mapped_flag: flags) {
        if (mapped_flag.second == flag) {
            switch (mapped_flag.first) {
                case 0: {
                    count_characters(*input_stream, size_outputs);
                    break;
                }
                case 1: {
                    count_lines(*input_stream, size_outputs);
                    break;
                }
                case 2: {
                    count_words(*input_stream, size_outputs);
                    break;
                }
                case 3: {
                    count_utf8_characters(*input_stream, size_outputs);
                    break;
                }
                default:
                    count_lines(*input_stream, size_outputs);
                    count_words(*input_stream, size_outputs);
                    count_characters(*input_stream, size_outputs);
            }
        }
    }

    if (!size_outputs.empty()) {
        for (auto output: size_outputs) {
            std::cout << output << " ";
        }
        std::cout << file_name << std::endl;
        return 0;
    }

    std::cout << "Bad amount of arguments" << std::endl;
    return 1;
}
