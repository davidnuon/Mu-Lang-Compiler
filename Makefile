build: 
	mkdir -p build
	gcc *.c -o build/mu-lang
build-wasm: 
	mkdir -p build
	emcc *.c -o build/mu-wasm.html
