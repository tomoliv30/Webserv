#include <string>//std::string
#include <string.h>//memset
#include <unistd.h>//size_t
#include <stdint.h>
#include <limits.h>

static size_t		write_reverse(size_t n, char *buf)
{
	size_t	len;

	len = 0;
	while (n)
	{
		buf[len] = '0' + n % 10;
		n *= 0.1;
		len++;
	}
	return (len);
}

std::string sputnbr(size_t n)
{
	size_t	start;
	size_t	end;
	char	buf[20];
	char	swap;

	memset(buf, 0, 20);
	end = 0;
	if (!n)
	{
		buf[0] = '0';
		buf[1] = '\0';
		return (std::string(buf));
	}
	start = write_reverse(n, buf);
	buf[start] = '\0';
	start--;
	while (start > end)
	{
		swap = buf[end];
		buf[end] = buf[start];
		buf[start] = swap;
		start--;
		end++;
	}
	return (std::string(buf));
}


static int	atoiIsBlank(char c)
{
	return (c == '\t' || c == '\v' || c == '\f'
				|| c == '\r' || c == '\n' || c == ' ');
}

static int	atoiConvert(const char *str, int sign)
{
	long int	result;
	int			i;

	result = 0;
	i = 0;
	while (str[i] >= 48 && str[i] <= 57)
	{
		if (result && LONG_MAX / result == 10)
			return (((int)result * 10 + (str[i] - '0')) * sign);
		if (result && LONG_MAX / result < 10 && sign == -1)
			return (0);
		if (result && LONG_MAX / result < 10 && sign == 1)
			return (-1);
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (((int)result) * sign);
}

int			ftAtoi(const char *str)
{
	int			i;
	int			sign;

	i = 0;
	sign = 1;
	while (atoiIsBlank(str[i]))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	return (atoiConvert(str + i, sign));
}

bool isBigEndian() {
	union {
		uint32_t i;
		uint8_t	 c[4];
	} u = {0x01020304};

	return u.c[0] == 1;
}

uint32_t hostToNetworkLong(uint32_t hostlong) {
	if (isBigEndian())
		return hostlong;
	return (hostlong & 0x000000FF) << 24 |
		   (hostlong & 0x0000FF00) << 8 |
		   (hostlong & 0x00FF0000) >> 8 |
		   (hostlong & 0xFF000000) >> 24;
}

uint16_t hostToNetworkShort(uint16_t hostshort) {
	if (isBigEndian())
		return hostshort;
	return (hostshort & 0x00FF) << 8 | (hostshort & 0xFF00) >> 8;
}

int		ft_getline(std::string& total, std::string& line, int line_break) // make line_break 1 if you want it in, zero if you don't
{
	static size_t start;
	size_t len;

	if (total.find('\n', start) != std::string::npos)
	{
		len = total.find_first_of('\n', start) - start;
		line = total.substr(start, len + line_break);
		start = total.find_first_of('\n', start) + 1;
		return 1;
	}
	else {
		line = total.substr(start, total.size() - start);
		start = 0;
	}
	return 0;
}
