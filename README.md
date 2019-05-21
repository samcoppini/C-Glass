# C Glass
This is the repository for a work-in-progress interpreter for the esoteric 
programming language [Glass](https://esolangs.org/wiki/Glass). For those
unfamiliar with the language, here is a quote from the creator:

> Glass is considered unique by its creator because it combines the
> unintuitive postfix notation with object orientation, and also 
> requires extensive use of pointers and a main stack, despite being
> (mostly) object oriented. No other language (that he knows of) is
> implemented like this, because it would be idiotic to do so.

I had previously written an interpreter for Glass in C++, but when I  realized
I could make a [sea glass](https://en.wikipedia.org/wiki/Sea_glass) pun if I
wrote it in C, I had to re-write the interpreter.

## Building the interpreter

This repo uses the [Meson build system](https://mesonbuild.com/) to build. To build the interpreter, navigate to the source root, and run these commands:

```shell
$ meson build
$ ninja -C build install
```

Then, to test that it worked, try the following:

```shell
$ cglass ./glass/examples/hello.glass
Hello World!
```
