import logging
from typing import List

import board
from adafruit_bus_device.i2c_device import I2CDevice

from utils.bits import bit_read
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

CHESSBOARD_ADDRESS = 0x50


class ReedSwitchChessboard:
    state: List[List[bool]]
    address: int
    device: I2CDevice

    def __init__(self):
        self.state = []
        for i in range(8):
            self.state.append([False] * 8)
        self.address = CHESSBOARD_ADDRESS
        logger.debug("Initiating hardware")
        self.device = I2CDevice(board.I2C(), self.address)
        logger.debug(f"Chessboard on I2C bus at 0x{self.address:02x}")
        self.read()

    def update(self) -> bool:
        with self.device:
            if self.read_uint8_register(0x8F) > 0:
                logger.debug("Board has new state")
                self.read()
                return True
            else:
                return False

    def read(self):
        logger.debug("Reading board state")
        for row in range(8):
            register = 0x90 + row
            row_byte = self.read_uint8_register(register)
            for col in range(8):
                self.state[row][col] = bit_read(row_byte, col)

    def read_uint8_register(self, reg_addr: int) -> int:
        result = bytearray(1)
        self.device.write_then_readinto(bytes([reg_addr]), result)
        return result[0]

    def __str__(self):
        s = ""
        for row in self.state:
            for col in row:
                s += "1" if col else "0"
            s += "\n"
        return s

    def __repr__(self):
        return self.__str__()
