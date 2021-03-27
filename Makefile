.PHONY: all compile run diff

all: compile run diff

compile:
	@gcc process.c -o process
	@gcc watchdog.c -o watchdog
	@g++ executor.cpp -o executor
run:
	@./executor numberOfProcesses instruction_path &
	@./watchdog numberOfProcesses process_output watchdog_output