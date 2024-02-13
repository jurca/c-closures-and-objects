#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

typedef struct Object {
	unsigned long id;
	int (*_foo)(struct Object * self,
		    int(*self_foo)(struct Object * self, int value), int value);
} Object;

#define foo(value) _foo(NULL, NULL, value)

int main(void);

int Object_foo(Object *self, int value)
{
	printf("Object_foo{id=%lu} value=%i\n", self->id, value);
	return self->id + value;
}

static int
#ifdef __GNUC__
    __attribute__((optimize(0)))
#endif
Object_foo_binder(Object *self,
		  int (*self_foo)(Object *self, int value), int value)
{
	static Object *instance = NULL;
	static int (*instance_foo)(Object * self, int value) = NULL;
	if (instance == NULL) {
		instance = self;
		instance_foo = self_foo;
		return 0;
	}

	return instance_foo(instance, value);
}

static int OBJECT_FOO_BINDER_END = 0;

int (*Object_foo_bind(Object * self))(Object * self,
				      int(*self_foo)(Object * self, int value),
				      int value) {
	char *object_foo_binder_start =(char *)Object_foo_binder;
	char *object_foo_binder_end = (char *)&OBJECT_FOO_BINDER_END;
	unsigned long object_foo_binder_size;
	char *object_foo_binder_page;
	int (*bound_foo)(Object *, int(*)(Object *, int), int);

	if (object_foo_binder_start > object_foo_binder_end) {
		/* Who knows what the compiler did to memory layout */
		return NULL;
	}
	/* calculate (well, estimate) size of Object_foo_binder */
	object_foo_binder_size =
	    object_foo_binder_end - object_foo_binder_start;

	/* allocate executable page and copy Object_foo_binder into it */
#ifdef _WIN32
	object_foo_binder_page =
	    VirtualAlloc(NULL, object_foo_binder_size, MEM_COMMIT | MEM_RESERVE,
			 PAGE_EXECUTE_READWRITE);
	if (object_foo_binder_page == NULL) {
		return NULL;
	}
#else
	object_foo_binder_page = mmap(NULL, object_foo_binder_size,
				      PROT_READ | PROT_WRITE | PROT_EXEC,
				      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (object_foo_binder_page == MAP_FAILED) {
		return NULL;
	}
#endif
	memcpy(object_foo_binder_page, object_foo_binder_start,
	       object_foo_binder_size);
	bound_foo = (int (*)(Object *, int (*)(Object *, int), int))
	    object_foo_binder_page;

	/* set self and implementation references in Object_foo_binder's copy */
	bound_foo(self, Object_foo, 0);

	return bound_foo;
}

Object *Object_new(unsigned long id)
{
	Object *self = malloc(sizeof(Object));
	self->id = id;
	self->_foo = Object_foo_bind(self);
	return self;
}

int main()
{
	Object *object1;
	Object *object2;
	int result;

	object1 = Object_new(42);
	result = object1->foo(8);
	printf("result: %d\n", result);

	object2 = Object_new(43);
	result = object2->foo(8);
	printf("result: %d\n", result);

	result = object1->foo(10);
	printf("result: %d\n", result);

	return 0;
}
