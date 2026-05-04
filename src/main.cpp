#include <cstddef>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "Encoder.h"
#include "Decoder.h"
#include "Exceptions.h"

void help() {
    std::cout << "Usage: huff < -e > (< -d >) < input > -o < output > \n";
    std::cout << "flags:\n";

    std::cout << "-e : " << std::setw(4) << "encode provided file\n";
    std::cout << "-c : " << std::setw(4) << "encode only if compressed file is smaller than input\n";
    std::cout << "-d : " << std::setw(4) << "decode provided file\n";
    std::cout << "-o : " << std::setw(4) << "set output file\n\twithout this flag programm output will be pushed directly to the terminal window\n";
}

int main(int argc, char* argv[]) {
    try {
        if (argc > 5 || argc == 0) {
            throw UsageException();
        }
    } catch (const UsageException& ex) {
        std::cerr << ex.what() << "\n";
    }

    std::string currentCommand = argv[1];
    if (currentCommand == "--help") {
        help();
    } else if (currentCommand == "-e") {
        std::ifstream in(argv[2], std::ios::binary);
        std::ofstream out(argv[3], std::ios::binary);

        if (!in) {
            std::cerr << "Cannot open input file: " << argv[1] << "\n";
            return 1;
        }

        if (!out) {
            std::cerr << "Cannot open output file: " << argv[2] << "\n";
            return 1;
        }

        try {
            encodeFile(in, out);
        } catch (const EmptyInputFile& ex) {
            std::cerr << ex.what() << "\n";
            return 1;
        }
    } else if (currentCommand == "-c") {
        std::ifstream in(argv[2], std::ios::binary);

        if (!in) {
            std::cerr << "Cannot open input file: " << argv[1] << "\n";
            return 1;
        }

        std::ofstream out(argv[3], std::ios::binary);

        if (!out) {
            std::cerr << "Cannot open output file: " << argv[2] << "\n";
            return 1;
        }

        try {
            carefulEncodeFile(in, out);
        } catch (const EmptyInputFile& ex) {
            out.close();
            std::filesystem::remove(argv[3]);
            std::cerr << ex.what() << "\n";
            return 1;
        } catch (const CompressionIneffective& ex) {
            out.close();
            std::filesystem::remove(argv[3]);
            std::cerr << ex.what() << "\n";
            return 1;
        }
    } else if (currentCommand == "-d") {
        std::ifstream in(argv[2], std::ios::binary);
        std::ofstream out(argv[3], std::ios::binary);

        if (!in) {
            std::cerr << "Cannot open input file: " << argv[1] << "\n";
            return 1;
        }

        if (!out) {
            std::cerr << "Cannot open output file: " << argv[2] << "\n";
            return 1;
        }

        decodeFile(in, out);
    }

    return 0;
}
