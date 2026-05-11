#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>

#include "Encoder.h"
#include "Decoder.h"
#include "Exceptions.h"

#include <iostream>

void help() {
    std::cout <<
R"(HUFF(1)                         User Commands                        HUFF(1)

NAME
    huff - file compressor/decompressor based on Huffman coding

SYNOPSIS
    huff [OPTIONS] INPUT [-o OUTPUT]

DESCRIPTION
    huff encodes or decodes files using Huffman coding.

    Exactly one mode must be specified:
        -e    encode input file
        -d    decode input file
        -c    encode only if result is smaller than input

    INPUT is a path to the input file.

    If -o is not specified, output is written to standard output.

OPTIONS
    -e
        Encode INPUT into compressed form.

    -d
        Decode INPUT from compressed form.

    -c
        Encode INPUT only if compressed size is smaller than original.
        Otherwise no output file is produced.

    -o OUTPUT
        Write result to OUTPUT file instead of standard output.

    --help
        Display this help and exit.

BEHAVIOR
    - Only one of -e, -d, -c can be used at a time.
    - If multiple modes are provided, the program exits with error.
    - If INPUT is missing, the program exits with error.
    - If OUTPUT is not specified, result is written to stdout.

EXIT STATUS
    0    success
    1    invalid arguments or runtime error

EXAMPLES
    Encode file:
        huff -e input.txt -o output.huff

    Decode file:
        huff -d output.huff -o restored.txt

    Encode only if beneficial:
        huff -c input.txt -o output.huff

    Write to stdout:
        huff -e input.txt

NOTES
    This is a simple implementation of Huffman coding.
    Binary input/output is always used.

)";
}

enum class Mode {
    None,
    Encode,
    Decode,
    EncodeCareful
};

struct Config {
    Mode mode = Mode::None;
    const char* input = nullptr;
    const char* output = nullptr;
    bool help = false;
};

bool parseArgs(int argc, char* argv[], Config& cfg) {
    if (argc < 2) {
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];

        // help
        if (strcmp(arg, "--help") == 0) {
            cfg.help = true;
            return true;
        }

        else if (strcmp(arg, "-e") == 0) {
            if (cfg.mode != Mode::None) return false;
            cfg.mode = Mode::Encode;
        }
        else if (strcmp(arg, "-d") == 0) {
            if (cfg.mode != Mode::None) return false;
            cfg.mode = Mode::Decode;
        }
        else if (strcmp(arg, "-c") == 0) {
            if (cfg.mode != Mode::None) return false;
            cfg.mode = Mode::EncodeCareful;
        }

        // output
        else if (strcmp(arg, "-o") == 0) {
            if (i + 1 >= argc) return false;
            cfg.output = argv[++i];
        }

        // input
        else {
            if (arg[0] == '-') {
                return false;
            }

            if (cfg.input != nullptr) {
                return false;
            }

            cfg.input = arg;
        }
    }

    if (!cfg.help) {
        if (cfg.mode == Mode::None) return false;
        if (cfg.input == nullptr) return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    Config cfg;

    if (!parseArgs(argc, argv, cfg)) {
        std::cerr << "Invalid arguments\n";
        help();
        return 1;
    }

    if (cfg.help) {
        help();
        return 0;
    }

    std::ifstream in(cfg.input, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open input file: " << cfg.input << "\n";
        return 1;
    }
    in.exceptions(std::ios::badbit);

    std::ofstream outFile;
    std::ostream* out = &std::cout;

    if (cfg.output != nullptr) {
        outFile.open(cfg.output, std::ios::binary);
        if (!outFile) {
            std::cerr << "Cannot open output file: " << cfg.output << "\n";
            return 1;
        }
        outFile.exceptions(std::ios::failbit | std::ios::badbit);
        out = &outFile;
    }

    try {
        switch (cfg.mode) {
            case Mode::Encode:
                encodeFile(in, *out);
                break;

            case Mode::Decode:
                decodeFile(in, *out);
                break;

            case Mode::EncodeCareful:
                carefulEncodeFile(in, *out);
                break;

            default:
                return 1;
        }
    }
    catch (const CompressionIneffective& e) {
        if (cfg.output) {
            outFile.close();
            std::filesystem::remove(cfg.output);
        }
        std::cerr << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
