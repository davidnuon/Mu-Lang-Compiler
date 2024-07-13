build: 
	gcc *.c -o mu
build-wasm: 
	mkdir -p build
	emcc *.c -o build/mu-wasm.html
