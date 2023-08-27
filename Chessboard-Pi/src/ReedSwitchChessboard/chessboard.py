import logging
from typing import List

import chess

from src.utils.logger import create_logger
from .reader import RSCbReader

logger = create_logger(name=__name__, level=logging.DEBUG)


class RSCb:
    board: chess.Board
    last_state: List[List[bool]]
    reader: RSCbReader

    def __init__(self):
        self.board = chess.Board()
        self.reader = RSCbReader()
        self.last_state = RSCbReader.copy_state(self.reader.state)

    def readerBitsMatchBoard(self) -> bool:
        for ri, row in enumerate(self.reader.state):
            for ci, col in enumerate(row):
                if col != (self.board.piece_at(((7 - ri) * 8) + ci) is not None):
                    return False
        return True

    def __str__(self):
        return self.board.__str__()

    def __repr__(self):
        return self.__str__()
