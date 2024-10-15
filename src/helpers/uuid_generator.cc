
#include "uuid_generator.h"
#include <cstdio>
#include <random>

const std::string uuid_version = "4";
std::string       generate_uuid_segment(int length) {
    //possible characters for generation of UUID.
    const std::string               CHARACTERS = "abcdef1234567890";
    std::random_device              rd;
    std::mt19937                    generator(rd());
    std::uniform_int_distribution<> distibution(0, CHARACTERS.size() - 1);
    std::string                     random_string;
    for (int i = 0; i < length; i++) {
        random_string += CHARACTERS[distibution(generator)];
    }
    return random_string;
}

std::string generate_uuid() {
    return generate_uuid_segment(8) + "-" + generate_uuid_segment(4) + "-" + uuid_version + generate_uuid_segment(3) + "-" + generate_uuid_segment(4) + "-" +
        generate_uuid_segment(12);
}
