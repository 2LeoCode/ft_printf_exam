#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ft_putnbr(n) _Generic((n), int: ft_putint_nosign, unsigned: ft_puthex)(n)
#define ft_nbrlen(n) _Generic((n), int: ft_intlen, unsigned: ft_hexlen)(n)
#define FLAG_UNSET ((size_t)-1)

typedef struct s_flags {
	size_t	minimal_field;
	size_t	precision;
}	t_flags;

typedef	int	(*t_print_fun)(t_flags, va_list);

int	ft_print_int(t_flags flags, va_list ap);
int	ft_print_str(t_flags flags, va_list ap);
int	ft_print_hex(t_flags flags, va_list ap);

t_print_fun	g_print_fun[127] = {
	['d'] = ft_print_int,
	['s'] = ft_print_str,
	['x'] = ft_print_hex
};

bool	ft_isspace(char c) {
	return (c == ' ' || (c >= '\t' && c <= '\r'));
}

bool	ft_isalpha(char c) {
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

bool	ft_isdigit(char c) {
	return (c >= '0' && c <= '9');
}

size_t	ft_intlen(int n) {
	size_t	len = n < 0;

	while (n)
		len++, n /= 10;
	return (len);
}

size_t	ft_hexlen(int n) {
	size_t	len = 0;

	while (n)
		len++, n /= 16;
	return (len);
}

size_t	ft_strlen(const char *s) {
	const char	*ptr = s;

	while (*ptr)
		ptr++;
	return ((size_t)ptr - (size_t)s);
}

void	*ft_memset(void *b, int c, size_t len) {
	char	*ptr = b;

	for (size_t i = 0; i < len; i++) {
		ptr[i] = c;
	}
	return (b);
}

int	ft_putchar(char c) {
	return (write(1, &c, 1));
}

int	ft_putint_nosign(int n) {
	unsigned	u = n < 0 ? -(long)n : (unsigned)n;
	int			ret = 0;

	if (u < 10)
		return (ft_putchar(u + 48));
	ret += ft_putint_nosign(u / 10);
	ret += ft_putint_nosign(u % 10);
	return (ret);
}

int	ft_puthex(unsigned n) {
	int ret = 0;

	if (n < 10)
		return (ft_putchar(n + 48));
	if (n < 16)
		return (ft_putchar(n + 87));
	ret += ft_puthex(n / 16);
	ret += ft_puthex(n % 16);
	return (ret);
}

int	process_number_flags(t_flags flags, size_t len, bool neg) {
	size_t	full_len;
	int	cnt = 0;

	if (flags.precision != FLAG_UNSET && flags.precision > len)
		full_len = flags.precision;
	else
		full_len = len;
	if (flags.minimal_field != FLAG_UNSET && flags.minimal_field > len) {
		const size_t	space_cnt = flags.minimal_field - full_len;
		char			spaces[space_cnt];

		ft_memset(spaces, ' ', space_cnt);
		cnt += write(1, spaces, space_cnt);
	}
	if (neg)
		cnt += write(1, "-", 1);
	if (flags.precision != FLAG_UNSET && flags.precision > len) {
		const size_t	zero_count = flags.precision - len;
		char			zeroes[zero_count];

		ft_memset(zeroes, '0', zero_count);
		cnt += write(1, zeroes, zero_count);
	}
	return (cnt);
}

int	process_string_flags(t_flags flags, size_t len) {
	int	cnt = 0;

	if (flags.minimal_field != FLAG_UNSET && flags.minimal_field > len) {
		const size_t	space_cnt = flags.minimal_field - len;
		char			spaces[space_cnt];

		ft_memset(spaces, ' ', space_cnt);
		cnt += write(1, spaces, space_cnt);
	}
	return (cnt);
}

int	ft_print_int(t_flags flags, va_list ap) {
	const int		nb = va_arg(ap, int);

	return (process_number_flags(flags, ft_nbrlen(nb), nb < 0) + ft_putnbr(nb));
}

int	ft_print_str(t_flags flags, va_list ap) {
	const char		*s = va_arg(ap, const char *);
	size_t			len = ft_strlen(s);

	if (flags.precision < len)
		len = flags.precision;
	return (process_string_flags(flags, len) + write(1, s, len));
}

int	ft_print_hex(t_flags flags, va_list ap) {
	const unsigned	nb = va_arg(ap, const unsigned);

	return (process_number_flags(flags, ft_nbrlen(nb), false) + ft_putnbr(nb));
}

int	ft_print_deflt(t_flags flags, char c) {
	int		cnt = 0;

	if (flags.minimal_field != FLAG_UNSET) {
		size_t	space_cnt = flags.minimal_field - 1;
		char	spaces[space_cnt];

		ft_memset(spaces, ' ', space_cnt);
		cnt += write(1, spaces, space_cnt);
	}
	return (cnt + write(1, &c, 1));
}

t_flags	get_flags(const char *fmt) {
	t_flags	flags = { FLAG_UNSET, FLAG_UNSET, false };

	while (*fmt && !ft_isalpha(*fmt)) {
		if (ft_isdigit(*fmt)) {
			flags.minimal_field = 0;
			while (ft_isdigit(*fmt))
				flags.minimal_field = 10 * flags.minimal_field + *fmt++ - 48;
		}
		if (*fmt == '.') {
			flags.precision = 0;
			fmt++;
			while (ft_isdigit(*fmt))
				flags.precision = 10 * flags.precision + *fmt++ - 48;
		}
	}
	if (!ft_isalpha(*fmt))
		flags.error = true;
	return (flags);
}

int	ft_printf(const char *fmt, ...) {
	va_list	ap;
	t_flags	flags;
	int		cnt = 0;

	va_start(ap, fmt);
	while (*fmt) {
		if (*fmt == '%') {
			flags = get_flags(fmt + 1);
			while (*fmt && !ft_isalpha(*fmt))
				fmt++;
			if (g_print_fun[(unsigned)*fmt])
				cnt += (*g_print_fun[(unsigned)*fmt])(flags, ap);
			else
				cnt += ft_print_deflt(flags, *fmt);
		} else
			cnt += write(1, fmt, 1);
		fmt++;
	}
	va_end(ap);
	return (cnt);
}
