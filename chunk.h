#ifndef mu_chunk_h
#define mu_chunk_h

#include "common.h"
#include "value.h"

typedef enum
{
  OP_CONSTANT,
  OP_NEGATE,
  OP_RETURN, // return from the current function
} OpCode;

typedef struct
{
  int count;
  int capacity;
  uint8_t *code; // array of bytecode
  int *lines;    // array of lines
  ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);

#endif