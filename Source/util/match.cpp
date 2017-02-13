#include <cstring>
#include <cctype>


namespace rvg {
    namespace util {

int match(const char *argument, const char *option, const char **value) {
    int length = (int) strlen(option);
    if (ispunct(option[length-1])) length--;
    if (strncmp(argument, option, length) == 0
        && (ispunct(argument[length]) || !argument[length])) {
        if (value) {
            if (option[length] == argument[length]) length++;
            *value = argument+length;
        }
        return 1;
    } else return 0;
}

} } // namespace rvg::util
