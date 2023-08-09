def bit_set(value: int, bit: int) -> int:
    value |= 1 << bit
    return value


def bit_clear(value: int, bit: int) -> int:
    value &= ~(1 << bit)
    return value


def bit_write(value: int, bit: int, bit_value: bool) -> int:
    if bit_value:
        return bit_set(value, bit)
    else:
        return bit_clear(value, bit)


def bit_read(value: int, bit: int) -> bool:
    return ((value >> bit) & 0x01) == 1
