#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

struct UsageException : public std::exception {
    const char* what() const noexcept override {
        return "Usage: huff <encode> (<decode>) <input> <output> \nUse 'huff --help' for additional info \n";
    }
};

struct QueueUnderflow : public std::exception {
    const char* what() const noexcept override {
        return "Queue underflow";
    }
};

struct WrongQueueSize : public std::exception {
    const char* what() const noexcept override {
        return "Wrong queue size";
    }
};

struct EmptyInputFile : public std::exception {
    const char* what() const noexcept override {
        return "Cannot encode empty file";
    }
};

struct CompressionIneffective : public std::exception {
    const char* what() const noexcept override {
        return "Compression is not effective for this file";
    }
};

#endif // EXCEPTIONS_H
