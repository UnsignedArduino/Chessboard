import logging
from typing import List

import board
from adafruit_bus_device.i2c_device import I2CDevice
from digitalio import DigitalInOut, Direction

from utils.bits import bit_read
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

CHESSBOARD_ADDRESS = 0x55


class ReedSwitchChessboard:
    state: List[List[bool]]
    address: int
    device: I2CDevice
    interrupt_pin: DigitalInOut

    def __init__(self):
        self.state = []
        for i in range(8):
            self.state.append([False] * 8)
        self.address = CHESSBOARD_ADDRESS
        logger.debug("Initiating hardware")
        self.device = I2CDevice(board.I2C(), self.address)
        logger.debug(f"Chessboard on I2C bus at 0x{self.address:02x}")
        self.interrupt_pin = DigitalInOut(board.D4)
        self.interrupt_pin.direction = Direction.INPUT
        self.read()

    def update(self) -> bool:
        if self.interrupt_pin.value:
            logger.debug("Board has new state")
            self.read()
            return True
        else:
            return False

    def read(self):
        logger.debug("Reading board state")
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
