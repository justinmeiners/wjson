import sys, json;
data = json.load(sys.stdin)

assert(data["test_simple"]["age"] == 40)
assert(len(data["test_simple"]["children"]) == 3)

assert(data["test_escape"]["line\nbreak"])

print("PASS")





