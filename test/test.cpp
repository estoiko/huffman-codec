#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "Decoder.h"
#include "Encoder.h"
#include "Exceptions.h"

namespace {

std::vector<std::filesystem::path> inputFiles() {
    std::vector<std::filesystem::path> files;

    for (const auto& entry : std::filesystem::directory_iterator(HUFFMAN_TEST_INPUT_DIR)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().filename().string()[0] == '.') continue;
        files.push_back(entry.path());
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::filesystem::path> corruptedFiles() {
    const std::filesystem::path corruptedDir =
        std::filesystem::path(HUFFMAN_TEST_INPUT_DIR).parent_path() / "corrupted";

    std::vector<std::filesystem::path> files;

    if (!std::filesystem::exists(corruptedDir)) {
        return files;
    }

    for (const auto& entry : std::filesystem::directory_iterator(corruptedDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".huf") {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

bool filesEqual(const std::filesystem::path& a, const std::filesystem::path& b) {
    if (std::filesystem::file_size(a) != std::filesystem::file_size(b)) {
        return false;
    }

    std::ifstream fa(a, std::ios::binary);
    std::ifstream fb(b, std::ios::binary);

    return std::equal(
        std::istreambuf_iterator<char>(fa),
        std::istreambuf_iterator<char>(),
        std::istreambuf_iterator<char>(fb),
        std::istreambuf_iterator<char>()
    );
}

std::uintmax_t fileSize(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return 0;
    }
    return std::filesystem::file_size(path);
}

void prepareArtifactDirs() {
    std::error_code ec;
    std::filesystem::remove_all(HUFFMAN_TEST_CODED_DIR);
    std::filesystem::remove_all(HUFFMAN_TEST_OUTPUT_DIR);
    std::filesystem::create_directories(HUFFMAN_TEST_CODED_DIR);
    std::filesystem::create_directories(HUFFMAN_TEST_OUTPUT_DIR);
}

} // namespace

TEST(HuffmanEncoding, CorruptedFiles) {
    const auto files = corruptedFiles();

    ASSERT_FALSE(files.empty())
        << "No corrupted files found. Put broken archives into test/corrupted";

    for (const auto& corruptedPath : files) {
        SCOPED_TRACE(corruptedPath.string());

        std::ifstream in(corruptedPath, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        ASSERT_TRUE(in) << "Cannot open corrupted file: " << corruptedPath;

        EXPECT_THROW(decodeFile(in, out), InvalidArchive);
    }
}

TEST(HuffmanEncoding, IneffectiveCompressionThrow) {
    const std::filesystem::path emptyPath = std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "empty.txt";
    const std::filesystem::path onePath = std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "one_character.txt";
    const std::filesystem::path repeatedPath =
        std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "one_character_repeated.txt";

    {
        std::ifstream in(emptyPath, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        ASSERT_TRUE(in);
        EXPECT_THROW(encodeFile(in, out), EmptyInputFile);
    }

    {
        std::ifstream in(onePath, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        ASSERT_TRUE(in);
        EXPECT_THROW(encodeFile(in, out), CompressionIneffective);
    }

    {
        std::ifstream in(repeatedPath, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        ASSERT_TRUE(in);
        EXPECT_THROW(encodeFile(in, out), CompressionIneffective);
    }
}

TEST(HuffmanEncoding, BasicEncode) {
    prepareArtifactDirs();

    const auto files = inputFiles();
    ASSERT_FALSE(files.empty()) << "No input files found in " << HUFFMAN_TEST_INPUT_DIR;

    for (const auto& inputPath : files) {
        SCOPED_TRACE(inputPath.string());

        const std::uintmax_t originalSize = fileSize(inputPath);
        const auto encodedPath =
            std::filesystem::path(HUFFMAN_TEST_CODED_DIR) /
            (inputPath.filename().string() + ".huf");

        const auto decodedPath = std::filesystem::path(HUFFMAN_TEST_OUTPUT_DIR) / inputPath.filename();

        if (originalSize == 0) {
            std::ifstream in(inputPath, std::ios::binary);
            std::ostringstream out(std::ios::binary);

            ASSERT_TRUE(in) << "Cannot open input file: " << inputPath;
            EXPECT_THROW(encodeFile(in, out), EmptyInputFile);
            std::cout << inputPath.filename().string()
                      << ": original=0 bytes - cannot encode empty file\n";
            continue;
        }

        bool wasEncoded = false;

        {
            std::ifstream in(inputPath, std::ios::binary);
            std::ofstream out(encodedPath, std::ios::binary);

            ASSERT_TRUE(in) << "Cannot open input file: " << inputPath;
            ASSERT_TRUE(out) << "Cannot open encoded output file: " << encodedPath;

            try {
                encodeFile(in, out);
                wasEncoded = true;
            } catch (const CompressionIneffective&) {
                std::cout << inputPath.filename().string()
                          << ": compression ineffective, skipping decode check.\n";
            }
        }

        if (!wasEncoded) {
            continue; // Идем к следующему файлу, если этот сжался неэффективно
        }

        const std::uintmax_t encodedSize = fileSize(encodedPath);

        std::cout << inputPath.filename().string()
                  << ": original=" << originalSize
                  << " bytes, encoded=" << encodedSize
                  << " bytes";

        if (originalSize > 0) {
            const double compressionPercent =
                (1.0 - static_cast<double>(encodedSize) / static_cast<double>(originalSize)) * 100.0;
            std::cout << ", compression=" << std::fixed << std::setprecision(2)
                      << compressionPercent << "%";
        } else {
            std::cout << ", compression=N/A";
        }

        {
            std::ifstream in(encodedPath, std::ios::binary);
            std::ofstream out(decodedPath, std::ios::binary);

            ASSERT_TRUE(in) << "Cannot open encoded file: " << encodedPath;
            ASSERT_TRUE(out) << "Cannot open decoded output file: " << decodedPath;

            decodeFile(in, out);
        }

        const bool equal = filesEqual(inputPath, decodedPath);
        std::cout << (equal ? " - OK\n" : " - FAIL\n");
        EXPECT_TRUE(equal);
    }
}
