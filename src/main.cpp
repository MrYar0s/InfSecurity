#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include <CLI/CLI.hpp>

#include <hash2string.hpp>

#include <sha256.hpp>
#include <keccak.hpp>
#include <lazy.hpp>
#include <blake256.hpp>
#include <gost.hpp>

std::string hash(const std::string &hash_name, const std::string &data)
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
        uint8_t out[32] = {};
        gost.reset();
        gost.update(reinterpret_cast<const uint8_t *>(data.c_str()), data.size());
        gost.final(out);
        return bytes2hex(out);
    }

    std::cerr << "Unknown hash function: " << hash_name << std::endl;
    std::terminate();
}

int main(int argc, char **argv)
{
    CLI::App app("Hash stat");

    std::string output_file {};
    auto *output_arg = app.add_option("-o,--out", output_file, "Output file");
    output_arg->required();

    std::string input_file {};
    auto *input_arg = app.add_option("--in", input_file, "Input file");
    input_arg->required();

    std::string hash_func {};
    auto *hash_func_arg = app.add_option("--hash", hash_func, "Hash func name");
    hash_func_arg->required();

    CLI11_PARSE(app, argc, argv);

    std::ifstream in {input_file};
    std::ofstream out {output_file};

    std::string to_hash {};
    while (std::getline(in, to_hash)) {
        out << hash(hash_func, to_hash) << std::endl;
    }

    return 0;
}
