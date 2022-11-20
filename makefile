
.PHONY: test

test_wjson: test.c wjson.h
	gcc $< -o $@

test: test_wjson verify.py
	./test_wjson | python3 verify.py



    
