#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <chrono>
#include <filesystem>

#include <CLI/CLI.hpp>

#include <hash2string.hpp>

#include <sha256.hpp>
#include <keccak.hpp>
#include <lazy.hpp>
#include <blake256.hpp>
#include <gost.hpp>

inline std::string hash(const std::string &hash_name, const std::string &data)
{
    static SHA256 sha256 {};
    static Keccak keccak {};
    static BLAKE256 blake256 {};
    static GOST gost {};

    if (hash_name == "sha256") {
        return sha256(data.c_str(), data.size());
    }
    if (hash_name == "keccak") {
        return keccak(data.c_str(), data.size());
    }
    if (hash_name == "lazy") {
        return lazy_hash(data.c_str(), data.size());
    }
    if (hash_name == "blake256") {
        return blake256(data.c_str(), data.size());
    }
    if (hash_name == "gost") {
        return gost(data.c_str(), data.size());
    }

    std::cerr << "Unknown hash function: " << hash_name << std::endl;
    std::terminate();
}

int main(int argc, char **argv)
{
    CLI::App app("Hash stat");

    std::string input_file {};
    auto *input_arg = app.add_option("--in", input_file, "Input file");
    input_arg->required();

    std::string hash_func {};
    auto *hash_func_arg = app.add_option("--hash", hash_func, "Hash func name");
    hash_func_arg->required();

    CLI11_PARSE(app, argc, argv);

    std::ifstream in {input_file};

    std::string file_name = std::filesystem::path(input_file).filename();

    std::string to_hash {};
#ifdef MEASURE_TIME
    std::ofstream outfile_time ("consumed_time_" + hash_func + "_" + file_name + ".txt");
    auto start = std::chrono::high_resolution_clock::now();
#endif
#ifndef MEASURE_TIME
    std::ofstream outfile_hashes ("hashes_for_" + hash_func + "_" + file_name + ".txt");
#endif
    while (std::getline(in, to_hash)) {
        auto hash_val = hash(hash_func, to_hash);
#ifndef MEASURE_TIME
        outfile_hashes << hash_val << std::endl;
#endif
    }
#ifndef MEASURE_TIME
    outfile_hashes.close();
#endif
#ifdef MEASURE_TIME
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
    outfile_time << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;
    outfile_time.close();
#endif
    return 0;
}
