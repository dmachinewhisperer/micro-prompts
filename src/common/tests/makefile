all: run_build_script

run_python_script:
	@echo "Prepping /providers/google.c for build..."
	python3 prep_files.py ../providers/google.c || exit 1
	@echo "Prepping /providers/openai.c for build..."
	python3 prep_files.py ../providers/openai.c || exit 1
	@echo "Done."

restore:
	@echo "Restoring /providers/google.c for build..."
	python3 prep_files.py ../providers/google.c --rev || exit 1
	@echo "Restoring /providers/openai.c for build..."
	python3 prep_files.py ../providers/openai.c --rev || exit 1
	@echo "Done."

run_build_script:
	@echo "Running build script compile.sh..."
	./compile.sh || exit 1
	@echo "Done."

real: run_build_script
	@echo "Building the real project..."

test: run_python_script run_build_script
	@echo "Building the test project..."

run: all
	@echo "Running the program..."
	./testbench || exit 1

valgrind: all
	@echo "Running with valgrind..."
	valgrind --leak-check=full --track-origins=yes ./testbench || exit 1

clean:
	@echo "Cleaning up..."
	# Add cleanup commands here (e.g., remove temporary files)

.PHONY: all run_python_script run_build_script clean run valgrind real test
