import logging
from enum import Enum

import chess

from src.utils.logger import create_logger
from .reader import RSCbReader, RSCbReaderState

logger = create_logger(name=__name__, level=logging.DEBUG)


class RSCbState(Enum):
    NullState = 0
    WaitingForSetup = 1
    WaitingForWhiteMove = 2
    WaitingForBlackMove = 3
    PieceMoving = 4
    IllegalWhiteMove = 5
    IllegalBlackMove = 6


class RSCb:
    board: chess.Board
    last_state: RSCbReaderState
    reader: RSCbReader

    def __init__(self):
        self.board = chess.Board()
        self.reader = RSCbReader()
        self.last_state = RSCbReader.copy_state(RSCbReader.STARTING_POSITION_BITS)

    @staticmethod
    def stringify_square_set(ss: chess.SquareSet) -> str:
        return [chess.square_name(i) for i in list(ss)].__str__()

    def readerBitsMatchBoard(self) -> bool:
        for ri, row in enumerate(self.reader.state):
            for ci, col in enumerate(row):
                if col != (
                    self.board.piece_at(RSCbReader.pos_to_square(ri, ci)) is not None
                ):
                    return False
        return True

    def update(self) -> RSCbState:
        self.reader.update()
        if self.readerBitsMatchBoard():
            # logger.debug("Reader matches chess board game")
            if self.board.turn == chess.WHITE:
                # logger.debug("White to move")
                return RSCbState.WaitingForWhiteMove
            else:
                # logger.debug("Black to move")
                return RSCbState.WaitingForBlackMove
        else:
            # logger.debug("Reader does not match chess board game, piece moving?")
            comparison = RSCbReader.compare_states(self.last_state, self.reader.state)
            # logger.debug(comparison)
            if len(comparison.added) == 1 and len(comparison.removed):
                logger.debug("Trying move")
                try:
                    move = self.board.find_move(
                        comparison.removed[0], comparison.added[0]
                    )
                    logger.debug(f"Found legal move {move}")
                except chess.IllegalMoveError:
                    logger.debug("Move not legal")
                    return (
                        RSCbState.IllegalWhiteMove
                        if self.board.turn == chess.WHITE
                        else RSCbState.IllegalBlackMove
                    )
                logger.debug(f"Making move {move}")
                self.board.push(move)
                self.last_state = RSCbReader.copy_state(self.reader.state)
                return (
                    RSCbState.WaitingForBlackMove
                    if self.board.turn == chess.WHITE
                    else RSCbState.WaitingForWhiteMove
                )
            else:
                return RSCbState.PieceMoving

    def __str__(self):
        return self.board.__str__()

    def __repr__(self):
        return self.__str__()
