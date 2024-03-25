cmake_minimum_required(VERSION 3.12)

project(assembler)

set(SRCS
	assembler.c
	fileIO.c
	utils/mem.c
	utils/exception.c
	utils/logger.c
	symbols.c
	converter.c
	instruction.c
	COFF.c
	sections.c
	directives.c
	)
	
set(HEADERS
	fileIO.h
	utils/exception.h
	utils/mem.h
	utils/logger.h
	symbols.h
	converter.h
	instruction.h
	COFF.h
	sections.h
	directives.h
	)

add_executable(assembler ${SRCS} ${HEADERS})