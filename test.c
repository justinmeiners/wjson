#define W_JSON_IMPLEMENTATION
#include "w_json.h"

void test_simple(wJson* w) {
    wJson_object(w);
    wJson_string_pair(w, "name", "bob");

    wJson_key_raw(w, "age"); wJson_int(w, 40);
    wJson_key_raw(w, "height"); wJson_double(w, 5.11);
    wJson_key(w, "citizen"); wJson_bool(w, 1);
    
    wJson_key(w, "children");
    const char* names[] = {
        "Jill",
        "Adam",
        "Alice"
    };
    wJson_stringv(w, 3, names);
    wJson_end(w);
}

void test_escape(wJson* w) {
    wJson_object(w);
    wJson_key(w, "line\nbreak");
    wJson_string(w, "here is a quote: \" and backslash: \\");
    wJson_end(w);
}

int main() {
    wJson w;
    wJson_init(&w, stdout);

    wJson_object(&w);

    wJson_key_raw(&w, "test_simple");
    test_simple(&w);

    wJson_key_raw(&w, "test_escape");
    test_escape(&w);

    wJson_end(&w);

    wJson_shutdown(&w);
}
