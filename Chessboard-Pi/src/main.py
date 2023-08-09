import logging
from typing import List

from adafruit_bus_device.i2c_device import I2CDevice
from board import I2C

from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

CHESSBOARD_ADDRESS = 0x55
chessboard = I2CDevice(I2C(), CHESSBOARD_ADDRESS)
logger.debug(f"Chessboard on I2C bus at 0x{CHESSBOARD_ADDRESS:02x}")


def bit_read(value: int, bit: int) -> bool:
    return ((value >> bit) & 0x01) == 1


def get_chessboard_state() -> List[List[bool]]:
    logger.debug("Getting chessboard state")
    state = []
    for i in range(8):
        state.append([False] * 8)
    with chessboard as cb:
        for row in range(8):
            register = 0x90 + row
            result = bytearray(1)
            cb.write_then_readinto(bytes([register]), result)
            row_byte = result[0]
            for col in range(8):
                state[row][col] = bit_read(row_byte, col)
    return state


def print_chessboard_state(cb: List[List[bool]]):
    for row in cb:
        for col in row:
            print("1" if col else "0", end="")
        print()


print_chessboard_state(get_chessboard_state())
