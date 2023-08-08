#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

int ft_strlen(const char *str) {
	int i = 0;
	for(;str[i]; i++);
	return i;
}

char *ft_strcpy(const char *str) {
	char *copy;

	copy = malloc(sizeof(char) * (ft_strlen(str) + 1));
	int i = 0;
	for (; str[i]; i++)
		copy[i] = str[i];
	copy[i] = '\0';
	return copy;
}

char *ft_strncpy(char *src1, char *src2, int *i) {
	char	*copy = malloc(sizeof(char) * (ft_strlen(src1) + ft_strlen(src2) + 1));
	int		i_copy = 0;
	int		i1 = 0;

	for (; src1[i1] && i1 < (*i - 1); i1++) {
		copy[i_copy] = src1[i1];
		i_copy++;
	}
	for (int i2 = 0; src2[i2]; i2++) {
		copy[i_copy] = src2[i2];
		i_copy++;
	}
	*i = i_copy - 1;
	for (i1 += 2; src1[i1]; i1++) {
		copy[i_copy] = src1[i1];
		i_copy++;
	}
	copy[i_copy] = '\0';
	free(src1);
	free(src2);
	return copy;
}

char *int_tostr(int n) {
  long ln = n;
  int range = 1;
  int neg = 0;

	if(n < 0) {
		ln *= -1;
		neg = 1;
	}
	for (long dec = 10; dec <= ln; dec *= 10)
		range++;

  char *str = malloc(sizeof(char) * (neg + range + 1));
  str[neg + range] = '\0' ;
  for (int i = neg + range - 1; i > (neg - 1); i--) {
  	str[i] = (ln % 10) + '0';
    ln /= 10;
  }
  if(neg)
  	str[0] = '-';
	return str;
}

char *hex(long ln) {
	int range = 1;
	if (ln < 0) {
		ln = 0x100000000 - ln * -1;
	}

	for (long hex = 16; hex <= ln; hex *= 16)
		range++;

	char *str = malloc(sizeof(char) * (range + 1));
	str[range] = '\0' ;
	for (int i = range - 1; i >= 0; i--) {
		if ((ln % 16) < 10) {
			str[i] = (ln % 16) + '0';
		} else {
			str[i] = (ln % 16 - 10) + 'a';
		}
		ln /= 16;
	}
	return str;
}

int ft_printf(const char* form, ...) {
	va_list ap;
	char *str = ft_strcpy(form);

	va_start(ap, form);
	for (int i = 0; str[i]; i++) {
		if (str[i] == '%') {
			i++;
			if (str[i] == '%')
				str = ft_strncpy(str, ft_strcpy("%"), &i);
			else if (str[i] == 's')
				str = ft_strncpy(str, ft_strcpy(va_arg(ap, char *)), &i);
      		else if (str[i] == 'd')
				str = ft_strncpy(str, int_tostr(va_arg(ap, int)), &i);
      		else if (str[i] == 'x')
				str = ft_strncpy(str, hex(va_arg(ap, int)), &i);
		}
	}
	va_end(ap);
	int count = ft_strlen(str);
	write(1, str, count);
	free(str);
	return count;
}
