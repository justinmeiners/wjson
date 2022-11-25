
.PHONY: test

test_w_json: test.c w_json.h
	gcc $< -o $@

test: test_w_json verify.py
	./test_w_json | python3 verify.py



    
