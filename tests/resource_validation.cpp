#include "../src/engine/asset_validation.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
int main(int argc, char **argv) {
    unsigned char empty[24] = {};
    assert(!AssetValidation::War(empty, sizeof(empty)));
    assert(!AssetValidation::Dav(empty, sizeof(empty)));
    assert(!AssetValidation::Range(16, 17, 0));
    assert(!AssetValidation::Range(16, 0, SIZE_MAX, 4));
    int checked = 0;
    for (int i = 1; i < argc; ++i) {
        std::ifstream f(argv[i], std::ios::binary);
        std::vector<unsigned char> b((std::istreambuf_iterator<char>(f)), {});
        std::string path = argv[i]; bool war = path.size() >= 4 && (path.substr(path.size()-4) == ".WAR" || path.substr(path.size()-4) == ".war");
        auto valid = war ? AssetValidation::War : AssetValidation::Dav;
        if (!valid(b.data(), b.size())) { fprintf(stderr, "INVALID %s\n", argv[i]); return 1; }
        assert(!valid(b.data(), 8));
        auto broken = b;
        size_t at = war ? 12 : 20;
        for (size_t j = 0; j < 4; ++j) broken[at+j] = 255;
        assert(!valid(broken.data(), broken.size()));
        ++checked;
    }
    printf("Validated %d real resources plus truncated/overflow mutations\n", checked);
}
