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

    INPUT is a path to the input file.

    If -o is not specified, the output file is generated automatically
    in the same directory:
        * Encoding appends '.huf' to the input filename.
        * Decoding removes '.huf' from the input filename.

OPTIONS
    -e
        Encode INPUT into compressed form.

    -d
        Decode INPUT from compressed form.

    -o OUTPUT
        Write result to OUTPUT file instead of generating the name automatically.

    --help
        Display this help and exit.

BEHAVIOR
    - Only one of -e or -d can be used at a time.
    - If multiple modes are provided, the program exits with error.
    - If INPUT is missing, the program exits with error.
    - Decoding requires the input file to have a '.huf' extension.

EXIT STATUS
    0    success
    1    invalid arguments or runtime error

EXAMPLES
    Encode file (creates input.txt.huf):
        huff -e input.txt

    Decode file (creates restored.txt):
        huff -d archive.huf -o restored.txt

    Decode file automatically (creates archive):
        huff -d archive.huf

NOTES
    This is a simple implementation of Huffman coding.
    Binary input/output is always used.

)";
}

enum class Mode {
    None,
    Encode,
    Decode
};

bool hasHufExtension(const std::filesystem::path& path) {
    return path.extension() == ".huf";
}

std::filesystem::path withHufExtension(const std::filesystem::path& path) {
    if (hasHufExtension(path)) {
        return path;
    }

    return std::filesystem::path(path.string() + ".huf");
}

std::filesystem::path decodedDefaultOutputPath(std::filesystem::path inputPath) {
    inputPath.replace_extension();
    return inputPath;
}

struct Config {
    Mode mode = Mode::None;
    const char* input = nullptr;
    const char* output = nullptr;
    bool help = false;
    bool force = false;
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
        } else if (strcmp(arg, "-d") == 0) {
            if (cfg.mode != Mode::None) return false;
            cfg.mode = Mode::Decode;
        } else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--force") == 0) {
            cfg.force = true;
        } else if (strcmp(arg, "-o") == 0) {
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

    if (cfg.mode == Mode::Decode && !hasHufExtension(cfg.input)) {
        std::cerr << "Decode input file must have .huf extension\n";
        return 1;
    }

    std::ifstream in(cfg.input, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open input file: " << cfg.input << "\n";
        return 1;
    }
    in.exceptions(std::ios::badbit);

    std::ofstream outFile;
    std::ostream* out = &std::cout;
    std::filesystem::path outputPath;

    if (cfg.output != nullptr) {
        outputPath = cfg.output;

        if (cfg.mode == Mode::Encode) {
            outputPath = withHufExtension(outputPath);
        }
    } else if (cfg.mode == Mode::Encode) {
        outputPath = withHufExtension(cfg.input);
    } else if (cfg.mode == Mode::Decode) {
        outputPath = decodedDefaultOutputPath(cfg.input);
    }

    if (!outputPath.empty()) {
        outFile.open(outputPath, std::ios::binary);

        if (!outFile) {
            std::cerr << "Cannot open output file: " << outputPath << "\n";
            return 1;
        }

        outFile.exceptions(std::ios::failbit | std::ios::badbit);
        out = &outFile;
    }

    try {
        switch (cfg.mode) {
            case Mode::Encode:
                encodeFile(in, *out, cfg.force);
                break;

            case Mode::Decode:
                decodeFile(in, *out);
                break;

            default:
                return 1;
        }
    } catch (const CompressionIneffective& e) {
        if (outFile.is_open()) {
            outFile.close();
        }

        if (!outputPath.empty()) {
            std::error_code ec;
            std::filesystem::remove(outputPath, ec);
        }

        std::cerr << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
