#include <iostream>
#include <fstream>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>

using namespace boost::multiprecision;
using namespace boost::random;

cpp_int mod_exp(const cpp_int& base, const cpp_int& exp, const cpp_int& mod) {
    cpp_int result = 1;
    cpp_int b = base % mod;
    cpp_int e = exp;
    while (e > 0) {
        if (e % 2 == 1) {
            result = (result * b) % mod;
        }
        b = (b * b) % mod;
        e /= 2;
    }
    return result;
}

cpp_int gcd(const cpp_int& a, const cpp_int& b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

cpp_int mod_inverse(const cpp_int& e, const cpp_int& phi) {
    cpp_int t = 0, new_t = 1;
    cpp_int r = phi, new_r = e;

    while (new_r != 0) {
        cpp_int quotient = r / new_r;
        std::swap(t, new_t -= quotient * t);
        std::swap(r, new_r -= quotient * r);
    }
    if (t < 0) {
        t += phi;
    }
    return t;
}

bool is_prime(const cpp_int& n, size_t rounds = 25) {
    if (n <= 1)
        return false;
    if (n <= 3)
        return true;
    if (n % 2 == 0)
        return false;

    cpp_int d = n - 1;
    while (d % 2 == 0)
        d /= 2;

    boost::random::random_device rd;
    boost::random::mt19937 gen(rd());

    for (size_t i = 0; i < rounds; ++i) {
        boost::random::uniform_int_distribution<cpp_int> dist(2, n - 2);
        cpp_int a = dist(gen);
        cpp_int x = mod_exp(a, d, n);

        if (x == 1 || x == n - 1)
            continue;

        bool composite = true;
        cpp_int temp_d = d;
        while (temp_d != n - 1) {
            x = mod_exp(x, 2, n);
            temp_d *= 2;

            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite)
            return false;
    }
    return true;
}

cpp_int generate_prime(size_t bits) {
    boost::random::random_device rd;
    boost::random::mt19937 gen(rd());
    boost::random::uniform_int_distribution<cpp_int> dist(1 << (bits - 1), (cpp_int(1) << bits) - 1);

    cpp_int prime;
    do {
        prime = dist(gen);
    } while (!is_prime(prime));

    return prime;
}

void generate_keys(cpp_int& n, cpp_int& e, cpp_int& d) {
    size_t bits = 1024;

    cpp_int p = generate_prime(bits);
    cpp_int q = generate_prime(bits);
    n = p * q;
    cpp_int phi = (p - 1) * (q - 1);
    e = 65537;  
    d = mod_inverse(e, phi);
}

cpp_int encrypt(const cpp_int& message, const cpp_int& e, const cpp_int& n) {
    return mod_exp(message, e, n);
}

cpp_int decrypt(const cpp_int& ciphertext, const cpp_int& d, const cpp_int& n) {
    return mod_exp(ciphertext, d, n);
}

void save_key_to_file(const std::string& filename, const cpp_int& key) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << key;
        file.close();
    }
}

void load_key_from_file(const std::string& filename, cpp_int& key) {
    std::ifstream file(filename);
    if (file.is_open()) {
        file >> key;
        file.close();
    }
}

void read_from_file(const std::string filename, std::string& message) {
    std::ifstream file(filename);
    std::string str;
    if (file.is_open()) {
        while (!file.eof()) {
            getline(file, str);
            message += str+'\n';
        }
        
    }
}

int main() {
    cpp_int n, e, d;
    char y;
    std::cout << "Do you want to generate keys? Y/n";
    std::cin >> y;
    if (tolower(y) == 'y') {
        generate_keys(n, e, d);

        save_key_to_file("public_key.txt", e);
        save_key_to_file("private_key.txt", d);
        save_key_to_file("modulus.txt", n);
    }
    else {
        std::ifstream file1("public_key.txt");
        file1 >> e;
        std::ifstream file1("private_key.txt");
        file1 >> d;
        std::ifstream file1("modulus.txt");
        file1 >> n;
    }

    std::string message;
    read_from_file("input.txt",message);
    cpp_int plaintext(message);
    cpp_int encrypted_message;
    std::cout << "Do you want to encrypt message? Y/n";
    std::cin >> y;
    if (tolower(y) == 'y') {
        encrypted_message = encrypt(plaintext, e, n);
        std::ofstream file("encrypted_message.txt");
        file << encrypted_message;
    }
    else {
        std::ifstream file("encrypted_message.txt");
        file >> encrypted_message;
    }
    std::cout << "Do you want to encrypt message? Y/n";
    std::cin >> y;
    if (tolower(y) == 'y') {
        cpp_int decrypted_message = decrypt(encrypted_message, d, n);
        std::ifstream file("decrypted_message.txt");
        file >> decrypted_message;
    }
    return 0;
}
