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
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
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
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::vector<char> readBinaryFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    EXPECT_TRUE(in) << "Cannot open file: " << path;

    return std::vector<char>(
        std::istreambuf_iterator<char>(in),
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

        EXPECT_ANY_THROW(decodeFile(in, out));
    }
}

TEST(HuffmanEncoding, BasicEncode) {
    prepareArtifactDirs();

    const auto files = inputFiles();
    ASSERT_FALSE(files.empty()) << "No input files found in " << HUFFMAN_TEST_INPUT_DIR;

    for (const auto& inputPath : files) {
        SCOPED_TRACE(inputPath.string());

        const auto original = readBinaryFile(inputPath);
        const auto encodedPath = std::filesystem::path(HUFFMAN_TEST_CODED_DIR) / inputPath.filename();
        const auto decodedPath = std::filesystem::path(HUFFMAN_TEST_OUTPUT_DIR) / inputPath.filename();

        if (original.empty()) {
            std::ifstream in(inputPath, std::ios::binary);
            std::ostringstream out(std::ios::binary);

            ASSERT_TRUE(in) << "Cannot open input file: " << inputPath;
            EXPECT_THROW(encodeFile(in, out), EmptyInputFile);
            std::cout << inputPath.filename().string()
                      << ": original=0 bytes - cannot encode empty file\n";
            continue;
        }

        {
            std::ifstream in(inputPath, std::ios::binary);
            std::ofstream out(encodedPath, std::ios::binary);

            ASSERT_TRUE(in) << "Cannot open input file: " << inputPath;
            ASSERT_TRUE(out) << "Cannot open encoded output file: " << encodedPath;

            encodeFile(in, out);
        }

        const std::uintmax_t originalSize = original.size();
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

        const auto decoded = readBinaryFile(decodedPath);
        if (decoded.size() == original.size()) {
            std::cout << " - OK\n";
        } else {
            std::cout << " - FAIL\n";
        }

        EXPECT_EQ(decoded.size(), original.size());
        EXPECT_EQ(decoded, original);
    }
}

TEST(HuffmanEncoding, CarefulEncodeSkip) {
    const std::filesystem::path emptyPath = std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "empty.txt";
    const std::filesystem::path onePath = std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "one_character.txt";
    const std::filesystem::path repeatedPath =
        std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "one_character_repeated.txt";

    {
        std::ifstream in(emptyPath, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        ASSERT_TRUE(in);
        EXPECT_THROW(carefulEncodeFile(in, out), EmptyInputFile);
    }

    {
        std::ifstream in(onePath, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        ASSERT_TRUE(in);
        EXPECT_THROW(carefulEncodeFile(in, out), CompressionIneffective);
    }

    {
        std::ifstream in(repeatedPath, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        ASSERT_TRUE(in);
        EXPECT_THROW(carefulEncodeFile(in, out), CompressionIneffective);
    }
}

TEST(HuffmanEncoding, CarefulEncode) {
    const std::vector<std::filesystem::path> effectiveFiles = {
        std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "big.txt",
        std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "english-lorem.txt",
        std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "russian-lorem.txt",
        std::filesystem::path(HUFFMAN_TEST_INPUT_DIR) / "utf8-demo.txt",
    };

    const auto workDir = std::filesystem::temp_directory_path() / "huffman_encoding_careful_tests";
    std::filesystem::create_directories(workDir);

    for (const auto& inputPath : effectiveFiles) {
        SCOPED_TRACE(inputPath.string());

        const auto original = readBinaryFile(inputPath);
        const auto encodedPath = workDir / (inputPath.filename().string() + ".careful.huf");
        const auto decodedPath = workDir / (inputPath.filename().string() + ".careful.decoded");

        {
            std::ifstream in(inputPath, std::ios::binary);
            std::ofstream out(encodedPath, std::ios::binary);

            ASSERT_TRUE(in);
            ASSERT_TRUE(out);
            EXPECT_NO_THROW(carefulEncodeFile(in, out));
        }

        ASSERT_LT(fileSize(encodedPath), original.size());

        {
            std::ifstream in(encodedPath, std::ios::binary);
            std::ofstream out(decodedPath, std::ios::binary);

            ASSERT_TRUE(in);
            ASSERT_TRUE(out);
            decodeFile(in, out);
        }

        EXPECT_EQ(readBinaryFile(decodedPath), original);
    }
}
