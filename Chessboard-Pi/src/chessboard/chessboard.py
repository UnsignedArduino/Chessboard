import logging
from enum import Enum

import chess

from src.utils.logger import create_logger
from .reader import ReedSwitchChessboardReader

logger = create_logger(name=__name__, level=logging.DEBUG)


class ReedSwitchChessboardState(Enum):
    NullState = 0
    WaitingForSetup = 1
    WaitingForMove = 2
    ThinkingMove = 3
    GameEnd = 4


class ReedSwitchChessboard:
    board: chess.Board
    reader: ReedSwitchChessboardReader
    playingAs: bool

    def __init__(self, playingAs: bool = chess.WHITE):
        self.board = chess.Board()
        self.reader = ReedSwitchChessboardReader()
        self.playingAs = playingAs

    def readerBitsMatchBoard(self) -> bool:
        for ri, row in enumerate(self.reader.state):
            for ci, col in enumerate(row):
                if col != (self.board.piece_at(((7 - ri) * 8) + ci) is not None):
                    return False
        return True

    def update(self) -> ReedSwitchChessboardState:
        return ReedSwitchChessboardState.NullState

    def __str__(self):
        return self.board.__str__()

    def __repr__(self):
        return self.__str__()
