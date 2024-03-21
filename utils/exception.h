#pragma once

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct s_exceptionContext {
	jmp_buf buf;
	struct s_exceptionContext* prev;
} ExceptionContext;

extern ExceptionContext* currentContext;

#define try \
	{ \
		ExceptionContext ctx; \
		ctx.prev = currentContext; \
		currentContext = &ctx; \
		if (setjmp(ctx.buf) == 0) {

#define catch \
		} \
		else {

#define end_try \
		} \
		currentContext = ctx.prev; \
	}

#define throw(e) \
	if (currentContext != NULL) { \
		ExceptionContext* oldContext = currentContext; \
		currentContext = currentContext->prev; \
		longjmp(oldContext->buf, e); \
	} else { \
		fprintf(stderr, "Unhandled exception in %s at line %d\n", __FILE__, __LINE__); \
		exit(e); \
	}
