#include <iostream>
#include <vector>

#include "stroke/style.h"

using namespace rvg::stroke;

int main(void) {
    Style st;
    std::cout << st.stroked(1.f).joined(Join::round).capped(Cap::square).dashed({1, 2, 1, 1, 3}).by(Method::driver) << '\n';
    return 0;
}
