#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include "../src/Encoder.h"
#include "../src/Decoder.h"
#include "../src/Exceptions.h"

namespace fs = std::filesystem;

static int passed = 0;
static int failed = 0;

void pass(const std::string& name, std::pair<uint64_t, uint64_t> info = {0, 0}) {
    ++passed;

    constexpr int nameWidth = 35;
    uint64_t originalSize = info.first;
    uint64_t archiveSize = info.second;

    std::cout << "[PASS] "
                << std::left << std::setw(nameWidth) << name;

    if (!(archiveSize == 0 && originalSize == 0)) {
        double ratio =
            100.0 * (1.0 -
            static_cast<double>(archiveSize)
            / static_cast<double>(originalSize));

        std::cout << std::right
                    << std::fixed << std::setprecision(2)
                    << ratio << "%";
    }

    std::cout << '\n';
}

void fail(const std::string& name, const std::string& reason) {
    ++failed;
    std::cerr << "[FAIL] " << name
              << " : " << reason << '\n';
}

void assertTrue(bool cond,
                const std::string& testName,
                const std::string& msg) {
    if (!cond) {
        throw std::runtime_error(testName + " : " + msg);
    }
}

std::string readWholeFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);

    return std::string(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    );
}

template<typename Exception, typename Func>
void expectThrow(Func func, const std::string& testName) {
    try {
        func();
        fail(testName, "exception was not thrown");
    } catch (const Exception&) {
        pass(testName);
    } catch (const std::exception& e) {
        fail(testName,
             std::string("wrong exception type: ") + e.what());
    } catch (...) {
        fail(testName, "unknown exception type");
    }
}

template<typename Func>
void expectThrowAny(Func func, const std::string& testName) {
    try {
        func();
        fail(testName, "exception was not thrown");
    } catch (...) {
        pass(testName);
    }
}

void testCorruptedFiles() {
    const fs::path dir = "test/corrupted";

    for (const auto& entry : fs::directory_iterator(dir)) {
        const fs::path path = entry.path();

        expectThrow<InvalidArchive>([&]() {

            std::ifstream in(path, std::ios::binary);

            if (!in) {
                throw std::runtime_error(
                    "cannot open file"
                );
            }

            std::ostringstream out;

            decodeFile(in, out);

        }, path.filename().string());
    }
}

void testIneffectiveCompression() {
    const fs::path dir = "test/input";

    const char* files[] = {
        "empty.txt",
        "one_character.txt",
        "one_character_repeated.txt"
    };

    for (const char* name : files) {
        fs::path path = dir / name;

        expectThrowAny([&]() {

            std::ifstream in(path, std::ios::binary);

            if (!in) {
                throw std::runtime_error(
                    "cannot open file"
                );
            }

            std::ostringstream encoded;

            encodeFile(in, encoded, false);

        }, name);
    }
}

void testEncodeDecode() {
    const fs::path dir = "test/input";

    for (const auto& entry : fs::directory_iterator(dir)) {
        const fs::path path = entry.path();

        try {
            std::ifstream originalInput(
                path,
                std::ios::binary
            );

            if (!originalInput) {
                throw std::runtime_error(
                    "cannot open input file"
                );
            }

            std::stringstream encoded;

            std::pair<uint64_t, uint64_t> info = encodeFile(originalInput,
                    encoded,
                    true);

            encoded.seekg(0);

            std::stringstream decoded;

            decodeFile(encoded, decoded);

            const std::string originalData =
                readWholeFile(path);

            const std::string decodedData =
                decoded.str();

            assertTrue(
                originalData == decodedData,
                path.filename().string(),
                "decoded file differs from original"
            );

            pass(path.filename().string(), info);
        } catch (const CompressionIneffective&) {
            pass(path.filename().string()
                 + " (compression ineffective)");
        } catch (const std::exception& e) {
            fail(path.filename().string(), e.what());
        } catch (...) {
            fail(path.filename().string(),
                 "unknown error");
        }
    }
}

int main() {
    std::cout << "=== Huffman Tests ===\n\n";

    testCorruptedFiles();

    std::cout << '\n';

    testIneffectiveCompression();

    std::cout << '\n';

    testEncodeDecode();

    std::cout << "\n=== Result ===\n";

    std::cout << "Passed: "
              << passed
              << '\n';

    std::cout << "Failed: "
              << failed
              << '\n';

    if (failed == 0) {
        std::cout << "\nALL TESTS PASSED\n";
        return 0;
    }

    return 1;
}
