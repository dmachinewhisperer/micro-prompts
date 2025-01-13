all: run_python_script run_build_script

run_python_script:
	@echo "Prepping /providers/google.c for build..."
	python3 prep_files.py ../providers/google.c || exit 1
	@echo "Prepping /providers/openai.c for build..."
	python3 prep_files.py ../providers/openai.c || exit 1
	@echo "Done."

run_build_script: run_python_script
	@echo "Running build script compile.sh..."
	./compile.sh || exit 1
	@echo "Done."

run: all
	@echo "Running the program..."
	./testbench || exit 1

valgrind: all
	@echo "Running with valgrind..."
	valgrind --leak-check=full --track-origins=yes ./testbench || exit 1

clean:
	@echo "Cleaning up..."
	#

.PHONY: all run_python_script run_build_script clean run valgrind