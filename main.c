#include <stdio.h>

int ft_printf(const char*, ...);

int main() {
	char *str = "Hello, World!\n";
	printf ("my: %d lib: %d \n", printf(str), ft_printf(str));
  fflush(stdout);
	str = "%s, %s!\n";
	printf ("my: %d lib: %d \n", printf(str, "Hello", "World"), ft_printf(str, "Hello", "World"));
	
  fflush(stdout);
  str = "%s, %s%s";
  printf ("my: %d lib: %d \n", printf(str, "Hello", "World", "!\n"), ft_printf(str, "Hello", "World", "!\n"));
	fflush(stdout);

  str = "%d %d %d %d %d %d %d %d\n";
	printf ("my: %d lib: %d \n", printf(str, 0, 1, -1, 42, 10, 10000000, 0x80000000, 0x7fffffff), ft_printf(str, 0, 1, -1, 42, 10, 10000000,0x80000000, 0x7fffffff));

	str = "%x %x %x %x %x %x %x %x\n";
	printf ("my: %d lib: %d \n", printf("lb: %x %x %x %x %x %x %x %x %x\n", -42, 0, 1, -1, 42, 10, 10000000, 0x80000000, 0x7fffffff), ft_printf("my: %x %x %x %x %x %x %x %x %x\n", -42, 0, 1, -1, 42, 10, 10000000, 0x80000000, 0x7fffffff));

	return 0;
}
