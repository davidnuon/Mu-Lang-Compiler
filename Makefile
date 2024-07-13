build: 
	mkdir -p build
	gcc *.c -o build/mu-lang
build-wasm: 
	mkdir -p build
	emcc *.c -o build/mu-wasm.html

build-wasm-lib:
	mkdir build
	emcc app.c chunk.c compiler.c disassemble.c memory.c object.c parser.c scanner.c table.c value.c vm.c web.c -o function.html -sEXPORTED_FUNCTIONS=_run_source -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
