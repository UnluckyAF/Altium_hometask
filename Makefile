.PHONY: build
build:
	g++ -std=c++17 solution.cpp

.PHONY: run
run:
	./a.out

.PHONY: test
test: build
	for name in tests/[A-Z]; do\
		echo "------------------------------------------------------\n";\
		echo "TEST CASE $${name}:\n";\
		cat $${name};\
		echo "------------------------------------------------------\n";\
		echo "SOLUTION'S ANSWER:\n";\
		./a.out $${name};\
		echo "------------------------------------------------------\n";\
		echo "ANSWER:\n";\
		cat $${name}_ans;\
		echo "------------------------------------------------------\n\n";\
	done
