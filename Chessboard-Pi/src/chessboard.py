import logging
from typing import List

from adafruit_bus_device.i2c_device import I2CDevice
from board import I2C

from utils.bits import bit_read
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

CHESSBOARD_ADDRESS = 0x55


class ReedSwitchChessboard:
    state: List[List[bool]]

    def __init__(self):
        self.state = []
        for i in range(8):
            self.state.append([False] * 8)
        self.address = CHESSBOARD_ADDRESS
        self.device = I2CDevice(I2C(), self.address)
        logger.debug(f"Chessboard on I2C bus at 0x{self.address:02x}")

    def update(self):
        logger.debug("Getting chessboard state")
        with self.device as device:
            for row in range(8):
                register = 0x90 + row
                result = bytearray(1)
                device.write_then_readinto(bytes([register]), result)
                row_byte = result[0]
                for col in range(8):
                    self.state[row][col] = bit_read(row_byte, col)

    def __str__(self):
        s = ""
        for row in self.state:
            for col in row:
                s += "1" if col else "0"
            s += "\n"
        return s

    def __repr__(self):
        return self.__str__()
