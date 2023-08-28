import logging
from typing import List, NamedTuple

import board
import chess
from adafruit_bus_device.i2c_device import I2CDevice

from src.utils.bits import bit_read
from src.utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)


RSCbReaderState = List[List[bool]]


class RSCbComparedState(NamedTuple):
    added: List[chess.Square]
    removed: List[chess.Square]


def string_rows_to_state(rows: List[str]) -> RSCbReaderState:
    return [[char == "1" for char in list(row)] for row in rows]


class RSCbReader:
    state: RSCbReaderState
    address: int
    device: I2CDevice

    CHESSBOARD_ADDRESS = 0x50
    STARTING_POSITION_BITS = string_rows_to_state(
        [
            "11111111",
            "11111111",
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "11111111",
            "11111111",
        ]
    )

    def __init__(self):
        self.state = []
        for i in range(8):
            self.state.append([False] * 8)
        self.address = RSCbReader.CHESSBOARD_ADDRESS
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

    @staticmethod
    def pos_to_square(row: int, col: int) -> chess.Square:
        return chess.Square(((7 - row) * 8) + col)

    @staticmethod
    def copy_state(s: RSCbReaderState) -> RSCbReaderState:
        new_s = []
        for ri in range(8):
            row = []
            for ci in range(8):
                row.append(s[ri][ci])
            new_s.append(row)
        return new_s

    @staticmethod
    def equal_states(s1: RSCbReaderState, s2: RSCbReaderState) -> bool:
        for ri in range(len(s1)):
            for ci in range(len(s1[ri])):
                if s1[ri][ci] != s2[ri][ci]:
                    return False
        return True

    @staticmethod
    def compare_states(s1: RSCbReaderState, s2: RSCbReaderState) -> RSCbComparedState:
        added = []
        removed = []
        for ri in range(8):
            for ci in range(8):
                first = s1[ri][ci]
                second = s2[ri][ci]
                if first and not second:
                    removed.append(RSCbReader.pos_to_square(ri, ci))
                elif not first and second:
                    added.append(RSCbReader.pos_to_square(ri, ci))
        return RSCbComparedState(added=added, removed=removed)

    @staticmethod
    def stringify_state(state: RSCbReaderState) -> str:
        s = ""
        for ri, row in enumerate(state):
            for col in row:
                s += "1" if col else "0"
            if ri < len(state) - 1:
                s += "\n"
        return s

    def __str__(self):
        return RSCbReader.stringify_state(self.state)

    def __repr__(self):
        return self.__str__()
