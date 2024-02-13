# Closures and object-oriented syntax in C - tech demo

This repository contains a proof-of-concept for implementing closures and using
them for adding true object-oriented method calling syntax to C - that is
calling an object's method without having to pass the object pointer to the
method manually.

Instead of:

```c
object1 = Object_new(42);
result = object1->foo(object1, 8);
```

This tech demo enables the following:

```c
object1 = Object_new(42);
result = object1->foo(8);
```

## How does it work

Note: This section expects the reader to at least superficially understand
memory management.

Binding the `foo` method (implemented by the `Object_foo` function) to an object
uses the following two helpers:

- `Object_foo_binder` - function uses static local variables to keep track of
  the object pointer. A function like this one is needed for every method.
- `Object_foo_bind` - function that allocates a new executable memory page and
  copies the `Object_foo_binder` function into it, then configures it by passing
  the object pointer to it. This function can be somewhat generalized and
  reused.

The `Object_foo` function has the following signature:

```c
int Object_foo(Object *self, int value);
```

The function requires a pointer to the object, which will be provided by the
`Object_foo_binder` function.

The `Object_foo_binder` function sets its static local object pointer and
`Object_foo` function pointer to the pointers provided to it on its first
invocation and returns, remembering them for all subsequent invocations. Any
subsequent call ignores the object pointer and `Object_foo` function pointer
arguments and uses the remembered pointers to call the `Object_foo` function.

The `Object_foo_bind` handles the method setup. It estimates the size of
`Object_foo_binder` function, allocates executable memory pages and copies it
into the newly allocated memory. The created `Object_foo_binder`'s copy is then
invoked by `Object_foo_bind` with the object instance pointer and `Object_foo`
function pointer to set it up for calling `Object_foo` with the provided object
instance pointer.

The pointer to the `Object_foo_binder`'s copy is then set in the `Object` struct
as the `foo` field's value.

Note: functions use relative references to call each other, that's why we need
to store the pointer to `Object_foo` so it can be invoked from
`Object_foo_binder`'s copy in another memory location.

This allows us to call the bound method in the following manner:

```c
instance->foo(
    NULL /* the no-longer-needed instance pointer */,
    NULL /* the no-longer-needed Object_foo pointer */,
    1 /* the remaining arguments - int value */
);
```

Since we can use `NULL` for the first two arguments, we can have it provided
automatically using a simple C macros and end up with this:

```c
instance->foo(1 /* method's arguments - int value */);
```

## Caveats

- Cross-compiler (and better cross-platform) portability would need some work.
- The binder function must not be optimized away by the compiler, otherwise the
  code would not be able to create copies of it at runtime.
- Overall portability is unknown and depends on how the compiler will handle
  memory layout of declarations.
- Allocating new executable memory pages may not be allowed in certain
  environments for security reasons (risk of copying of application's input into
  the allocated pages and causing a remote code execution vulnerability).
- The memory requirements are impractical.
- You should be using a different language than C if you want object-oriented
  syntax.

## Recommendations

If you are looking for an object-oriented syntax in C, just use another
language. Seriously. This was just a fun challenge to see how far can C be
pushed.

If you want an object system for C, there are much better (and feature-complete)
ways of doing this, see for example https://github.com/CObjectSystem/COS and
https://ldeniau.web.cern.ch/ldeniau/cos.html.
